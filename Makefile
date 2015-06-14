CC = gcc
CFLAGS = -Wall -O1 -std=c99 -fPIC
COMMON_FLAGS = -Wall -O1
COMMON_OBJS = $(addprefix libhbase/build/, $(notdir $(wildcard libhbase/build/*.o)))
INCLUDE = -Ilibhbase/include -Ilibhbase/include/hbase
LD_LIBRARY_PATH = -Llibhbase/lib/native -L/usr/lib/jvm/java-7-oracle/jre/lib/amd64/server -lhbase -ljvm -lstdc++ -lpthread

NAME = $(shell basename $(shell pwd))
TARGET = lib${NAME}.so

.PHONY: install uninstall clean

main: main.c libhbase/build/admin_ops.o libhbase/build/byte_buffer.o libhbase/build/common_utils.o libhbase/build/test_types.o
	${CC} -Wall -std=c99 -o main main.c ${COMMON_OBJS} ${INCLUDE} ${LD_LIBRARY_PATH}

libhbase/build/admin_ops.o: libhbase/src/common/admin_ops.cc
	${CC} ${COMMON_FLAGS} $< -c -o $@ ${INCLUDE}

libhbase/build/byte_buffer.o: libhbase/src/common/byte_buffer.cc
	${CC} ${COMMON_FLAGS} $< -c -o $@ ${INCLUDE}

libhbase/build/common_utils.o: libhbase/src/common/common_utils.cc
	${CC} ${COMMON_FLAGS} $< -c -o $@ ${INCLUDE}

libhbase/build/test_types.o: libhbase/src/common/test_types.cc
	${CC} ${COMMON_FLAGS} $< -c -o $@ ${INCLUDE}

install:
	cp ${TARGET} /usr/lib/

uninstall:
	rm /usr/lib/${TARGET}

clean:
	$(info Clean all artifacts)
	rm -rf *.o *.so
