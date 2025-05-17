#include <sys/statvfs.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

static const char *
fs_name(unsigned long magic)
{
	switch (magic)
	{
		case DEVFS_MAGIC:
			return "devfs";
		case RAMFS_MAGIC:
			return "ramfs";
		case SYSFS_MAGIC:
			return "sysfs";
		case PROCFS_MAGIC:
			return "procfs";
		case TARFS_MAGIC:
			return "tarfs";
		case EXT2FS_MAGIC:
			return "ext2fs";
		default:
			return "unknown";
	}
}

static const char *
flags_str(unsigned long flags)
{
	static char str[512];

	str[0] = '\0';
	if (flags & ST_RDONLY)
		strlcat(str, "ro,", sizeof(str));
	else
		strlcat(str, "rw,", sizeof(str));
	if (flags & ST_NOSUID)
		strlcat(str, "nosuid,", sizeof(str));
	if (flags & ST_NOEXEC)
		strlcat(str, "noexec,", sizeof(str));
	str[strlen(str) - 1] = '\0';
	return str;
}

static int
statfs_file(const char *progname, const char *file)
{
	struct statvfs st;

	if (statvfs(file, &st) == -1)
	{
		fprintf(stderr, "%s: statvfs: %s\n",
		        progname,
		        strerror(errno));
		return 1;
	}
	printf(" block size: %lu\n", st.f_bsize);
	printf("  frag size: %lu\n", st.f_frsize);
	printf("     blocks: %" PRIu64 "\n", st.f_blocks);
	printf("free blocks: %" PRIu64 "\n", st.f_bfree);
	printf("avail block: %" PRIu64 "\n", st.f_bavail);
	printf("      files: %" PRIu64 "\n", st.f_files);
	printf(" free files: %" PRIu64 "\n", st.f_ffree);
	printf("avail files: %" PRIu64 "\n", st.f_favail);
	printf("         id: %lu\n", st.f_fsid);
	printf("      flags: %lu (%s)\n", st.f_flag, flags_str(st.f_flag));
	printf("   max name: %lu\n", st.f_namemax);
	printf("      magic: %lx\n", st.f_magic);
	printf("       name: %s\n", fs_name(st.f_magic));
	return 0;
}

int
main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = 1; i < argc; ++i)
	{
		if (statfs_file(argv[0], argv[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
