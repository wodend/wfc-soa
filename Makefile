include config.mk

SRC = wfc.c matrix2.c util.c
OBJ = ${SRC:.c=.o}

all: options wfc

options:
	@echo wfc build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

wfc: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

debug: CFLAGS += ${DEBUG}
debug: all

clean:
	@echo cleaning
	@rm -f wfc ${OBJ}

.PHONY: all options debug clean
