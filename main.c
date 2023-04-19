#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
		return 1;
	}

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");
	printf("\tmov\trax, %d\n", atoi(argv[1]));
	printf("\tret\n");
}
