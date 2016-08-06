#/bin/sh

export LD_LIBRARY_PATH="`pwd`/.libs"
export CLASSPATH="\
libs/jboss-logging-3.2.1.Final.jar:\
libs/xnio-api-3.3.6.Final.jar:\
libs/xnio-nio-3.3.6.Final.jar:\
libs/undertow-core-1.3.23.Final.jar:\
target/SBServer-1.0.jar"

java -server net.sourceforge.smallbasic.WebServer

