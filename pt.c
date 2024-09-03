/* Copyright (c) 2006 Marc Butler */

/* functions used to handle the point data type. */

#include <string.h>
#include <stdlib.h>

#include "pt.h"

int pt_eq(struct point *p1, struct point *p2)
{
	return (p1->line == p2->line) && (p1->col == p2->col);
}

void pt_assign(struct point *lhs, const struct point *rhs)
{
	lhs->line = rhs->line;
	lhs->col = rhs->col;
}

int pt_lt(struct point *p1, struct point *p2)
{
	if (p1->line < p2->line)
		return 1;
	if (p1->line == p2->line && p1->col < p2->col)
		return 1;
	return 0;
}

void pt_swap(struct point *p1, struct point *p2)
{
	struct point tmp;
	memcpy(&tmp, p1, sizeof(struct point));
	memcpy(p1, p2, sizeof(struct point));
	memcpy(p2, &tmp, sizeof(struct point));
}

struct point *pt_dup(const struct point *p)
{
	struct point *ptnew;
	ptnew = malloc(sizeof(struct point));
	memcpy(ptnew, p, sizeof(struct point));
	return ptnew;
}
