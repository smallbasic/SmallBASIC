CLASSPATH:="c:/devtools/antlr/antlr-3.5-complete-no-st3.jar;."

#
# JAR files deployed with project
#
#./war/WEB-INF/lib/antlr-runtime-3.5.jar
#./war/WEB-INF/lib/appengine-api-1.0-sdk-1.7.4.jar
#./war/WEB-INF/lib/appengine-api-labs.jar
#./war/WEB-INF/lib/appengine-endpoints.jar
#./war/WEB-INF/lib/appengine-jsr107cache-1.7.4.jar
#./war/WEB-INF/lib/jsr107cache-1.1.jar
#

all: Translator.class

Translator.class: Translator.java SBLexer.java
	javac -cp ${CLASSPATH} Translator.java

SBLexer.java : SB.g
	java -cp ${CLASSPATH} org.antlr.Tool SB.g

test1: Translator.class
	java -cp ${CLASSPATH} Translator tc1.bas

test2: Translator.class
	java -cp ${CLASSPATH} Translator tc2.bas

test3: Translator.class
	java -cp ${CLASSPATH} Translator tc3.bas
