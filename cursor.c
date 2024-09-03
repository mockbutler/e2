/* Copyright (c) 2006 Marc Butler */
#include "e2.h"
#include "debug.h"

#include "cursor.h"

int cur_move(int y, int x)
{
	/* move cursor relative to it's curr pos. */
	int sy, sx;
	getyx(editwin, sy, sx);
	sy += y;
	sx += x;
	wmove(editwin, sy, sx);
	return 0;
}

int cur_pos(int y, int x)
{
	wmove(editwin, y, x);
	wrefresh(editwin);
	return 0;
}

int cur_posx(int x)
{
	/* move cursor to an absolute horiz. pos. */
	int sy, sx;
	getyx(editwin, sy, sx);
	wmove(editwin, sy, x);
	return 0;
}

void cur_getpos(struct pos *p)
{
	int sy, sx;
	getyx(editwin, sy, sx);
	p->line = sx;
	p->col = sy;
}

void cur_setpos(const struct pos *pt)
{
	/* todo: need to check for horiz. and vert. scrolling here! */
	wmove(editwin, pt->col, pt->line);
}
