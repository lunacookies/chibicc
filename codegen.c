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

void
codegen(struct node *node)
{
	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");

	gen_expr(node);
	printf("\tret\n");

	assert(depth == 0);
}
