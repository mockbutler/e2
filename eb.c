/* Copyright (c) 2006 Marc Butler */

#include <stdlib.h>

#include "debug.h"
#include "e2.h"
#include "eb.h"
#include "line.h"

struct editbuf* eb_alloc_empty()
{
    struct editbuf* eb = malloc(sizeof(struct editbuf));
    ASSERT(eb);
    eb->line_cnt = 0;
    eb->top = eb->bot = NULL;
    eb->file_path = NULL;
    eb->flags = 0;
    eb->fmt = EB_FMT_UNIX;
    eb->ln = NULL;
    eb->cursor.col = 0;
    eb->cursor.line = 0;

    eb->ln = ln_alloc(COLS + 1);
    eb->top = eb->ln;
    eb->bot = eb->ln;
    eb->line_cnt = 1;
    eb->next = eb->prev = NULL;

    return eb;
}

void eb_free(struct editbuf* eb)
{
    struct line* ln;
    if (eb->file_path) {
        free(eb->file_path);
    }
    ln = eb->top;
    while (ln) {
        struct line* trash = ln;
        ln = ln->next;
        ln_free(trash);
    }
    free(eb);
}

/** Returns non-zero if buffer is empty. */
int eb_emptybuf(struct editbuf* eb)
{
    return (eb->line_cnt == 1 && eb->ln->len == 0);
}

int eb_at_bot(struct editbuf* eb)
{
    return (eb->ln->next == NULL);
}

size_t eb_calc_size(struct editbuf* eb)
{
    /* calculate the size of the current buffer */
    size_t size = 0;
    struct line* l = eb->top;

    if (eb_emptybuf(eb)) {
        return 0;
    }

    while (l != NULL) {
        size += l->len;
        l = l->next;
    }

    switch (eb->fmt) {
    case EB_FMT_MAC: /* fall thru */
    case EB_FMT_UNIX:
        size += eb->line_cnt;
        break;
    case EB_FMT_WIN:
        size += eb->line_cnt * 2;
    }

    return size;
}

int eb_buf_info_cmd(void)
{
    showmsg("Size = %lu Path = %s\n",
        eb_calc_size(curr_buf), curr_buf->file_path);
    return 1;
}

struct line* eb_find_first_visible_line(struct editbuf* eb)
{
    int sy, sx, y;
    struct line* ln;
    getyx(editwin, sy, sx);
    ASSERT(sy <= eb->cursor.line);
    ln = eb->ln;
    for (y = sy; y > 0; y--) {
        ln = ln->prev;
        ASSERT(ln != NULL);
    }
    return ln;
}

struct line* eb_get_line_at(struct editbuf* eb, int num)
{
    struct line* ln;
    int lnum;

    if (eb->cursor.line < num) {
        lnum = eb->cursor.line;
        ln = eb->ln;
    } else {
        lnum = 0;
        ln = eb->top;
    }

    while (lnum++ < num) {
        ln = ln->next;
        ASSERT(ln);
    }

    return ln;
}

/*
 * Return line above current line.
 * @param eb Must not be null.
 * @param[in, out] num Number of lines to move up, returns number of lines
 *      moved up.
 */
struct line* eb_move_up_nlines(struct editbuf* eb, int* num)
{
    ASSERT(*num && *num > 0);
    ASSERT(eb);

    struct line* ln = eb->ln;
    int n = 0;
    while (n < *num && ln->prev != NULL) {
        n++;
        ln = ln->prev;
    }

    *num = n;
    return ln;
}

/*
 * Delete current line.
 */
void eb_delete_current_line(struct editbuf *eb)
{
    struct line *todel = eb->ln;
    if (todel->next != NULL) {
        /* Delete current line bring up next line. */
        eb->ln = todel->next;
        eb->ln->prev = todel->prev;
        if (eb->ln->prev->next != NULL) {
            eb->ln->prev->next = eb->ln;
        }
        if (eb->ln->next != NULL) {
            eb->ln->next->prev = eb->ln;
        }
        ln_free(todel);
    } else {
        ln_erase_rgn(curr_line, 0, curr_line->len);
    }

    /* Snap cursor to end of line. */
    eb->cursor.col = MIN(eb->ln->len, eb->cursor.col);
}