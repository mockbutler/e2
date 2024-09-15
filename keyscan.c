/* Copyright (c) 2006 Marc Butler.
 *
 * Still too much to do to make this a working editor!
 * No horizontal scrolling!
 * Flags are not used in the edit buffer structure.
 * Backspace at the start of a line is still broken.
 *
 */

#include <ctype.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void startup();
static void shutdown();

int main(int argc, char** argv)
{
    int key;

    startup();
    atexit(shutdown);
    printw("PRESS 'q' to exit!\n");
    for (;;) {
        key = getch();
        printw("KEY = %s %d (0x%x)\n", keyname(key), key, key);
        if (key == 'q')
            break;
    }
    return 0;
}

static void startup()
{
    if (initscr() == NULL) {
        puts("Fatal error unable to initialize curses!");
        exit(1);
    }
    cbreak();
    noecho();
    nonl();
    raw();
    intrflush(stdscr, FALSE);
    keypad(stdscr, FALSE);
    meta(stdscr, FALSE);
}

static void shutdown()
{
    endwin();
}
