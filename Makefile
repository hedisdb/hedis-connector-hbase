CC = gcc
CFLAGS = -O1 -std=c99 -fPIC
INCLUDE = -I libhbase/include -I libhbase/include/hbase
NAME = $(shell basename $(shell pwd))
TARGET = lib${NAME}.so

.SUFFIXS: .c .cpp .cc

main: main.o
	${CC} -shared $< -o ${TARGET}

%.o: %.c
	${CC} ${INCLUDE} $< ${CFLAGS} -c

%.o: %.cc
	${CC} ${INCLUDE} $< ${CFLAGS} -c

.PHONY: install

install:
	cp ${TARGET} /usr/lib/

.PHONY: uninstall

uninstall:
	rm /usr/lib/${TARGET}

.PHONY: clean

clean:
	@rm -rf *.o *.so
