#pragma once
// Copyright (c) 2006 Marc Butler

#include <string>
#include <iosfwd>

#include "e2.hh"
#include "pos.hh"

enum {
    EB_DIRTY = 1 << 0,      // Modified.
    EB_RDONLY = 1 << 1,     // Read only.
    EB_MARKSET = 1 << 2,    // Mark is active.
};

struct editbuf {
    long line_cnt; /* Number of lines in buffer. */

    struct line* top; /* Top line. */
    struct line* bot; /* Bottom line. */

    std::string file_path;
    char flags; /* State flags. */
    char fmt; /* Line ending format. */

    struct pos mark; /* position of the mark anchor */

    /* Current cursor location. */
    struct line* ln; /* Current line. */
    struct pos cursor;

    void insert(char);
    void outputToStream(std::ostream&);

    bool regionIsActive() const;
    void regionMarkActive();
    void regionMarkInactive();
    bool isDirty();
    void markDirty();
};

#include "eb.inl"

#define eb_rgn_marked(eb) (eb)->regionIsActive()

static inline void eb_set_region_marked(struct editbuf* eb, bool yes)
{
    if (yes)
        eb->regionMarkActive();
    else
        eb->regionMarkInactive();
}

struct editbuf* eb_alloc_empty();
void eb_free(struct editbuf* eb);
int eb_emptybuf(struct editbuf* eb);
int eb_at_bot(struct editbuf* eb);
size_t eb_calc_size(struct editbuf* eb);
int eb_buf_info_cmd(void);
struct line* eb_find_first_visible_line(struct editbuf* eb);
struct line* eb_get_line_at(struct editbuf* eb, int num);
struct line* eb_move_up_nlines(struct editbuf* eb, int* num);
void eb_delete_current_line(struct editbuf* eb);
