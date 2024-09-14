/* Copyright (c) 2006 Marc Butler */

/* functions used to handle the point data type. */

#include <string.h>
#include <stdlib.h>

#include "pos.h"

int pos_eq(struct pos *p1, struct pos *p2)
{
	return (p1->line == p2->line) && (p1->col == p2->col);
}

void pos_assign(struct pos *lhs, const struct pos *rhs)
{
	lhs->line = rhs->line;
	lhs->col = rhs->col;
}

int pos_lt(struct pos *p1, struct pos *p2)
{
	if (p1->line < p2->line)
		return 1;
	if (p1->line == p2->line && p1->col < p2->col)
		return 1;
	return 0;
}

void pos_swap(struct pos *p1, struct pos *p2)
{
	struct pos tmp;
	memmove(&tmp, p1, sizeof(struct pos));
	memmove(p1, p2, sizeof(struct pos));
	memmove(p2, &tmp, sizeof(struct pos));
}

struct pos *pos_dup(const struct pos *p)
{
	struct pos *ptnew;
	ptnew = malloc(sizeof(struct pos));
	memmove(ptnew, p, sizeof(struct pos));
	return ptnew;
}
