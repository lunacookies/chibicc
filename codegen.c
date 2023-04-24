#include "chibicc.h"

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

// Compute the absolute address of a given node.
// It’s an error if a given node does not reside in memory.
static void
gen_addr(struct node *node)
{
	if (node->kind == ND_VAR) {
		int offset = (node->name - 'a' + 1) * 8;
		printf("\tlea\trax, [rbp - %d]\n", offset);
		return;
	}

	error("not an lvalue");
}

// Generate code for a given node.
static void
gen_expr(struct node *node)
{
	switch (node->kind) {
	case ND_NUM:
		printf("\tmov\trax, %d\n", node->val);
		return;
	case ND_VAR:
		gen_addr(node);
		printf("\tmov\trax, [rax]\n");
		return;
	case ND_ASSIGN:
		gen_addr(node->lhs);
		push();
		gen_expr(node->rhs);
		pop("rdi");
		printf("\tmov\t[rdi], rax\n");
		return;
	default:
		break;
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

static void
gen_stmt(struct node *node)
{
	if (node->kind == ND_EXPR_STMT) {
		gen_expr(node->lhs);
		return;
	}

	error("invalid statement");
}

void
codegen(struct node *node)
{
	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");

	// Prologue
	printf("\tpush\trbp\n");
	printf("\tmov\trbp, rsp\n");
	printf("\tsub\trsp, 208\n");

	for (struct node *n = node; n; n = n->next) {
		gen_stmt(n);
		assert(depth == 0);
	}

	printf("\tmov\trsp, rbp\n");
	printf("\tpop\trbp\n");
	printf("\tret\n");
}
