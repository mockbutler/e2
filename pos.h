/* Copyright (c) 2006 Marc Butler */
#pragma once

/* points are used to represent positions in the editing buffer. */

struct pos {
	long line;
	long col;
};

#define pos_copy(pdst, psrc) memcpy(pdst, psrc, sizeof (struct pos))

int pos_eq(struct pos *p1, struct pos *p2);
void pos_assign(struct pos *lhs, const struct pos *rhs);
int pos_lt(struct pos *p1, struct pos *p2);
void pos_swap(struct pos *p1, struct pos *p2);
struct pos *pos_dup(const struct pos *p);
