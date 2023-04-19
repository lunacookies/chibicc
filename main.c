#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Tokenizer
//

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
static _Noreturn void
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

static _Noreturn void
error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror_at(loc, fmt, ap);
	va_end(ap);
}

static _Noreturn void
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
	return head.next;
}

//
// Parser
//

enum node_kind {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_EQ,  // ==
	ND_NE,  // !=
	ND_LT,  // <
	ND_LE,  // <=
	ND_NUM, // Integer
};

struct node {
	enum node_kind kind; // node kind
	struct node *lhs;    // left-hand side
	struct node *rhs;    // right-hand side
	int val;             // used if kind == ND_NUM
};

static struct node *
new_node(enum node_kind kind)
{
	struct node *node = calloc(1, sizeof(struct node));
	node->kind = kind;
	return node;
}

static struct node *
new_binary(enum node_kind kind, struct node *lhs, struct node *rhs)
{
	struct node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

static struct node *
new_num(int val)
{
	struct node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

static struct node *
expr(struct token **rest, struct token *tok);

static struct node *
equality(struct token **rest, struct token *tok);

static struct node *
relational(struct token **rest, struct token *tok);

static struct node *
add(struct token **rest, struct token *tok);

static struct node *
mul(struct token **rest, struct token *tok);

static struct node *
unary(struct token **rest, struct token *tok);

static struct node *
primary(struct token **rest, struct token *tok);

// expr = equality
static struct node *
expr(struct token **rest, struct token *tok)
{
	return equality(rest, tok);
}

// equality = relational ("==" relational | "!=" relational)*
static struct node *
equality(struct token **rest, struct token *tok)
{
	struct node *node = relational(&tok, tok);

	for (;;) {
		if (equal(tok, "==")) {
			struct node *rhs = relational(&tok, tok->next);
			node = new_binary(ND_EQ, node, rhs);
			continue;
		}

		if (equal(tok, "!=")) {
			struct node *rhs = relational(&tok, tok->next);
			node = new_binary(ND_NE, node, rhs);
			continue;
		}

		*rest = tok;
		return node;
	}
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static struct node *
relational(struct token **rest, struct token *tok)
{
	struct node *node = add(&tok, tok);

	for (;;) {
		if (equal(tok, "<")) {
			struct node *rhs = add(&tok, tok->next);
			node = new_binary(ND_LT, node, rhs);
			continue;
		}

		if (equal(tok, "<=")) {
			struct node *rhs = add(&tok, tok->next);
			node = new_binary(ND_LE, node, rhs);
			continue;
		}

		if (equal(tok, ">")) {
			struct node *rhs = add(&tok, tok->next);
			node = new_binary(ND_LT, rhs, node);
			continue;
		}

		if (equal(tok, ">=")) {
			struct node *rhs = add(&tok, tok->next);
			node = new_binary(ND_LE, rhs, node);
			continue;
		}

		*rest = tok;
		return node;
	}
}

// add = mul ("+" mul | "-" mul)*
static struct node *
add(struct token **rest, struct token *tok)
{
	struct node *node = mul(&tok, tok);

	for (;;) {
		if (equal(tok, "+")) {
			struct node *rhs = mul(&tok, tok->next);
			node = new_binary(ND_ADD, node, rhs);
			continue;
		}

		if (equal(tok, "-")) {
			struct node *rhs = mul(&tok, tok->next);
			node = new_binary(ND_SUB, node, rhs);
			continue;
		}

		*rest = tok;
		return node;
	}
}

// mul = unary ("*" unary | "/" unary)*
static struct node *
mul(struct token **rest, struct token *tok)
{
	struct node *node = unary(&tok, tok);

	for (;;) {
		if (equal(tok, "*")) {
			struct node *rhs = unary(&tok, tok->next);
			node = new_binary(ND_MUL, node, rhs);
			continue;
		}

		if (equal(tok, "/")) {
			struct node *rhs = unary(&tok, tok->next);
			node = new_binary(ND_DIV, node, rhs);
			continue;
		}

		*rest = tok;
		return node;
	}
}

// unary = ("+" | "-") unary
//       | primary
static struct node *
unary(struct token **rest, struct token *tok)
{
	if (equal(tok, "+"))
		return unary(rest, tok->next);

	if (equal(tok, "-"))
		return new_binary(ND_SUB, new_num(0), unary(rest, tok->next));

	return primary(rest, tok);
}

// primary = "(" expr ")" | num
static struct node *
primary(struct token **rest, struct token *tok)
{
	if (equal(tok, "(")) {
		struct node *node = expr(&tok, tok->next);
		*rest = skip(tok, ")");
		return node;
	}

	if (tok->kind == TK_NUM) {
		struct node *node = new_num(tok->val);
		*rest = tok->next;
		return node;
	}

	error_tok(tok, "expected an expression");
}

//
// Code generator
//

static int depth;

static void
push(void)
{
	printf("\tpush\trax\n");
	depth++;
}

static void
pop(char *arg)
{
	printf("\tpop\t%s\n", arg);
	depth--;
}

static void
gen_expr(struct node *node)
{
	if (node->kind == ND_NUM) {
		printf("\tmov\trax, %d\n", node->val);
		return;
	}

	gen_expr(node->rhs);
	push();
	gen_expr(node->lhs);
	pop("rdi");

	switch (node->kind) {
	case ND_ADD:
		printf("\tadd\trax, rdi\n");
		return;
	case ND_SUB:
		printf("\tsub\trax, rdi\n");
		return;
	case ND_MUL:
		printf("\timul\trax, rdi\n");
		return;
	case ND_DIV:
		printf("\tcqo\n");
		printf("\tidiv\trdi\n");
		return;

	case ND_EQ:
	case ND_NE:
	case ND_LT:
	case ND_LE:
		printf("\tcmp\trax, rdi\n");

		if (node->kind == ND_EQ)
			printf("\tsete\tal\n");
		else if (node->kind == ND_NE)
			printf("\tsetne\tal\n");
		else if (node->kind == ND_LT)
			printf("\tsetl\tal\n");
		else if (node->kind == ND_LE)
			printf("\tsetle\tal\n");

		printf("\tmovzx\trax, al\n");
		return;

	default:
		error("invalid expression");
	}
}

int
main(int argc, char **argv)
{
	if (argc != 2)
		error("%s: invalid number of arguments\n", argv[0]);

	// Tokenize and parse.
	current_input = argv[1];
	struct token *tok = tokenize();
	struct node *node = expr(&tok, tok);

	if (tok->kind != TK_EOF)
		error_tok(tok, "extra token");

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");

	// Traverse the AST to emit assembly.
	gen_expr(node);
	printf("\tret\n");

	assert(depth == 0);
}
