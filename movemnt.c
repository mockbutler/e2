/* Copyright (c) 2006 Marc Butler */

#include "e2.h"
#include "line.h"
#include "eb.h"
#include "debug.h"
#include "editing.h"

#include "movemnt.h"

static void screenpos(struct pos *p)
{
	int sx, sy;
	getyx(editwin, sy, sx);
	ASSERT(sy >= 0 && sx >= 0);
	p->line = (size_t)sy;
	p->col = (size_t)sx;
}

int move_up(void)
{
	struct pos spos;
	if (eb_at_tob(curr_buf)) {
		return 1;
	}

	curr_buf->ln = curr_buf->ln->prev;
	if (curr_line->len <= curr_buf->cursor.col) {
		curr_buf->cursor.col = curr_line->len;
	}
	curr_buf->cursor.line--;

	screenpos(&spos);
	if (spos.line > 0) {
		wmove(editwin, spos.line - 1, curr_buf->cursor.col);
	} else {
		/* vertical scroll up */
		redraw();
	}
	return 1;
}

int move_left(void)
{
	struct pos spos;

	if (eb_at_bol(curr_buf) && eb_at_tob(curr_buf)) {
		flash();
		return -1;
	}

	screenpos(&spos);
	if (spos.col > 0) {
		wmove(editwin, spos.line, spos.col - 1);
		curr_buf->cursor.col--;
	} else {
		if (eb_at_bol(curr_buf)) {
			struct line *l;
			l = curr_buf->ln->prev;
			wmove(editwin, spos.line - 1, min((COLS - 1), l->len - 1));
			curr_buf->ln = l;
			curr_buf->cursor.col = l->len - 1;
			curr_buf->cursor.line--;
		} else {
			/* trigger horizontal scroll left */
		}
	}

	return 0;
}

int move_right(void)
{
	int sx, sy;

	if (eb_at_eol(curr_buf) && eb_at_bob(curr_buf)) {
		flash();
		return -1;
	}

	getyx(editwin, sy, sx);
	if (sx == COLS) {
		/* trigger horizontal scroll right */
	} else {
		if (!eb_at_eol(curr_buf)) {
			wmove(editwin, sy, sx + 1);
			curr_buf->cursor.col++;
		} else {
			struct line *l;
			l = curr_buf->ln->next;
			wmove(editwin, sy + 1, 0);
			curr_buf->ln = l;
			curr_buf->cursor.col = 0;
			curr_buf->cursor.line++;
		}
	}

	return 0;
}

int move_down()
{
	if (eb_at_bob(curr_buf)) {
		return -1;
	}

	int sx, sy;
	getyx(editwin, sy, sx);
	curr_buf->ln = curr_buf->ln->next;
	curr_buf->cursor.line++;

	if (curr_buf->ln->len <= curr_buf->cursor.col) {
		curr_buf->cursor.col = curr_buf->ln->len;
		sx = curr_buf->ln->len;
	}

	if (sy < LINES - 3) {
		sy += 1;
		wmove(editwin, sy, curr_buf->cursor.col);
	} else {
		redraw();
	}
	return 0;
}

int move_eol()
{
	struct pos spos;
	screenpos(&spos);

	if (curr_buf->cursor.col <= curr_line->len) {
		size_t diff = curr_line->len - curr_buf->cursor.col;
		if (diff < ((size_t)COLS - spos.col)) {
			wmove(editwin, spos.line, spos.col + diff);
			curr_buf->cursor.col = curr_line->len;
		} else {
			/* scroll vertically */
		}
	} else {
		/* snap back to line end */
	}
	wrefresh(editwin);
	return 0;
}

int move_bol()
{
	if (curr_buf->cursor.col == 0)
		return -1;
	else {
		struct pos spos;
		screenpos(&spos);
		if (curr_buf->cursor.col == spos.col) {
			wmove(editwin, spos.line, 0);
			curr_buf->cursor.col = 0;
		} else {
			/* horizonal scroll */
		}
	}
	wrefresh(editwin);
	return 0;
}
