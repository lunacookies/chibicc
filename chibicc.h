#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
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

_Noreturn void
error(char *fmt, ...);

_Noreturn void
error_at(char *loc, char *fmt, ...);

_Noreturn void
error_tok(struct token *tok, char *fmt, ...);

bool
equal(struct token *tok, char *op);

struct token *
skip(struct token *tok, char *op);

struct token *
tokenize(char *input);

//
// parse.c
//

enum node_kind {
	ND_ADD,       // +
	ND_SUB,       // -
	ND_MUL,       // *
	ND_DIV,       // /
	ND_EQ,        // ==
	ND_NE,        // !=
	ND_LT,        // <
	ND_LE,        // <=
	ND_EXPR_STMT, // expression statement
	ND_NUM,       // integer
};

struct node {
	enum node_kind kind; // node kind
	struct node *next;   // next node
	struct node *lhs;    // left-hand side
	struct node *rhs;    // right-hand side
	int val;             // used if kind == ND_NUM
};

struct node *
parse(struct token *tok);

//
// codegen.c
//

void
codegen(struct node *node);
