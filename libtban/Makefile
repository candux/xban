# Commands
SHELL	 = /bin/sh
LINT     = lint
SRC      = tban.c mini_ng.c parser.c big_ng.c
OBJ      = $(SRC:.c=.o)
OUT      = libtban.a
INCLUDES = -I. -I../include/ -I/usr/local/include

# If we try to compile this at something else than Linux we may need to
# remoce some functions.
CFLAGS = -Wall -pedantic -ggdb -D LINUX -fPIC

ifeq ($(MACHTYPE),sparc)
  CFLAGS += -D DRYRUN
endif

.SUFFIXES: .c

default: $(OUT)

.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) $(OUT) Makefile.bak 

install:
	cp $(OUT) /usr/local/lib/

