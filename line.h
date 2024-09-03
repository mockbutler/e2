/* Copyright (c) 2006 Marc Butler. */
#ifndef LINE_H
#define LINE_H

struct line {
	/* Holds the contents of single line. No line ending information
	 * is stored. */

	char *text;		/* ASCII; not '\0' terminated. */
	long len;		/* Length in bytes. */
	long cap;		/* Capacity in bytes. */

	/* Lines are stored in doubly-linked lists in edit buffers. */
	struct line *prev;
	struct line *next;
};

static inline bool ln_empty(struct line *ln)
{
	return ln->len == 0;
}

static inline bool ln_avail(struct line *ln)
{
	return ln-> cap - ln->len;
}

struct line *line_alloc(size_t cap);
void line_free(struct line *l);
struct line *line_from_str(const char *s);
void ln_del_curr(void);

struct line *ln_copy(struct line *l);
struct line *ln_partial_copy(struct line *l, int start, int end);
void ln_ins_str_at(struct line *l, unsigned where, const char *s, unsigned len);

#endif
