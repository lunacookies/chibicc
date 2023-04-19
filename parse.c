#include "chibicc.h"

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

struct node *
parse(struct token *tok)
{
	struct node *node = expr(&tok, tok);
	if (tok->kind != TK_EOF)
		error_tok(tok, "extra token");
	return node;
}
