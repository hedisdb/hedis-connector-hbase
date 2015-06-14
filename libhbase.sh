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