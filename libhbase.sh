#!/bin/sh

git submodule init

git submodule foreach --recursive git pull

cd third_party/libhbase

mvn install -DskipTests

mkdir -p ../../libhbase

tar zxvf target/libhbase-1.0-SNAPSHOT.tar.gz -C ../../libhbase

cp src/test/native/common/* ../../libhbase/include/hbase/

cd ../..