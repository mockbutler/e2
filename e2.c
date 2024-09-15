/* Copyright (c) 2006 Marc Butler */

#include <ctype.h>
#include <curses.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cursor.h"
#include "debug.h"
#include "eb.h"
#include "editing.h"
#include "keycodes.h"
#include "line.h"
#include "mark.h"
#include "movemnt.h"
#include "search.h"

#include "e2.h"

WINDOW* status = NULL;
static char* sline = NULL;
static size_t sline_size = 0;
WINDOW* cmdwin = NULL;
WINDOW* editwin = NULL;
struct editbuf* curr_buf = NULL;
FILE* dribble = NULL;
int mod = MOD_NONE;
int last_key = -1;
int curr_key = -1;

struct cutting g_cutting = { 0, NULL };

action map_def[ACTION_MAP_SIZE] = {};
action map_esc[ACTION_MAP_SIZE] = {};
action map_ctrlx[ACTION_MAP_SIZE] = {};
action map_ctrlc[ACTION_MAP_SIZE] = {};

void startup();
void shutdown();
void setup_status();
void status_update(struct editbuf* eb);
void setup_editwin();
void display_err(const char* msg);
void insert(struct editbuf* eb, int ch);
void fix_cursor();
void handle_key(int key);
void add_line(void);

void save_editbuf(struct editbuf* eb);
int backspace();
void redraw(void);
void open_dribble();
void close_dribble();
void remove_line(struct line* ln);
struct editbuf* load(const char* path);
void append_line(struct editbuf* eb, struct line* ln);
void join_lines(struct line* above, struct line* below);
void strpadleft(char* s, char pad, unsigned cnt);
void kill_line(int i);
int only_whitespace(long from, long to, struct line* ln);
void showmsg(const char* fmt, ...);
int plain_insert(void);
void setup_keymaps(void);
int exit_editor(void);

int load_file(void);
int minibuf_edit(const char* prompt, char* resp, long respmax);
int linekill(void);
void buf_stk_ins(struct editbuf* eb);
void buf_stk_next(void);
int buf_next(void);
int page_down(void);
int page_up(void);

int cmd_redraw(void);
int cmd_copy(void);
int cmd_tab(void);
int cmd_buf_beg(void);
int cmd_buf_end(void);
int cmd_paste(void);

int esc(void)
{
    mod |= MOD_ESC;
    return 0;
}

int newline(void)
{
    add_line();
    move_down();
    move_bol();
    redraw();
    flag_dirty();
    return 1;
}

int linekill(void)
{
    if (curr_line->len >= 0) {
        if (eb_at_bol(curr_buf) || only_whitespace(0, curr_buf->cursor.col, curr_buf->ln))
            kill_line(1);
        else
            kill_line(0);
        redraw();
    }
    return 1;
}

int savebuffer(void)
{
    save_editbuf(curr_buf);
    curr_buf->flags &= ~EB_DIRTY;
    return 1;
}

int kbd_quit(void)
{
    curr_buf->flags &= ~EB_MARKSET;
    redrawwin(editwin);
    redraw();
    return 1;
}

int set_ctrlx_mod(void)
{
    mod |= MOD_CTRLX;
    return 0;
}

int set_ctrlc_mod(void)
{
    t_print("ctrl-c modifer!\n");
    mod |= MOD_CTRLC;
    return 0;
}

int key_not_bound(void)
{
    const char* mapname = "DEFAULT";
    switch (mod) {
    case MOD_CTRLX:
        mapname = "CTRL-X";
        break;
    case MOD_CTRLC:
        mapname = "CTRL-C";
        break;
    case MOD_ESC:
        mapname = "ESC";
        break;
    }

    showmsg("Warning key %s %d (0x%x) not bound in %s map! ",
        keyname(curr_key), curr_key, curr_key, mapname);
    t_print("NOT BOUND MAP = %s KEY = %d (0x%x)\n", mapname, curr_key,
        curr_key);
    return 1;
}

int main(int argc, char** argv)
{
    int key;

#ifdef ENABLE_DRIBBLE
    open_dribble();
    atexit(close_dribble);
#endif

    startup();
    atexit(shutdown);

    /* install window resize handler */
    /* handle command line options if any */
    /* open any files if necessary */

    if (argc == 2) {
        struct editbuf* eb = load(argv[1]);
        if (eb == NULL) {
            printf("Error loading file: %s\n", argv[1]);
            exit(1);
        }
        buf_stk_ins(eb);
    } else {
        buf_stk_ins(eb_alloc_empty());
    }

    setup_status();
    setup_editwin();
    setup_keymaps();

    clear();
    refresh();
    status_update(curr_buf);
    redraw();
    wmove(editwin, 0, 0);
    fix_cursor();
    doupdate();

    /* main loop - exit by user key stroke is done in
     * handle_cmd_key(). */
    for (;;) {
        key = getch();
        handle_key(key);
        status_update(curr_buf);
        draw_marked_region();
        touchwin(editwin);
        fix_cursor();
        doupdate();
    }

    return 0;
}

void startup()
{
    if (initscr() == NULL) {
        puts("Fatal error unable to initialize curses!");
        exit(1);
    }
    noecho();
    nonl();
    /* cbreak(); */
    raw();
    intrflush(stdscr, FALSE);
    meta(stdscr, TRUE);
    keypad(stdscr, TRUE);
}

void shutdown()
{
    endwin();
}

void setup_status()
{
    status = newwin(1, COLS, LINES - 2, 0);
    ASSERT(status);
    wbkgdset(status, ' ');
    wattron(status, A_REVERSE);

    sline_size = sizeof(char) * (COLS + 1);
    sline = malloc(sline_size);
    ASSERT(sline);
    memset(sline, ' ', sline_size);
    sline[sline_size - 1] = 0;

    cmdwin = newwin(1, COLS, LINES - 1, 0);
    ASSERT(cmdwin);
    werase(cmdwin);
}

void status_update(struct editbuf* eb)
{
    char* path = (eb->file_path == NULL) ? "*unnamed*" : eb->file_path;
    char tmp[64];
    long len;

    len = snprintf(tmp, sizeof(tmp) - 1, "e2 %li:%li ", eb->cursor.line + 1, eb->cursor.col + 1);
    if (len < 12) {
        strpadleft(tmp, ' ', 12 - len);
    }

    memset(sline, ' ', sline_size);
    sline[sline_size - 1] = 0;
    mvwprintw(status, 0, 0, "%s", sline);
    wrefresh(status);
    mvwprintw(status, 0, 0, "%s [%c%c%c] %s", tmp,
        (eb->flags & EB_DIRTY) ? '*' : '-',
        (eb->flags & EB_RDONLY) ? 'R' : '-', eb->fmt, path);
    wrefresh(status);
}

void setup_editwin()
{
    editwin = newwin(LINES - 2, COLS, 0, 0);
    ASSERT(editwin);
    werase(editwin);
    wrefresh(editwin);
}

void setup_keymaps()
{
    static const char syms[] = " \t~!@#$%^&*()_+`-={}|[]\\:\";'<>?,./";
    static const char lalpha[] = "abcdefghijklmnopqrstuvxywz";
    static const char ualpha[] = "ABCDEFGHIJKLMNOPQRSTUVXYWZ";
    static const char digits[] = "0123456789";

    size_t i;

    /* initalize key mappings to safety default */
    for (i = 0; i < ACTION_MAP_SIZE; i++) {
        map_def[i] = key_not_bound;
        map_esc[i] = key_not_bound;
        map_ctrlx[i] = key_not_bound;
        map_ctrlc[i] = key_not_bound;
    }

    /* default key mappings */
#define MAPCHARSTR(chstr)                \
    for (i = 0; i < ARYSIZE(chstr); i++) \
    map_def[(int)chstr[i]] = plain_insert

    MAPCHARSTR(syms);
    MAPCHARSTR(lalpha);
    MAPCHARSTR(ualpha);
    MAPCHARSTR(digits);
#undef MAPCHARSTR

    map_def[CTRL_SPC] = mrk_set_cmd;
    map_def[ESC] = esc;
    map_def[ENTER] = newline;
    map_def[BKSPC] = backspace;
    map_def[CTRL_X] = set_ctrlx_mod;
    map_def[CTRL_C] = set_ctrlc_mod;
    map_def[CTRL_A] = move_bol;
    map_def[CTRL_E] = move_eol;
    map_def[CTRL_I] = cmd_tab;
    map_def[CTRL_P] = move_up;
    map_def[CTRL_L] = cmd_redraw;
    map_def[CTRL_N] = move_down;
    map_def[CTRL_F] = move_right;
    map_def[CTRL_B] = move_left;
    map_def[CTRL_K] = linekill;
    map_def[CTRL_G] = kbd_quit;
    map_def[CTRL_V] = page_down;
    map_def[CTRL_S] = searchfwd_cmd;
    map_def[CTRL_Y] = cmd_paste;

    /* ctrl-x key mappings */
    map_ctrlx[CTRL_C] = exit_editor;
    map_ctrlx[CTRL_F] = load_file;
    map_ctrlx[CTRL_S] = savebuffer;
    map_ctrlx[CTRL_G] = kbd_quit;
    map_ctrlx['b'] = buf_next;
    map_ctrlx['i'] = eb_buf_info_cmd;

    /* ctrl-c key mappings */
    map_ctrlc[CTRL_G] = kbd_quit;
    map_ctrlc['i'] = eb_buf_info_cmd;

    /* esc key mappings */
    map_esc[CTRL_G] = kbd_quit;
    map_esc['v'] = page_up;
    map_esc['w'] = cmd_copy;
    map_esc['<'] = cmd_buf_beg;
    map_esc['>'] = cmd_buf_end;
}

void display_err(const char* msg)
{
    werase(cmdwin);
    mvwprintw(cmdwin, 0, 0, "ERROR: %s\n", msg);
    wrefresh(cmdwin);
}

void insert(struct editbuf* eb, int ch)
{
    int x, y;
    struct line* l;

    l = eb->ln;
    ASSERT(eb->cursor.col <= l->len);

    /* inserting this char will exceed the line capacity: so
     * reallocate the line */
    if (l->len >= l->cap) {
        char* txt = realloc(eb->ln->text, eb->ln->cap * 2);
        ASSERT(txt);
        l->text = txt;
        l->cap *= 2;
    }

    if (eb->cursor.col < l->len) {
        size_t len = l->len - eb->cursor.col;
        /* memcpy must accomodate overlapping moves */
        memmove(&l->text[eb->cursor.col + 1], &l->text[eb->cursor.col],
            len);
        l->text[eb->cursor.col] = (char)ch;
    } else {
        /* special case: append the char to the line */
        l->text[eb->cursor.col] = (char)ch;
    }
    l->len += 1;
    eb->cursor.col += 1;

    getyx(editwin, y, x);
    /* if (x + 1 > COLS) trigger horizontal scrolling */
    winsch(editwin, ch);
    wmove(editwin, y, x + 1);
    wrefresh(editwin);
}

void fix_cursor()
{
    int x, y;
    /* ensure that the cursor remains in the current edit window */
    getyx(editwin, y, x);
    wmove(editwin, y, x);
    wrefresh(editwin);
}

int translate_key(int key)
{
    if (key < 256) {
        return key;
    }

    /* looks like it was translated by they keypad option - try
     * and translate it into a native emacs key-chord */
    switch (key) {
    case KEY_LEFT:
        key = CTRL_B;
        break;
    case KEY_RIGHT:
        key = CTRL_F;
        break;
    case KEY_END:
        key = CTRL_E;
        break;
    case KEY_HOME:
        key = CTRL_A;
        break;
    case KEY_UP:
        key = CTRL_P;
        break;
    case KEY_DOWN:
        key = CTRL_N;
        break;
    case KEY_BACKSPACE:
        key = BKSPC;
        break;
    case KEY_NPAGE:
        key = CTRL_V;
        break;
    case KEY_PPAGE:
        key = 'v';
        mod = MOD_ESC;
        break;
    default:
        showmsg("Keycode %d (0x%x) is too big!\n", key, key);
        t_print("keycode %d (0x%x) is too big!\n", key, key);
        status_update(curr_buf);
    }

    return key;
}

void handle_key(int key)
{
    action act = NULL;

    key = translate_key(key);
    /* key handling funcs - rely on this global state being correct! */
    last_key = curr_key;
    curr_key = key;

    switch (mod) {
    case MOD_ESC:
        act = map_esc[key];
        break;
    case MOD_CTRLX:
        act = map_ctrlx[key];
        break;
    case MOD_CTRLC:
        act = map_ctrlc[key];
        break;
    default:
        act = map_def[key];
    }

    if ((*act)()) {
        mod = MOD_NONE;
    }
    status_update(curr_buf);
}

void add_line()
{
    struct line* newln;

    newln = line_alloc(COLS + 1);
    ASSERT(newln);

    newln->next = curr_buf->ln->next;
    newln->prev = curr_buf->ln;
    curr_buf->ln->next = newln;
    if (newln->next) {
        newln->next->prev = newln;
    }

    if (curr_buf->cursor.col < curr_line->len) {
        size_t tlen = curr_line->len - curr_buf->cursor.col;
        size_t toff = curr_line->len - tlen;
        memmove(newln->text, &(curr_buf->ln->text[toff]), tlen);
        newln->len = tlen;
        curr_line->len -= tlen;
    }

    if (eb_at_bob(curr_buf)) {
        curr_buf->bot = newln;
    }

    curr_buf->lines++;
}

void save_editbuf(struct editbuf* eb)
{
    FILE* fh;
    char* path;
    struct line* l;

    /*ASSERT(eb->file_path != NULL); */
    path = (eb->file_path == NULL) ? "tmp.txt" : eb->file_path;

    fh = fopen(path, "wb");
    if (!fh) {
        display_err("Error saving file!");
        return;
    }

    l = eb->top;
    if (l->len > 0) {
        while (l) {
            fwrite(l->text, 1, l->len, fh);
            fputc('\n', fh);
            l = l->next;
        }
    }

    fclose(fh);
    showmsg("Saved file: %s", path);
}

int backspace()
{
    if (curr_buf->cursor.col > 0) {
        move_left();
        wdelch(editwin);
        if (curr_buf->cursor.col < (curr_line->len - 1)) {
            /* overlapping memcopy must be supported */
            char* to = &(curr_line->text[curr_buf->cursor.col]);
            char* from = to + 1;
            size_t len = curr_line->len - (curr_buf->cursor.col + 1);
            memmove(to, from, len);
        } else {
            curr_line->text[curr_buf->cursor.col] = 0;
        }
        curr_line->len--;
    } else if (!eb_at_tob(curr_buf)) {
        if (ln_empty(curr_line)) {
            ln_del_curr();
            curr_buf->cursor.col = curr_buf->ln->len;
            cur_move(-1, 0);
            cur_posx(curr_line->len);
        } else {
            size_t len = curr_line->prev->len;
            join_lines(curr_line->prev, curr_line);
            cur_move(-1, 0);
            cur_posx(len);
        }
    }

    redraw();
    return 1;
}

void redraw(void)
{
    t_called();

    int sy, sx;
    getyx(editwin, sy, sx);

    struct line* ln;
    /* find the first line to start the redraw with */
    if (curr_buf->cursor.line != 0) {
        ln = eb_find_first_visible_line(curr_buf);
    } else {
        ln = curr_buf->top;
    }

    int lnum;

    lnum = 0;
    werase(editwin);
    wmove(editwin, 0, 0);
    while (ln && (lnum < LINES)) {
        mvwaddnstr(editwin, lnum, 0, ln->text, ln->len);
        ln = ln->next;
        lnum++;
    }

    cur_pos(sy, sx); /* restore cursor position. */
    wrefresh(editwin);
}

void open_dribble()
{
    dribble = fopen("dribble.txt", "w");
    ASSERT(dribble != NULL);
}

void close_dribble()
{
    ASSERT(dribble);
    fclose(dribble);
}

void remove_line(struct line* ln)
{
    t_called();

    if (ln->prev) {
        ln->prev->next = ln->next;
    }
    if (ln->next) {
        ln->next->prev = ln->prev;
    }
}

struct editbuf* load(const char* path)
{
    FILE* fh;
    struct editbuf* eb;
#define MAX_LINE 1024
    char buf[MAX_LINE];
    size_t buflen;
    struct line* ln;

    fh = fopen(path, "r");
    if (!fh) {
        return NULL;
    }

    eb = malloc(sizeof(struct editbuf));
    ASSERT(eb);
    eb->lines = 0;
    eb->top = eb->bot = NULL;
    eb->file_path = NULL;
    eb->flags = 0;
    eb->fmt = 'U';
    eb->ln = NULL;
    eb->cursor.col = 0;
    eb->cursor.line = 0;

    while (1) {
        if (fgets(buf, MAX_LINE, fh) == NULL)
            break;

        buflen = strlen(buf);
        ASSERT(buflen < MAX_LINE - 1);

        /* SPECIAL CASE if max_line - 1 chars are read and buf
         * does not end in a newline. need to read again as
         * the line must exceed max_line - 1 chars. */

        buf[buflen - 1] = 0; /* remove newline */
        if (buflen - 1 == 0) {
            ln = line_alloc(COLS);
        } else {
            ln = line_from_str(buf);
        }
        ASSERT(ln);
        append_line(eb, ln);
    }

    if (feof(fh)) {
        eb->ln = eb->top;
        eb->file_path = strdup(path);
        fclose(fh);

        t_print("Successfully loaded file: %s\n", path);
        wmove(editwin, 0, 0);
        return eb;
    }

    eb_free(eb);
    return NULL;
}

void append_line(struct editbuf* eb, struct line* ln)
{
    if (eb->lines == 0) {
        eb->top = eb->bot = ln;
    } else {
        eb->bot->next = ln;
        ln->prev = eb->bot;
        eb->bot = ln;
    }
    eb->lines++;
}

void join_lines(struct line* above, struct line* below)
{
    /* copy any text in the below line to the above line */
    long space = above->cap - above->len;
    if (space < below->len) {
        size_t newcap = below->len - space;
        above->text = realloc(above->text, newcap);
        ASSERT(above->text);
        above->cap = newcap;
    }
    memmove(&(above->text[above->len]), &below->text[0], below->len);
    above->len += below->len;

    /* remove the below line from the dbly-linked list and destroy it */
    if (below == curr_buf->bot)
        curr_buf->bot = above;
    remove_line(below);
    line_free(below);
    curr_buf->lines--;

    curr_buf->ln = above;
    curr_buf->cursor.line--;
}

void strpadleft(char* s, char pad, unsigned cnt)
{
    char* p = s + strlen(s);
    while (cnt > 0) {
        *p++ = pad;
        cnt--;
    }
    *p = 0;
}

void kill_line(int whole)
{
    /* Erase from current position to the end of the line. */
    if (whole) {
        curr_line->text[0] = 0;
        curr_line->len = 0;
        curr_buf->cursor.col = 0;
    } else {
        curr_line->text[curr_buf->cursor.col] = 0;
        curr_line->len = curr_buf->cursor.col;
    }
}

int only_whitespace(long from, long to, struct line* ln)
{
    long i;
    t_print("only_whitespace() from = %li to = %li len = %li\n", from, to,
        ln->len);
    ASSERT(from < ln->len);
    ASSERT(to < ln->len);
    ASSERT(from < to);

    for (i = from; i < to; i++)
        if (ln->text[i] != ' ')
            return 0;

    return 1;
}

void showmsg(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    werase(cmdwin);
    wmove(cmdwin, 0, 0);
    vwprintw(cmdwin, fmt, ap);
    va_end(ap);
    wrefresh(cmdwin);
}

int plain_insert(void)
{
    insert(curr_buf, curr_key);
    curr_buf->flags |= EB_DIRTY;
    return 1;
}

int exit_editor(void)
{
    exit(0);
    return 0; /* never reached */
}

int load_file(void)
{
    char path[1024];
    getcwd(path, 1024);

    if (minibuf_edit("File file: ", path, 1023) != 0)
        return -1;

    if (access(path, F_OK) == 0) {
        /* mlb: load the file into a buffer and display that buffer */
        struct editbuf* ebnew = load(path);
        if (ebnew == NULL) {
            showmsg("Error loading file!");
            return -1;
        }

        buf_stk_ins(ebnew);
        clear();
        refresh();
        status_update(curr_buf);
        redraw();
        wmove(editwin, 0, 0);
        fix_cursor();
        doupdate();
    } else {
        /* mlb: create the file here if the dir path exists */
        showmsg("Cannot open: %s", path);
    }

    return -1;
}

int minibuf_edit(const char* prompt, char* resp, long respmax)
{
    int key, i, sx, sy;

    (void)respmax;

    werase(cmdwin);
    wmove(cmdwin, 0, 0);
    wprintw(cmdwin, "%s", prompt);

    if (strlen(resp) > 0)
        wprintw(cmdwin, "%s", resp);
    i = strlen(resp);

    wrefresh(cmdwin);
    key = translate_key(getch());
    while (key != CTRL_G && key != ENTER) {
        /* mlb: need to handle scrolling and editing */
        if (isprint(key)) {
            waddch(cmdwin, key);
            resp[i++] = key;
        } else {
            switch (key) {
            case BKSPC:
                resp[--i] = 0;
                getyx(cmdwin, sy, sx);
                wmove(cmdwin, sy, sx - 1);
                wdelch(cmdwin);
                break;
            }
        }

        wrefresh(cmdwin);
        key = translate_key(getch());
    }

    if (key != CTRL_G)
        resp[i] = 0;
    else
        resp[0] = 0;

    werase(cmdwin);
    wrefresh(cmdwin);
    return (key != CTRL_G) ? 0 : -1;
}

void buf_stk_ins(struct editbuf* eb)
{
    ASSERT(eb);
    eb->next = curr_buf;
    if (curr_buf)
        curr_buf->prev = eb;
    curr_buf = eb;
}

void buf_stk_next(void)
{
    if (!curr_buf->next) {
        /* wrap-around by searching through the list to the first buffer
         */
        while (curr_buf->prev) {
            curr_buf = curr_buf->prev;
        }
    } else {
        curr_buf = curr_buf->next;
    }
}

int buf_next(void)
{
    buf_stk_next();
    redraw();
    return 1;
}

int page_down(void)
{
    t_called();

    /* a page is considered to be the edit window height - 1.  as
     * that allows for 1 line of overlap.
     */
    int max, i;
    struct line* ln;

    /* mlb: jump out early if at end of buffer */

    max = EBWINSIZE - 1;
    ln = curr_buf->ln;
    for (i = 0; i < max; i++) {
        if (ln->next == NULL)
            break;
        else
            ln = ln->next;
    }

    curr_buf->ln = ln;
    curr_buf->cursor.line += i;
    redraw();
    fix_cursor();
    return 1;
}

int page_up(void)
{
    t_called();

    int cnt, max;
    struct line* ln;
    max = EBWINSIZE - 1; /* max # of lines to move up by. */
    cnt = max;
    ln = eb_move_up_nlines(curr_buf, &cnt);
    curr_buf->ln = ln;
    curr_buf->cursor.line -= cnt;

    t_var2i(cnt, max);
    if (cnt <= max) {
        long sx, sy;
        getyx(editwin, sy, sx);
        t_print("cursor jumped: %li,%li -> %li,%li\n",
            sy, sx, 0L, min(sx, ln->len));
        cur_pos(0, min(sx, ln->len));
    }
    redraw();
    return 1;
}

int cmd_redraw(void)
{
    t_var2i(curr_buf->cursor.col, curr_buf->cursor.line);
    status_update(curr_buf);
    werase(editwin);
    redraw();
    fix_cursor();
    return 1;
}

int cmd_copy(void)
{
    t_called();
    if (!eb_rgn_marked(curr_buf)
        || pos_eq(&curr_buf->mark, &curr_buf->cursor))
        return 1; /* nothing to copy */

    /* set up the start and of the region: so that the start is
       guaranteed to occur before the end in the buffer. */
    struct pos sreg, ereg;
    pos_copy(&sreg, &curr_buf->mark);
    pos_copy(&ereg, &curr_buf->cursor);
    if (pos_lt(&ereg, &sreg)) {
        pos_swap(&ereg, &sreg);
    }

    /* todo: free any pre-existing cutting. (eventually should be push
       cutting down in cutting stack) */

    struct line *currln, *dupln;

    /* simple case: copy the text from a single line */
    if (sreg.line == ereg.line) {
        currln = eb_get_line_at(curr_buf, sreg.line);
        ASSERT(currln);
        dupln = ln_partial_copy(currln, sreg.col, ereg.col);
        g_cutting.text = dupln;
        g_cutting.linecount = 1;
        goto RTN;
    }

    /* complex case: multiple lines */
    struct line* cutln;
    int lcount;

    lcount = (ereg.line - sreg.line) + 1;
    currln = eb_get_line_at(curr_buf, sreg.line);
    currln = currln->next;

    /* copy the first line */
    g_cutting.text = ln_partial_copy(currln, sreg.col, currln->len);
    cutln = g_cutting.text;

    /* make full copies of all lines between the start and end line of
       the marked region.
     */
    if (lcount > 2) {
        int line;
        for (line = sreg.line + 1; line < ereg.line; line++) {
            cutln->next = ln_copy(currln);
            cutln = cutln->next;
            currln = currln->next;
        }
    }

    /* copy the last line */
    cutln->next = ln_partial_copy(currln, 0, ereg.col);
    g_cutting.linecount = lcount;

RTN:
    t_print("---- cutting begin\n");
    int i;
    currln = g_cutting.text;
    for (i = 0; currln != NULL; i++) {
        if (currln->len > 0)
            t_print("%d: '%*s'\n", i, (int)currln->len, currln->text);
        else
            t_print("%d: ''\n", i);
        currln = currln->next;
    }
    t_print("---- cutting end\n");

    eb_set_region_marked(curr_buf, FALSE);
    touchwin(editwin);
    wrefresh(editwin);
    redraw();
    return 1;
}

int cmd_paste(void)
{
    /* this corresponds to the yank command in emacs terminology. */
    if (g_cutting.text == NULL) {
        showmsg("Nothing to yank!");
    }

    /* note: repeatedly setting curr_key calling plain_insert() is so
       egregiously inefficient it is not to be considered.
     */

    if (g_cutting.linecount == 1) {
        ln_ins_str_at(curr_buf->ln, curr_buf->cursor.col,
            g_cutting.text->text, g_cutting.text->len);
        curr_buf->cursor.col += g_cutting.text->len;
        cur_move(0, g_cutting.text->len);
        goto RTN;
    }

    /* if the insertion is not at the beginning or the end of the
       current line. split the current line. */
    /* append each line in the cutting object to the current
       line. moving the current line down. */

RTN:
    redraw();
    return 1;
}

int cmd_tab(void)
{
    int dist2tab_stop;
    dist2tab_stop = SPACES_PER_TAB - (curr_buf->cursor.col % SPACES_PER_TAB);
    if (dist2tab_stop == 0)
        dist2tab_stop = SPACES_PER_TAB;
    int i;
    for (i = 0; i < dist2tab_stop; i++)
        insert(curr_buf, ' ');
    return 1;
}

int cmd_buf_beg(void)
{
    curr_buf->cursor.line = 0;
    curr_buf->cursor.col = 0;
    curr_buf->ln = curr_buf->top;
    wmove(editwin, 0, 0);
    redraw();
    return 1;
}

int cmd_buf_end(void)
{
    curr_buf->cursor.line = curr_buf->lines;
    curr_buf->cursor.col = curr_buf->bot->len;
    curr_buf->ln = curr_buf->bot;
    if (curr_buf->lines > EBWINSIZE)
        wmove(editwin, EBWINSIZE - 1, curr_buf->ln->len);
    else
        wmove(editwin, curr_buf->lines, curr_buf->ln->len);
    redraw();
    return 1;
}
