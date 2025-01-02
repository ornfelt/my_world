#include <stdlib.h>
#include <stdio.h>

int main()
{
	for (size_t i = 0; i < 10; ++i)
	{
		char tmp[] = "\033[0;30m0;30 \033[1;30m1;30 \033[0;40m0;40\033[0m \033[1;40m1;40\033[0m";
		tmp[5]  = '0' + i;
		tmp[10] = '0' + i;
		tmp[17] = '0' + i;
		tmp[22] = '0' + i;
		tmp[29] = '0' + i;
		tmp[34] = '0' + i;
		tmp[45] = '0' + i;
		tmp[50] = '0' + i;
		puts(tmp);
	}
	return EXIT_SUCCESS;
}
