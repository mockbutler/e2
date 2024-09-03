#ifndef CURSOR_H
#define CURSOR_H

#include "pt.h"

int cur_move(int y, int x);
int cur_pos(int y, int x);
int cur_posx(int x);

void cur_getpos(struct point *pt);
void cur_setpos(const struct point *pt);

#endif
