#include <stdio.h>
#include <stdlib.h>
void error_and_exit(char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(EXIT_FAILURE);
}
