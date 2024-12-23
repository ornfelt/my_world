#include "sha/sha.h"

#include <jkssl/rand.h>

#include <sys/types.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

static uint64_t rand_value;

static void add(uint64_t i)
{
	rand_value += i;
	rand_value = rand_value * 6364136223846793005ULL + 1442695040888963407ULL;
}

static int add_file(char *file)
{
	ssize_t rd;
	ssize_t tmp;
	char buf[2048];
	int fd;

	if ((fd = open(file, O_RDONLY)) == -1)
		return 0;
	rd = read(fd, buf, 2048);
	close(fd);
	if (rd < 0)
		return 0;
	tmp = 0;
	while (rd - tmp > 8)
	{
		add(*(uint64_t*)&buf[tmp]);
		tmp += 8;
	}
	while (tmp < rd)
	{
		add(buf[tmp]);
		++tmp;
	}
	return 1;
}

static int add_urandom(void)
{
	return add_file("/dev/urandom");
}

__attribute__((constructor))
static void setup(void)
{
	struct timespec ts;
	if (!clock_gettime(CLOCK_MONOTONIC, &ts))
	{
		add(ts.tv_sec);
		add(ts.tv_nsec);
	}
	add(time(NULL));
	add_urandom();
}

int rand_bytes(void *data, size_t size)
{
	while (size > 20)
	{
		add(0);
		sha1((uint8_t*)&rand_value, sizeof(rand_value), data);
		data += 20;
		size -= 20;
	}
	if (size)
	{
		uint8_t tmp[20];
		add(0);
		sha1((uint8_t*)&rand_value, sizeof(rand_value), tmp);
		memcpy(data, tmp, size);
	}
	return 1;
}
