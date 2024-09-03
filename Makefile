.PHONY: all clean

EXE=e2 keyscan
LOG=dribble.txt
CFLAGS=-Wall -Wextra

ALLSRC=$(wildcard *.c)

E2OBJ=e2.o
E2DEP=e2.d

KEYSCANOBJ=keyscan.o
KEYSCANDEP=keyscan.d

LIBSRC=$(filter-out e2.c keyscan.c, ${ALLSRC})
LIBOBJ=$(patsubst %.c, %.o, $(LIBSRC))

CFLAGS += -g -Wall -Wextra -DENABLE_DRIBBLE
# For a static build.
#CFLAGS += -static

LDFLAGS += -lncurses

%.d : %.c
	$(CC) -MM $< > $@

e2 : $(E2OBJ) libe2.a
	$(CC) -o $@ $(E2OBJ) $(LDFLAGS) -L$(PWD) -le2

lib: libe2
	
libe2: libe2.a

libe2.a: CFLAGS+=-fPIC
libe2.a: $(LIBOBJ)
	ar rcs $@ $(LIBOBJ)
	ranlib $@

keyscan : $(KEYSCANOBJ)
	gcc -o $@ $+ -lncurses

all: $(EXE)

clean:
	@-rm -f *.o
	@-rm -f $(EXE) libe2.a
	@-rm -f $(LOG)
	@-rm -f core.*
	@-rm -f tmp.txt
	@-rm -f dribble.txt

veryclean: clean
	@-rm *.d

include Make.deps
