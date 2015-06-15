CC = gcc
COMMON_FLAGS = -Wall -O1 -fPIC
COMMON_OBJS = $(addprefix libhbase/build/, $(notdir $(wildcard libhbase/build/*.o)))
INCLUDE = -Ilibhbase/include -Ilibhbase/include/hbase
LD_LIBRARY_PATH = -Llibhbase/lib/native -L/usr/lib/jvm/java-7-oracle/jre/lib/amd64/server -lhbase -ljvm -lstdc++ -lpthread

NAME = $(shell basename $(shell pwd))
TARGET = lib${NAME}.so

.PHONY: install uninstall clean

all: main.o libhbase/build/admin_ops.o libhbase/build/byte_buffer.o libhbase/build/common_utils.o libhbase/build/test_types.o
	${CC} -shared $^ ${LD_LIBRARY_PATH} -o ${TARGET}

main.o: main.c
	${CC} -Wall -O1 -std=c99 -fPIC -c $< ${INCLUDE} ${LD_LIBRARY_PATH}

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
	cp libhbase/lib/native/libhbase.so /usr/lib
	ln -s /usr/lib/jvm/java-7-oracle/jre/lib/amd64/server/libjvm.so /usr/lib/libjvm.so

uninstall:
	rm /usr/lib/${TARGET}
	rm /usr/lib/libhbase.so
	rm /usr/lib/libjvm.so

clean:
	$(info Clean all artifacts)
	rm -rf *.o *.so
	rm -rf libhbase/build/*
