#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node;

//
// tokenize.c
//

enum token_kind {
	TK_RESERVED, // keywords or punctuation
	TK_IDENT,    // identifiers
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

struct var {
	struct var *next;
	char *name; // variable name
	int offset; // offset from rbp
};

struct function {
	struct node *body;
	struct var *locals;
	int stack_size;
};

enum node_kind {
	ND_ADD,       // +
	ND_SUB,       // -
	ND_MUL,       // *
	ND_DIV,       // /
	ND_EQ,        // ==
	ND_NE,        // !=
	ND_LT,        // <
	ND_LE,        // <=
	ND_ASSIGN,    // =
	ND_RETURN,    // “return”
	ND_BLOCK,     // { ... }
	ND_EXPR_STMT, // expression statement
	ND_VAR,       // variable
	ND_NUM,       // integer
};

struct node {
	enum node_kind kind; // node kind
	struct node *next;   // next node
	struct node *lhs;    // left-hand side
	struct node *rhs;    // right-hand side

	// Block
	struct node *body;

	struct var *var; // used if kind == ND_VAR
	int val;         // used if kind == ND_NUM
};

struct function *
parse(struct token *tok);

//
// codegen.c
//

void
codegen(struct function *prog);
