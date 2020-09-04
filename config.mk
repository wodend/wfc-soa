VERSION = 0.1.0

INCS =
LIBS = -lm

CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS   = -std=c99 -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = -s ${LIBS}
DEBUG    = -ggdb -DDEBUG

CC = cc
