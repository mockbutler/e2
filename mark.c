/* Copyright (c) 2006 Marc Butler */

#include <string.h>

#include "debug.h"
#include "e2.h"
#include "eb.h"
#include "line.h"

#include "mark.h"

int mrk_set_cmd(void)
{
	pt_assign(&curr_buf->mark, &curr_buf->cursor);
	curr_buf->flags |= EB_MARKSET;
	return 1;
}

static void draw_whole_line_marked(int sy, struct line *ln)
{
	wattron(editwin, A_REVERSE);
	mvwaddnstr(editwin, sy, 0, ln->text, ln->len);
	wattroff(editwin, A_REVERSE);
}

static void draw_part_line_marked(int sy, struct line *ln,
				  long startcol, long endcol)
{
	int len;

	if (startcol - endcol == 0) {
		mvwaddnstr(editwin, sy, 0, ln->text, ln->len);
		return;
	}

	ASSERT(startcol < endcol);
	if (startcol == 0 && endcol == ln->len) {
		draw_whole_line_marked(sy, ln);
		return;
	}

	wclrtoeol(editwin);
	/* marked region is on the current line only! */

	/* draw text on the line before the marked text */
	if (startcol > 0) {
		len = startcol;
		mvwinsnstr(editwin, sy, 0, &ln->text[0], len);
	}

	/* draw marked text */
	wattron(editwin, A_REVERSE);
	len = endcol - startcol;
	ASSERT(len > 0);
	mvwinsnstr(editwin, sy, (int)startcol, &ln->text[startcol], len);
	wattroff(editwin, A_REVERSE);

	/* draw text on line after the marked text */
	if (endcol < curr_buf->ln->len) {
		len = endcol - ln->len;
		mvwinsnstr(editwin, sy, (int)endcol, &ln->text[endcol], len);
	}
}

void draw_marked_region(void)
{
	if ((curr_buf->flags & EB_MARKSET) == 0)
		return;

	if (pt_eq(&curr_buf->mark, &curr_buf->cursor))
		return;

	/* handle edge cases */

	int sy, sx;
	getyx(editwin, sy, sx);

	/* the cursor is at the top right position of the screen and the
	   mark is above the cursor position */
	if ((sy == 0 && sx == 0)
	    && (curr_buf->mark.line < curr_buf->cursor.line))
		return;

	/* the cursor is at the bottom of the screen and the mark is below
	   the cursor */
	if ((sy == EBWINSIZE && sx == COLS)
	    && (curr_buf->mark.line > curr_buf->cursor.line))
		return;

	/* cursor and mark are in the same postion so nothing to
	   highlight */
	if (pt_eq(&curr_buf->mark, &curr_buf->cursor))
		return;

	/* find the start and end of the region to be hilighted */
	struct point mstart, mend;
	pt_copy(&mstart, &curr_buf->mark);
	pt_copy(&mend, &curr_buf->cursor);
	if (pt_lt(&mend, &mstart)) {
		pt_swap(&mstart, &mend);
	}

	/* just highlight a sub-part of the current line */
	if (mstart.line == mend.line) {
		wmove(editwin, sy, 0);
		draw_part_line_marked(sy, curr_buf->ln, mstart.col, mend.col);
		touchline(editwin, sy, 1);
		wmove(editwin, sy, sx);
		return;
	}

	struct line *ln;
	long sline;
	long lnum;
	ln = eb_find_first_visible_line(curr_buf);
	lnum = curr_buf->cursor.line - sy;
	werase(editwin);
	wmove(editwin, 0, 0);
	for (sline = 0; ln && sline < EBWINSIZE; sline++, lnum++, ln = ln->next) {
		if (ln_empty(ln))
			continue;

		if ((lnum > mstart.line) && (lnum < mend.line)) {
			draw_whole_line_marked(sline, ln);
		} else if (lnum == mstart.line) {
			wmove(editwin, sline, 0);
			draw_part_line_marked(sline, ln, mstart.col, ln->len);
			wmove(editwin, sline, sx);
		} else if (lnum == mend.line) {
			wmove(editwin, sline, 0);
			draw_part_line_marked(sline, ln, 0, mend.col);
			wmove(editwin, sline, sx);
		} else {
			mvwaddnstr(editwin, sline, 0, ln->text, ln->len);
		}
	}

	wmove(editwin, sy, sx);	/* restore the original cursor location */
}
