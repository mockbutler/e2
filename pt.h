/* Copyright (c) 2006 Marc Butler */
#ifndef PT_H
#define PT_H

/* points are used to represent positions in the editing buffer. */

struct point {
	size_t line;
	size_t col;
};

#define pt_copy(pdst, psrc) memcpy(pdst, psrc, sizeof (struct point))

int pt_eq(struct point *p1, struct point *p2);
void pt_assign(struct point *lhs, const struct point *rhs);
int pt_lt(struct point *p1, struct point *p2);
void pt_swap(struct point *p1, struct point *p2);
struct point *pt_dup(const struct point *p);

#endif
