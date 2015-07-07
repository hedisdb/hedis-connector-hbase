# hedis-connector-hbase

## Configuration

```yaml
cdh1:
  type: hbase
  zookeeper: ZOOKEEPER_HOST
  env:
    HBASE_LIB_DIR: /home/kewang/hbase/lib
    HBASE_CONF_DIR: /home/kewang/hbase/conf
    CLASSPATH: $CLASSPATH:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/async-1.4.0.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/asynchbase-1.5.0-libhbase-20140311.193218-1.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/commons-configuration-1.6.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/commons-lang-2.5.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/commons-logging-1.1.1.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/hadoop-core-1.0.4.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/hbase-0.94.17.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/libhbase-1.0-SNAPSHOT.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/log4j-1.2.17.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/netty-3.8.0.Final.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/protobuf-java-2.5.0.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/slf4j-api-1.7.5.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/slf4j-log4j12-1.7.5.jar:/home/kewang/git/libhbase/target/libhbase-1.0-SNAPSHOT/lib/zookeeper-3.4.5.jar
    LD_LIBRARY_PATH: $LD_LIBRARY_PATH:/usr/lib/jvm/java-7-oracle/jre/lib/amd64/server
```

## Requirement

N/A

## Build & Install

```sh
make
sudo make install
```

## Command Syntax

([a-zA-Z0-9_\-]+)@([#:a-zA-Z0-9_\\\-]+)(@([@#a-zA-Z0-9_\\\-]+)(:([a-zA-Z0-9_\-]+))?)?

### Example

"user@kewang" will return "kewang" rowkey at "user" table

### Return Value

```json
{
  "rowkey": "kewang"
  "columns": [
    {
      "name": "cf:nk",
      "value": "kewang"
    },
    {
      "name": "cf:id",
      "value": "U001"
    }
  ]
}
```
