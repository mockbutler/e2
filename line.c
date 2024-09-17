/* Copyright (c) 2006 Marc Butler */

#include <stdlib.h>
#include <string.h>

#include "cursor.h"
#include "debug.h"
#include "e2.h"
#include "eb.h"
#include "line.h"

struct line* ln_alloc(size_t cap)
{
    /* default to a capacity of one character. this allows efficient
       representation of empty lines without having to deal with crappy
       NULL ptrs.
     */
    if (cap == 0)
        cap = COLS;

    struct line* l = malloc(sizeof(struct line));
    ASSERT(l);
    l->text = malloc(cap);
    ASSERT(l->text);
    l->len = 0;
    l->cap = cap;
    memset(l->text, 0, l->cap);
    l->next = l->prev = NULL;
    return l;
}

void ln_grow_cap(struct line* ln, size_t reqcap)
{
    ln->text = realloc(ln->text, ln->cap + reqcap);
    ln->cap += reqcap;
}

struct line* ln_from_str(const char* s)
{
    long len = strlen(s);
    long cap = MAX(COLS, len + 1);

    struct line* ln = ln_alloc(cap);

    memmove(ln->text, s, len);
    ln->text[len] = 0;
    ln->len = len;
    ln->next = ln->prev = NULL;
    ln->cap = cap;
    return ln;
}

void ln_free(struct line* l)
{
    ASSERT(l);
    free(l->text);
    free(l);
}

struct line* ln_copy(struct line* l)
{
    ASSERT(l);
    struct line* lcopy = ln_alloc(l->len);
    memmove(lcopy->text, l->text, l->len);
    lcopy->len = l->len;
    return lcopy;
}

struct line* ln_partial_copy(struct line* l, int start, int end)
{
    ASSERT(l);
    ASSERT(start >= 0);
    ASSERT(end <= l->len);
    ASSERT(start <= end);

    struct line* lcopy = ln_alloc(end - start);
    ASSERT(lcopy);
    memmove(lcopy->text, &l->text[start], end - start);
    lcopy->len = end - start;

    return lcopy;
}

void ln_split(struct line* src, int where, struct line** front,
    struct line** back)
{
    ASSERT(src);
    ASSERT(where >= 0 && where <= src->len);
    ASSERT(front);
    ASSERT(back);

    *front = ln_alloc(where);
    if (where > 0) {
        memmove((*front)->text, src->text, where);
        (*front)->len = where;
    }

    int len = src->len - where;
    *back = ln_alloc(len);
    if (len > 0) {
        memmove((*back)->text, &src->text[where], len);
        (*back)->len = len;
    }
}

/*
 * Insert string at position.
 */
void ln_ins_str_at(struct line* l, unsigned where, const char* s, unsigned len)
{
    ASSERT(where <= l->len);

    if (len == 0) {
        return;
    }
    if (ln_avail(l) < len) {
        ln_grow_cap(l, len);
    }

    size_t tlen = l->len - where;
    if (tlen > 0) {
        size_t toff = where + len;
        memmove(l->text + toff, l->text + where, tlen);
    }
    memmove(l->text + where, s, len);
    l->len += len;
}

/*
 * Erase region.
 */
void ln_erase_rgn(struct line *l, unsigned from, unsigned len)
{
    if ((from + len) == l->len) {
        memset(&l->text[from], 0, len);
    } else {
        unsigned tail_len = l->len - (from + len);
        memmove(&l->text[from], &l->text[from + len], tail_len);
        memset(&l->text[from + tail_len], 0, l->len - tail_len);
    }
    l->len -= len;
}