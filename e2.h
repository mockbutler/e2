/* Copyright (c) 2006 Marc Butler */
#ifndef E2_H
#define E2_H

#include <ncurses.h>

#define flag_dirty() curr_buf->flags |= EB_DIRTY
#define UNUSED(v) v __attribute__((unused))
#define ARYSIZE(ary) (sizeof(ary) / (sizeof((ary)[0])))
#define EBWINSIZE (LINES - 2)

#define SPACES_PER_TAB 8
#define ACTION_MAP_SIZE 255

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

extern struct editbuf* curr_buf;
#define curr_line (curr_buf->ln)

enum {
    MOD_NONE,
    MOD_ESC,
    MOD_CTRLX,
    MOD_CTRLC,
};

/* Line ending format of the buffer. */
enum {
    EB_FMT_UNIX = 'U',
    EB_FMT_WIN = 'W',
    EB_FMT_MAC = 'M'
};

struct cutting {
    int linecount;
    struct line* text;
};

static inline void swapint(int* a, int* b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void redraw(void);
void showmsg(const char* fmt, ...);

/* Editor action call-back. Usually these are initiated by the user
 * but may be called internally. */
typedef int (*action)(void);

extern WINDOW* editwin;

#endif
