#include "chibicc.h"

// Input string
static char *current_input;

// Reports an error and exits.
_Noreturn void
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
static _Noreturn void
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

_Noreturn void
error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror_at(loc, fmt, ap);
	va_end(ap);
}

_Noreturn void
error_tok(struct token *tok, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror_at(tok->loc, fmt, ap);
	va_end(ap);
}

// Consumes the current token if it matches `s`.
bool
equal(struct token *tok, char *s)
{
	return (int)strlen(s) == tok->len && !strncmp(tok->loc, s, tok->len);
}

// Ensures that the current token is `s`.
struct token *
skip(struct token *tok, char *s)
{
	if (!equal(tok, s))
		error_tok(tok, "expected '%s'", s);
	return tok->next;
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

static bool
startswith(char *p, char *q)
{
	return strncmp(p, q, strlen(q)) == 0;
}

// Returns true if c is valid as the first character of an identifier.
static bool
is_ident1(char c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// Returns true if c is valid as a non-first character of an identifier.
static bool
is_ident2(char c)
{
	return is_ident1(c) || ('0' <= c && c <= '9');
}

static void
convert_keywords(struct token *tok)
{
	for (struct token *t = tok; t->kind != TK_EOF; t = t->next)
		if (equal(t, "return"))
			t->kind = TK_RESERVED;
}

// Tokenizes `current_input` and returns new tokens.
struct token *
tokenize(char *p)
{
	current_input = p;
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

		// Identifier or keyword
		if (is_ident1(*p)) {
			char *q = p++;
			while (is_ident2(*p))
				p++;
			cur = new_token(TK_IDENT, cur, q, p - q);
			continue;
		}

		// Multi-letter punctuation
		if (startswith(p, "==") || startswith(p, "!=") ||
		    startswith(p, "<=") || startswith(p, ">=")) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		// Single-letter punctuation
		if (ispunct(*p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		error_at(p, "invalid token");
	}

	new_token(TK_EOF, cur, p, 0);
	convert_keywords(head.next);
	return head.next;
}
