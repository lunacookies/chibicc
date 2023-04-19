#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
		return 1;
	}

	char *p = argv[1];

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");
	printf("\tmov\trax, %ld\n", strtol(p, &p, 10));

	while (*p) {
		if (*p == '+') {
			p++;
			printf("\tadd\trax, %ld\n", strtol(p, &p, 10));
			continue;
		}

		if (*p == '-') {
			p++;
			printf("\tsub\trax, %ld\n", strtol(p, &p, 10));
			continue;
		}

		fprintf(stderr, "unexpected character: '%c'\n", *p);
		return 1;
	}

	printf("\tret\n");
}
