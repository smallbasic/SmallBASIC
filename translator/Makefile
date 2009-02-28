CLASSPATH:="/home/chris/antlr/antlr-3.1.1.jar:."

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
