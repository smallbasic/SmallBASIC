all: sbasic.hlp editor.hlp

sbasic.txt: ../../doc/ref.txt ref2tvh.bas
	-./ref2tvh.bas

sbasic.hlp: sbasic.txt
	-rm -f sbasic.hlp sbasic.h
	../parts/tvhc sbasic.txt sbasic.hlp sbasic.h

editor.hlp: editor.txt
	-rm -f editor.hlp sbeditor.h
	../parts/tvhc editor.txt editor.hlp sbeditor.h

clean:
	-rm -f {sbasic,editor}.hlp {sbasic,sbeditor}.h sbasic.txt
