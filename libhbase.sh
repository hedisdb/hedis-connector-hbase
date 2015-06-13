#!/bin/sh

git submodule update --init --recursive

cd third_party/libhbase

git checkout master

mvn install -DskipTests

mkdir -p ../../libhbase

tar zxvf target/libhbase-1.0-SNAPSHOT.tar.gz -C ../../libhbase

cp src/test/native/common/* ../../libhbase/include/hbase/

cd ../..