#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	while (1)
	{
		if (argc > 1)
		{
			for (int i = 1; i < argc; ++i)
			{
				fputs(argv[i], stdout);
				if (i != argc - 1)
					fputs(" ", stdout);
			}
			fputs("\n", stdout);
		}
		else
		{
			puts("y");
		}
	}
}
