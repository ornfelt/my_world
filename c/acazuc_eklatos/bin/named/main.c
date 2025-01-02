#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <resolv.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <time.h>

/* XXX
 * when processing a request, we should gather existing records
 * and if not all records are available, we should generate a request for all
 * the missing ones
 * after this, we shoud merge all of this into a single reply
 *
 * problem comes if some entries timeout before receiving the response
 * I think we should also gather records where the timeout is close (at least
 * the network timeout of a request)
 *
 * also, the data structures are (to be generous) less than optimal
 * I think an hash table of name->entry should be great
 * each entry should then contain an array of list of records
 * (one array index per class, ignoring type because it's always inet ?)
 *
 * we should also reuse the res_state across the server to avoid recreating
 * a server socket each request
 * the main loop should then listen on it to avoid blocking request when
 * records must be fetched from the server
 *
 * lots of error handling in there must be handled better by returning error
 * codes to the request client instead of dropping the request (set ns_f_rcode)
 */

struct ns_header
{
	uint16_t ident;
	uint16_t flags;
	uint16_t nreq;
	uint16_t nrep;
	uint16_t nauth;
	uint16_t nadd;
};

struct record
{
	int class;
	int type;
	time_t timeout;
	uint16_t rdlen;
	uint8_t *data;
	TAILQ_ENTRY(record) chain;
};

struct entry
{
	char name[256];
	TAILQ_HEAD(, record) records;
	TAILQ_ENTRY(entry) chain;
};

struct env
{
	const char *progname;
	struct in_addr server;
	int sock;
	TAILQ_HEAD(, entry) entries;
};

static int load_server(struct env *env)
{
	FILE *fp = NULL;
	char buf[256];
	int ret = 1;

	fp = fopen("/tmp/named_server", "r");
	if (!fp)
	{
		fprintf(stderr, "%s: fopen(%s): %s\n", env->progname,
		        "/tmp/named_server", strerror(errno));
		goto end;
	}
	if (!fgets(buf, sizeof(buf), fp))
	{
		fprintf(stderr, "%s: fgets(%s): %s\n", env->progname,
		       "/tmp/named_server", strerror(errno));
		goto end;
	}
	if (buf[0] && buf[strlen(buf) - 1] == '\n')
		buf[strlen(buf) - 1] = '\0';
	switch (inet_pton(AF_INET, buf, &env->server))
	{
		case -1:
			fprintf(stderr, "%s: inet_pton: %s\n", env->progname,
			        strerror(errno));
			goto end;
		case 0:
		default:
			fprintf(stderr, "%s: inet_pton: invalid address\n",
			        env->progname);
			goto end;
		case 1:
			break;
	}
	ret = 0;

end:
	if (fp)
		fclose(fp);
	return ret;
}

static int setup_sock(struct env *env)
{
	env->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (env->sock == -1)
	{
		fprintf(stderr, "%s: socket: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(53);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(env->sock, (struct sockaddr*)&sin, sizeof(sin)) == -1)
	{
		fprintf(stderr, "%s: bind: %s\n", env->progname,
		        strerror(errno));
		close(env->sock);
		env->sock = -1;
		return 1;
	}
	return 0;
}

static int generate_cached_reply_record(struct env *env, uint8_t *buf,
                                        size_t len, struct record *record,
                                        time_t t)
{
	if (len < 10)
	{
		fprintf(stderr, "%s: buffer too small\n", env->progname);
		return -1;
	}
	uint8_t *ptr = buf;
	*(uint16_t*)ptr = ntohs(record->type);
	ptr += 2;
	*(uint16_t*)ptr = ntohs(record->class);
	ptr += 2;
	if (t >= record->timeout)
		*(uint32_t*)ptr = 0;
	else
		*(uint32_t*)ptr = ntohl(record->timeout - t);
	ptr += 4;
	*(uint16_t*)ptr = ntohs(record->rdlen);
	ptr += 2;
	len -= 10;
	if (len < record->rdlen)
	{
		fprintf(stderr, "%s: buffer too small\n", env->progname);
		return -1;
	}
	memcpy(ptr, record->data, record->rdlen);
	ptr += record->rdlen;
	len -= record->rdlen;
	return ptr - buf;
}

static int generate_cached_reply(struct env *env, uint8_t *buf, size_t len,
                                 struct entry *entry, ns_msg *msg)
{
	time_t t = time(NULL);
	struct ns_header *header = (struct ns_header*)buf;
	header->ident = ntohs(ns_msg_id(*msg));
	header->flags = ntohs((1 << 15) | (ns_msg_getflag(*msg, ns_f_rd) << 8)); /* QR */
	header->nreq = 0;
	header->nrep = 0;
	header->nauth = 0;
	header->nadd = 0;
	uint8_t *ptr = &buf[sizeof(*header)];
	len -= sizeof(*header);
	for (int i = 0; i < ns_msg_count(*msg, ns_s_qd); ++i)
	{
		ns_rr rr;
		if (ns_parserr(msg, ns_s_qd, 0, &rr) == -1)
		{
			fprintf(stderr, "%s: ns_parserr failed\n", env->progname);
			return -1;
		}
		int ret = dn_comp(entry->name, ptr, len, NULL, NULL);
		if (ret == -1)
		{
			fprintf(stderr, "%s: dn_comp failed\n", env->progname);
			return -1;
		}
		ptr += ret;
		len -= ret;
		if (len < 4)
		{
			fprintf(stderr, "%s: buffer too small\n", env->progname);
			return -1;
		}
		*(uint16_t*)ptr = ntohs(ns_rr_type(rr));
		ptr += 2;
		*(uint16_t*)ptr = ntohs(ns_rr_class(rr));
		ptr += 2;
		len -= 4;
		header->nreq++;
	}
	for (size_t i = 0; i < 1; ++i)
	{
		struct record *record = TAILQ_FIRST(&entry->records);
		int ret = dn_comp(entry->name, ptr, len, NULL, NULL);
		if (ret == -1)
		{
			fprintf(stderr, "%s: dn_comp failed\n", env->progname);
			return -1;
		}
		ptr += ret;
		len -= ret;
		ret = generate_cached_reply_record(env, ptr, len, record, t);
		if (ret == -1)
			return -1;
		ptr += ret;
		len -= ret;
		header->nrep++;
	}
	header->nreq = ntohs(header->nreq);
	header->nrep = ntohs(header->nrep);
	return ptr - buf;
}

static int send_cached_reply(struct env *env, struct entry *entry,
                             ns_msg *msg, struct sockaddr *sockaddr,
                             socklen_t socklen)
{
	uint8_t reply[512];
	ssize_t ret = generate_cached_reply(env, reply, sizeof(reply),
	                                    entry, msg);
	if (ret == -1)
	{
		fprintf(stderr, "%s: ns_nmkquery failed\n", env->progname);
		return 1;
	}
	ret = sendto(env->sock, reply, ret, 0, sockaddr, socklen);
	if (ret == -1)
	{
		fprintf(stderr, "%s: sendto: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static struct record *record_new(struct env *env, ns_rr *rr)
{
	struct record *record = NULL;

	record = malloc(sizeof(*record));
	if (!record)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname,
		        strerror(errno));
		return NULL;
	}
	record->class = ns_rr_class(*rr);
	record->type = ns_rr_type(*rr);
	record->timeout = ns_rr_ttl(*rr);
	record->rdlen = ns_rr_rdlen(*rr);
	record->data = malloc(record->rdlen);
	if (!record->data)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname,
		        strerror(errno));
		free(record);
		return NULL;
	}
	memcpy(record->data, ns_rr_rdata(*rr), record->rdlen);
	return record;
}

static struct entry *entry_new(struct env *env, const uint8_t *buf, size_t len)
{
	struct entry *entry = NULL;
	time_t t;
	ns_msg msg;

	if (ns_initparse(buf, len, &msg) == -1)
	{
		fprintf(stderr, "%s: ns_initparse failed\n", env->progname);
		goto err;
	}
	if (ns_msg_getflag(msg, ns_f_rcode))
	{
		fprintf(stderr, "%s: ns error: %d\n", env->progname,
		        ns_msg_getflag(msg, ns_f_rcode));
		goto err;
	}
	if (!ns_msg_count(msg, ns_s_an))
	{
		fprintf(stderr, "%s: no answers in reply\n", env->progname);
		goto err;
	}
	entry = malloc(sizeof(*entry));
	if (!entry)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname,
		        strerror(errno));
		goto err;
	}
	TAILQ_INIT(&entry->records);
	t = time(NULL);
	for (size_t i = 0; i < ns_msg_count(msg, ns_s_an); ++i)
	{
		ns_rr rr;
		struct record *record;

		if (ns_parserr(&msg, ns_s_an, i, &rr) == -1)
		{
			fprintf(stderr, "%s: ns_parserr failed\n", env->progname);
			goto err;
		}
		/* XXX shouldn't do this */
		if (!i)
			strlcpy(entry->name, ns_rr_name(rr), sizeof(entry->name));
		record = record_new(env, &rr);
		if (!record)
			goto err;
		record->timeout += t;
		TAILQ_INSERT_TAIL(&entry->records, record, chain);
	}
	return entry;

err:
	free(entry);
	return NULL;
}

static int server_connect(struct env *env)
{
	struct sockaddr_in sin;
	struct timeval tv;
	int fd;

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == -1)
	{
		fprintf(stderr, "%s: socket: %s\n", env->progname,
		        strerror(errno));
		return -1;
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(53);
	sin.sin_addr = env->server;
	if (connect(fd, (struct sockaddr*)&sin, sizeof(sin)) == -1)
	{
		fprintf(stderr, "%s: connect: %s\n", env->progname,
		        strerror(errno));
		close(fd);
		return -1;
	}
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv,
	               sizeof(tv)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(SO_RCVTIMEO): %s\n",
		        env->progname, strerror(errno));
		return -1;
	}
	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv,
	               sizeof(tv)) == -1)
	{
		fprintf(stderr, "%s: setsockopt(SO_SNDTIMEO): %s\n",
		        env->progname, strerror(errno));
		return -1;
	}
	return fd;
}

static int server_request(struct env *env, ns_msg *msg, ns_rr *rr,
                          struct sockaddr *sockaddr, socklen_t socklen)
{
	uint8_t buf[512];
	ssize_t len;
	int sock;
	struct entry *entry;

	sock = server_connect(env);
	if (sock == -1)
		return 1;
	len = res_nmkquery(NULL, QUERY, ns_rr_name(*rr), ns_c_in, ns_t_a, NULL, 0,
	                   NULL, buf, sizeof(buf));
	if (len == -1)
	{
		close(sock);
		return 1;
	}
	if (send(sock, buf, len, 0) == -1)
	{
		fprintf(stderr, "%s: send: %s\n", env->progname, strerror(errno));
		close(sock);
		return 1;
	}
	len = recv(sock, buf, sizeof(buf), 0);
	close(sock);
	if (len == -1)
	{
		fprintf(stderr, "%s: recv: %s\n", env->progname, strerror(errno));
		return 1;
	}
	entry = entry_new(env, buf, len);
	if (!entry)
		return 1;
	return send_cached_reply(env, entry, msg, sockaddr, socklen);
}

static int handle_request(struct env *env)
{
	uint8_t buf[512];
	struct sockaddr sockaddr;
	socklen_t socklen = sizeof(sockaddr);
	ssize_t ret;
	struct entry *entry;
	ns_msg msg;
	ns_rr rr;

	ret = recvfrom(env->sock, buf, sizeof(buf), 0, &sockaddr, &socklen);
	if (ret == -1)
	{
		fprintf(stderr, "%s: recvfrom: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	if (ns_initparse(buf, ret, &msg) == -1)
	{
		fprintf(stderr, "%s: ns_initparse failed\n", env->progname);
		return 1;
	}
	if (ns_msg_getflag(msg, ns_f_opcode) != QUERY)
	{
		fprintf(stderr, "%s: request isn't QUERY\n", env->progname);
		return 1;
	}
	if (ns_msg_getflag(msg, ns_f_qr))
	{
		fprintf(stderr, "%s: response flag in request\n", env->progname);
		return 1;
	}
	if (ns_msg_count(msg, ns_s_qd) != 1)
	{
		fprintf(stderr, "%s: invalid number of request: %d\n",
		        env->progname, ns_msg_count(msg, ns_s_qd));
		return 1;
	}
	if (ns_msg_count(msg, ns_s_an)
	 || ns_msg_count(msg, ns_s_ns)
	 || ns_msg_count(msg, ns_s_ar))
	{
		fprintf(stderr, "%s: non-empty msg count\n", env->progname);
		return 1;
	}
	if (ns_parserr(&msg, ns_s_qd, 0, &rr) == -1)
	{
		fprintf(stderr, "%s: ns_parserr failed\n", env->progname);
		return 1;
	}
	if (ns_rr_type(rr) != ns_t_a) /* XXX */
	{
		fprintf(stderr, "%s: invalid ns type\n", env->progname);
		return 1;
	}
	if (ns_rr_class(rr) != ns_c_in) /* XXX */
	{
		fprintf(stderr, "%s: invalid ns class\n", env->progname);
		return 1;
	}
	TAILQ_FOREACH(entry, &env->entries, chain)
	{
		struct record *record = TAILQ_FIRST(&entry->records);
		if (record->class == ns_rr_class(rr)
		 && record->type == ns_rr_type(rr)
		 && !strcmp(entry->name, ns_rr_name(rr)))
			break;
	}
	if (entry)
	{
		struct record *record = TAILQ_FIRST(&entry->records);
		if (time(NULL) <= record->timeout)
			return send_cached_reply(env, entry, &msg,
			                         &sockaddr, socklen);
		TAILQ_REMOVE(&env->entries, entry, chain);
		free(entry);
	}
	return server_request(env, &msg, &rr, &sockaddr, socklen);
}

static void usage(const char *progname)
{
	printf("%s [-h]\n", progname);
	printf("-h: display this help\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	TAILQ_INIT(&env.entries);
	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (load_server(&env))
		return EXIT_FAILURE;
	if (setup_sock(&env))
		return EXIT_FAILURE;
	if (daemon(1, 1) == -1)
	{
		fprintf(stderr, "%s: daemon: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	while (1)
	{
		struct pollfd fds;
		fds.fd = env.sock;
		fds.events = POLLIN;
		int ret = poll(&fds, 1, -1);
		if (ret == -1)
		{
			if (ret != EINTR)
			{
				fprintf(stderr, "%s: poll: %s\n", argv[0],
				        strerror(errno));
				return EXIT_FAILURE;
			}
			continue;
		}
		handle_request(&env);
	}
	return EXIT_SUCCESS;
}
