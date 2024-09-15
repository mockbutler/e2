#pragma once

#include "pos.h"

int cur_move(int y, int x);
int cur_pos(int y, int x);
int cur_posx(int x);

void cur_getpos(struct pos* pt);
void cur_setpos(const struct pos* pt);
