#include <stdio.h>
#include <string.h>

int djb2_hash(unsigned char *str)
{
	unsigned long hash = 5381;
	unsigned char c = *str++;
	while (c != '\0') {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		c = *str++;
	}
	return hash;
}

int main(int argc, char *argv[])
{
	char djb2_char[4096];
	read(0,djb2_char, 4096 );
	printf("%d",(signed int)djb2_hash((unsigned char *)djb2_char));
	if (!*++argv || strcmp(*argv, "-n"))
		putchar('\n');
	return 0;
}