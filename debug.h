/* Copyright (c) 2006 Marc Butler.
 *
 */
#ifndef DEBUG_HDR
#define DEBUG_HDR

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#ifdef ENABLE_DRIBBLE

#define t_called() do { \
		fprintf(dribble, "CALLED: %s\n", __FUNCTION__); \
		fflush(dribble); \
	} while (0)

#define t_print(...) do { \
		fprintf(dribble, __VA_ARGS__); \
		fflush(dribble); \
	} while (0)

#define t_var1i(var) do { \
		fprintf(dribble, "%s:%d: VAR %s = %d\n", __FUNCTION__,	\
			__LINE__, #var, var); \
	} while (0)

#define t_var2i(v1, v2) do {						\
		fprintf(dribble, "%s:%d: VAR %s = %li, VAR %s = %li\n", \
			__FUNCTION__, __LINE__, #v1, (long)v1, #v2, (long)v2); \
	} while (0)

#else
#define TRACE(...)
#endif

#define ASSERT(cond) \
	do { \
		if (!(cond)) { \
			endwin();  \
			printf("%s:%d: %s\n", __FILE__, __LINE__ , #cond); \
			fprintf(dribble, "CRASHED: %s:%d %s\n", \
				__FILE__, __LINE__, #cond);	\
			abort(); \
		} \
	} while(0)

extern FILE *dribble;

#endif
