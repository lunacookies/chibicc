#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum token_kind {
	TK_RESERVED, // keywords or punctuation
	TK_NUM,      // numeric literals
	TK_EOF,      // end-of-file markers
};

struct token {
	enum token_kind kind; // token kind
	struct token *next;   // next token
	int val;              // if kind is TK_NUM, its value
	char *loc;            // token location
	int len;              // token length
};

// Input string
static char *current_input;

// Reports an error and exits.
static void
error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(1);
}

// Reports an error location and exits.
static void
verror_at(char *loc, char *fmt, va_list ap)
{
	int pos = loc - current_input;
	fprintf(stderr, "%s\n", current_input);
	fprintf(stderr, "%*s", pos, ""); // print `pos` spaces.
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

static void
error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror_at(loc, fmt, ap);
	va_end(ap);
}

static void
error_tok(struct token *tok, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror_at(tok->loc, fmt, ap);
	va_end(ap);
}

// Consumes the current token if it matches `s`.
static bool
equal(struct token *tok, char *s)
{
	return (int)strlen(s) == tok->len && !strncmp(tok->loc, s, tok->len);
}

// Ensures that the current token is `s`.
static struct token *
skip(struct token *tok, char *s)
{
	if (!equal(tok, s))
		error_tok(tok, "expected '%s'", s);
	return tok->next;
}

// Ensures that the current token is TK_NUM.
static int
get_number(struct token *tok)
{
	if (tok->kind != TK_NUM)
		error_tok(tok, "expected a number");
	return tok->val;
}

// Creates a new token and adds it as the next token of `cur`.
static struct token *
new_token(enum token_kind kind, struct token *cur, char *str, int len)
{
	struct token *tok = calloc(1, sizeof(struct token));
	tok->kind = kind;
	tok->loc = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

// Tokenizes `current_input` and returns new tokens.
static struct token *
tokenize(void)
{
	char *p = current_input;
	struct token head = {0};
	struct token *cur = &head;

	while (*p) {
		// Skip whitespace characters.
		if (isspace(*p)) {
			p++;
			continue;
		}

		// Numeric literal
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtoul(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		// Punctuation
		if (*p == '+' || *p == '-') {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		error_at(p, "invalid token");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

int
main(int argc, char **argv)
{
	if (argc != 2)
		error("%s: invalid number of arguments\n", argv[0]);

	current_input = argv[1];
	struct token *tok = tokenize();

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");

	// The first token must be a number ...
	printf("\tmov\trax, %d\n", get_number(tok));
	tok = tok->next;

	// ... followed by either `+ <number>` or `- <number>`.
	while (tok->kind != TK_EOF) {
		if (equal(tok, "+")) {
			printf("\tadd\trax, %d\n", get_number(tok->next));
			tok = tok->next->next;
			continue;
		}

		tok = skip(tok, "-");
		printf("\tsub\trax, %d\n", get_number(tok));
		tok = tok->next;
	}

	printf("\tret\n");
}
