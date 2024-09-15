#pragma once

#include "eb.h"
#include "line.h"

static inline bool eb_at_bol(struct editbuf* eb)
{
    return eb->cursor.col == 0;
}

static inline bool eb_at_eol(struct editbuf* eb)
{
    return eb->cursor.col == eb->ln->len;
}

static inline bool eb_at_tob(struct editbuf* eb)
{
    return eb->ln == eb->top;
}

static inline bool eb_at_bob(struct editbuf* eb)
{
    return eb->ln == eb->bot;
}
