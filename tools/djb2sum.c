#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

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
	unsigned int len_max = 1024;
	unsigned int current_size = 0;
	char *djb2_char = malloc(len_max);
	current_size = len_max;
	if(djb2_char != NULL)
	{
		char c = EOF;
		unsigned int i =0;
		while (( c = getchar() ) != EOF)
		{
			djb2_char[i++]=(char)c;
			if(i == current_size)
			{
				current_size = i+len_max;
				djb2_char = realloc(djb2_char, current_size);
			}
		}
		djb2_char[i] = '\0';
	}
	printf("%d",(signed int)djb2_hash((unsigned char *)djb2_char));
	if (!*++argv || strcmp(*argv, "-n"))
		putchar('\n');
	free(djb2_char);
	djb2_char = NULL;
	return 0;
}