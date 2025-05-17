#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int
getloadavg(double loadavg[], int nelem)
{
	if (nelem < 3)
		return -1;
	int ret = -1;
	char *line = NULL;
	size_t len = 0;
	FILE *fp = fopen("/sys/loadavg", "r");
	if (!fp)
		goto end;
	if (getline(&line, &len, fp) < 0)
		goto end;
	char *endptr;
	uint32_t avg[6];
	errno = 0;
	avg[0] = strtoul(line, &endptr, 10);
	if (errno || *endptr != '.')
		goto end;
	avg[1] = strtoul(&endptr[1], &endptr, 10);
	if (errno || *endptr != ' ')
		goto end;
	avg[2] = strtoul(&endptr[1], &endptr, 10);
	if (errno || *endptr != '.')
		goto end;
	avg[3] = strtoul(&endptr[1], &endptr, 10);
	if (errno || *endptr != ' ')
		goto end;
	avg[4] = strtoul(&endptr[1], &endptr, 10);
	if (errno || *endptr != '.')
		goto end;
	avg[5] = strtoul(&endptr[1], &endptr, 10);
	if (errno || *endptr != '\n')
		goto end;
	loadavg[0] = avg[0] + avg[1] / 100.;
	loadavg[1] = avg[2] + avg[3] / 100.;
	loadavg[2] = avg[4] + avg[5] / 100.;
	ret = 0;

end:
	if (fp)
		fclose(fp);
	free(line);
	return ret;
}
