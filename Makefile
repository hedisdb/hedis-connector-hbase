CC = gcc
CFLAGS = -O1 -std=c99 -fPIC
TARGET = hedis-connector-template

.SUFFIXS: .c .cpp

main: main.o
	${CC} -shared $< -o lib${TARGET}.so

%.o: %.c
	${CC} $< ${CFLAGS} -c

.PHONY: clean

clean:
	@rm -rf *.o *.so
