#!/bin/sh

git submodule update

cd libhbase

mvn install -DskipTests

mkdir ../../libhbase

tar zxvf target/libhbase-1.0-SNAPSHOT.tar.gz -C ../../libhbase

cp src/test/native/common/* ../../libhbase/include/hbase/
