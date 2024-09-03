#ifndef EDITBUF_H
#define EDITBUF_H

#include "e2.h"
#include "pt.h"

/* Edit buffer status flags. */
enum {
	EB_DIRTY = 1 << 0,
	EB_RDONLY = 1 << 1,
	EB_MARKSET = 1 << 2,
};

struct editbuf {
	/* An editor buffer. Buffers typically correspond to a file. However
	 * newly created bufers may be "unassociated" until they are saved.
	 */

	size_t lines;		/* Number of lines in buffer. */

	struct line *top;	/* Top line. */
	struct line *bot;	/* Bottom line. */

	char *file_path;	/* NULL if no file associated with
				 * buffer. */
	char flags;		/* State flags. */
	char fmt;		/* Line ending format. */

	struct point mark;	/* position of the mark anchor */

	/* Current cursor location. */
	struct line *ln;	/* Current line. */
	struct point cursor;	/* position in buffer of the user's
				   cursor. known as the "point". */

	/* mlb: Editor buffers are to be stored in a circular
	 * doubly-linked list. */
	struct editbuf *next;
	struct editbuf *prev;
};

#define eb_rgn_marked(eb) ((eb)->flags & EB_MARKSET)

static inline void eb_set_region_marked(struct editbuf *eb, bool yes)
{
	if (yes)
		eb->flags |= EB_MARKSET;
	else
		eb->flags &= ~EB_MARKSET;
}

struct editbuf *eb_alloc_empty();
void eb_free(struct editbuf *eb);
int eb_emptybuf(struct editbuf *eb);
int eb_at_bot(struct editbuf *eb);
size_t eb_calc_size(struct editbuf *eb);
int eb_buf_info_cmd(void);
struct line *eb_find_first_visible_line(struct editbuf *eb);
struct line *eb_get_line_at(struct editbuf *eb, int num);
struct line *eb_move_up_nlines(struct editbuf *eb, int *num);

#endif
