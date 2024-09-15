/* Copyright (c) 2006 Marc Butler */
#include <string.h>

#include "cursor.h"
#include "debug.h"
#include "e2.h"
#include "eb.h"
#include "line.h"

#include "search.h"

#define SEARCH_MAX 127

static char last_fwdsearch[SEARCH_MAX + 1] = { 0 };

extern int minibuf_edit(const char* prompt, char* resp, unsigned respmax);

int searchfwd_cmd(void)
{
    char str[127 + 1] = { 0 };

    if (strlen(last_fwdsearch) > 0)
        strcpy(str, last_fwdsearch);

    if (minibuf_edit("Search: ", str, 127) == 0) {
        if (strlen(str) > 0) {
            searchfwd(str);
            strcpy(last_fwdsearch, str);
        }
    }
    return 1;
}

int searchfwd(const char* str)
{
    char *anchor, *pos;
    struct line* start_line = curr_line;

    t_print("search fwd = '%s'\n", str);

    if (eb_emptybuf(curr_buf))
        return 1;

    /* search the current line for the pattern */
    anchor = &(curr_line->text[curr_buf->cursor.col]);
    pos = strstr(anchor, str);
    if (pos != NULL && pos != anchor) {
        cur_move(0, pos - anchor);
        curr_buf->cursor.col += pos - anchor;
        return 1;
    }

    /* search through the buffer wrapping around at the end of the
     * buffer.
     */
    for (curr_line = curr_line->next;
         curr_line != start_line;
         curr_line = curr_line->next, curr_buf->cursor.line++) {
        /* wrap at bottom */
        if (curr_line == NULL) {
            curr_line = curr_buf->top;
            curr_buf->cursor.line = 1;
        }

        if (!ln_empty(curr_line)) {
            anchor = &curr_line->text[0];
            pos = strstr(anchor, str);
            if (pos != NULL) {
                curr_buf->cursor.col = pos - anchor;
                cur_posx(pos - anchor);
                redraw();
                break;
            }
        }
    }

    return 1;
}
