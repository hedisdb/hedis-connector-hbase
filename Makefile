CC = gcc
CFLAGS = -O1 -std=c99 -fPIC
NAME = $(shell basename $(shell pwd))
TARGET = lib${NAME}.so

.SUFFIXS: .c .cpp

main: main.o
	${CC} -shared $< -o ${TARGET}

%.o: %.c
	${CC} $< ${CFLAGS} -c

install:
	cp ${TARGET} /usr/lib/

uninstall:
	rm /usr/lib/${TARGET}

clean:
	@rm -rf *.o *.so

pre_install:
	@echo "No other pre_install jobs"
