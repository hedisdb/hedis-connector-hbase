#!/bin/sh

git submodule update --init --recursive

cd third_party/libhbase

git checkout master

mvn install -DskipTests

mkdir -p ../../libhbase/src/common

tar zxvf target/libhbase-1.0-SNAPSHOT.tar.gz -C ../../libhbase

cp src/test/native/common/*.cc ../../libhbase/src/common
cp src/test/native/common/*.h ../../libhbase/include/hbase

cd ../../libhbase

mkdir -p build

gcc -Wall src/common/admin_ops.cc -c -o build/admin_ops.o -Iinclude/hbase -Iinclude
gcc -Wall src/common/byte_buffer.cc -c -o build/byte_buffer.o -Iinclude/hbase -Iinclude
gcc -Wall src/common/common_utils.cc -c -o build/common_utils.o -Iinclude/hbase -Iinclude
gcc -Wall src/common/test_types.cc -c -o build/test_types.o -Iinclude/hbase -Iinclude

gcc -Wall -o example_async src/examples/async/example_async.c build/admin_ops.o build/byte_buffer.o build/common_utils.o build/test_types.o -Iinclude -Iinclude/hbase -Llib/native -L/usr/lib/jvm/java-7-oracle/jre/lib/amd64/server -lhbase -ljvm -lstdc++ -lpthread -std=c99