#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <resolv.h>
#include <stdio.h>
#include <errno.h>

struct env
{
	const char *progname;
	const char *type;
};

static int print_rr(struct env *env, ns_msg *msg, ns_sect sect, ns_rr *rr)
{
	(void)msg;
	printf(" rr:\n");
	printf("  name: %s\n", ns_rr_name(*rr));
	printf("  type: %" PRIu16 "\n", ns_rr_type(*rr));
	printf("  class: %" PRIu16 "\n", ns_rr_class(*rr));
	if (sect == ns_s_qd)
		return 0;
	printf("  ttl: %" PRIu32 "\n", ns_rr_ttl(*rr));
	printf("  rdlen: %" PRIu16 "\n", ns_rr_rdlen(*rr));
	switch (ns_rr_type(*rr))
	{
		case ns_t_a:
			if (ns_rr_rdlen(*rr) != 4)
			{
				fprintf(stderr, "%s: invalid rr a rdlen",
				        env->progname);
				return 1;
			}
			printf("  addr: %s\n",
			       inet_ntoa(*(struct in_addr*)ns_rr_rdata(*rr)));
			break;
		case ns_t_aaaa:
			if (ns_rr_rdlen(*rr) != 16)
			{
				fprintf(stderr, "%s: invalid rr aaaa rdlen",
				        env->progname);
				return 1;
			}
			break;
		default:
			printf("unknown rr %d\n", ns_rr_type(*rr));
			break;
	}
	return 0;
}

static int print_sec(struct env *env, ns_msg *msg, ns_sect sect)
{
	int count;

	count = ns_msg_count(*msg, sect);
	if (!count)
		return 0;
	for (int i = 0; i < count; ++i)
	{
		ns_rr rr;
		if (ns_parserr(msg, sect, i, &rr) == -1)
		{
			fprintf(stderr, "%s: failed to parse rr\n",
			        env->progname);
			return 1;
		}
		if (print_rr(env, msg, sect, &rr))
			return 1;
	}
	return 0;
}

static int ns_t_from_str(struct env *env, const char *type, ns_type *ns_t)
{
#define TEST_T(v) \
	if (!strcasecmp(type, #v)) \
	{ \
		*ns_t = ns_t_##v; \
		return 0; \
	}

	TEST_T(a);
	TEST_T(ns);
	TEST_T(md);
	TEST_T(mf);
	TEST_T(cname);
	TEST_T(soa);
	TEST_T(mb);
	TEST_T(mg);
	TEST_T(mr);
	TEST_T(null);
	TEST_T(wks);
	TEST_T(ptr);
	TEST_T(hinfo);
	TEST_T(minfo);
	TEST_T(mx);
	TEST_T(txt);
	TEST_T(aaaa);

	fprintf(stderr, "%s: unknown ns type\n", env->progname);
	return 1;

#undef TEST_T
}

static int dig_query(struct env *env, const char *hostname)
{
	struct __res_state state;
	uint8_t buf[512];
	int len;
	ns_msg msg;
	ns_type type;

	if (ns_t_from_str(env, env->type, &type))
		return 1;
	if (res_ninit(&state))
	{
		fprintf(stderr, "%s: failed to init resolved\n", env->progname);
		return 1;
	}
	len = res_nquery(&state, hostname, ns_c_in, type, buf, sizeof(buf));
	res_nclose(&state);
	if (len < 0)
	{
		fprintf(stderr, "%s: failed to query DNS\n", env->progname);
		return 1;
	}
	if (ns_initparse(buf, len, &msg) == -1)
	{
		fprintf(stderr, "%s: failed to init ns parse\n", env->progname);
		return 1;
	}
	printf("ident: %" PRIx16 "\n", ns_msg_id(msg));
	printf("nreq: %" PRIu16 "\n", ns_msg_count(msg, ns_s_qd));
	printf("nrep: %" PRIu16 "\n", ns_msg_count(msg, ns_s_an));
	printf("nauth: %" PRIu16 "\n", ns_msg_count(msg, ns_s_ns));
	printf("nadd: %" PRIu16 "\n", ns_msg_count(msg, ns_s_ar));
	printf("flags:\n");
	printf(" qr: %d\n", ns_msg_getflag(msg, ns_f_qr));
	printf(" opcode: %d\n", ns_msg_getflag(msg, ns_f_opcode));
	printf(" aa: %d\n", ns_msg_getflag(msg, ns_f_aa));
	printf(" tc: %d\n", ns_msg_getflag(msg, ns_f_tc));
	printf(" rd: %d\n", ns_msg_getflag(msg, ns_f_rd));
	printf(" ra: %d\n", ns_msg_getflag(msg, ns_f_ra));
	printf(" z: %d\n", ns_msg_getflag(msg, ns_f_z));
	printf(" rcode: %d\n", ns_msg_getflag(msg, ns_f_rcode));
	for (int i = 0; i < ns_s_max; ++i)
	{
		if (print_sec(env, &msg, i))
			return 1;
	}
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-h]\n", progname);
	printf("-h: display this help\n");
}

int main(int argc, char **argv)
{
	struct env env;
	const char *hostname;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.type = "A";
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
	switch (argc - optind)
	{
		case 0:
			fprintf(stderr, "%s: missing operand\n", argv[0]);
			return EXIT_FAILURE;
		case 1:
			hostname = argv[optind];
			break;
		case 2:
			hostname = argv[optind];
			env.type = argv[optind + 1];
			break;
		default:
			fprintf(stderr, "%s: extra operand\n", argv[0]);
			return EXIT_FAILURE;
	}
	if (dig_query(&env, hostname))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
