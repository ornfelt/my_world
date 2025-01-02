#include <sys/stat.h>
#include <sys/ipc.h>

key_t ftok(const char *pathname, int proj_id)
{
	struct stat st;
	if (stat(pathname, &st) == -1)
		return -1;
	return (proj_id & 0xFF)
	     | ((st.st_ino & 0xFFFF) << 8)
	     | ((st.st_dev & 0xFF) << 24);
}
