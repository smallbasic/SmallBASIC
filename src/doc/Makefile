#
#	SmallBASIC USERS GUIDE
#		Source: guide.texi
#

h2t=lynx -dump
h2t_pretty=$(h2t) -width=79 -with_backspaces
h2t_plain=$(h2t) -width=79
h2t_palm=$(h2t) -width=39

all: guide.html guide.pdf sbasic.info guide.dvi guide.txt guide-palm.txt sbasic.1 guide-plain.txt \
	txt2c guide.c

#gema: use /ext/gema/

cutof: cutof.cxx
	gcc cutof.cxx -o cutof -lstdc++ -lpcre

txt2c: txt2c.cxx
	gcc txt2c.cxx -o txt2c -lstdc++

guide.c: guide.txt txt2c
	./txt2c guide.txt guide.c

guide.html: guide.texi
	./texi2html guide.texi -init_file=./t2h.init

sbasic.info: guide.texi
	makeinfo --no-split guide.texi
	chmod 0644 sbasic.info
	#install-info sbasic.info --info-dir=/usr/share/info/
	cp ./sbasic.info /usr/share/info

guide.dvi: guide.texi
	texi2dvi guide.texi	

guide.pdf: guide.texi
	texi2pdf guide.texi	

guide.txt: guide.html cutof cutof-guide.conf
#	html2text -style pretty -o guide-alpha.txt guide.html
	$(h2t_pretty) guide.html > guide-alpha.txt
	cat guide-alpha.txt | ./cutof -f cutof-guide.conf > guide.txt

guide-palm.txt: guide.html cutof cutof-guide-palm.conf
#	html2text -style pretty -nobs -width 39 -o guide-palm-alpha.txt guide.html 
	$(h2t_palm) guide.html > guide-palm-alpha.txt
	cat guide-palm-alpha.txt | ./cutof -f cutof-guide-palm.conf > guide-palm.txt

guide-plain.txt: guide.html cutof cutof-guide.conf
#	html2text -style pretty -nobs -width 79 -o guide-plain-alpha.txt guide.html 
	$(h2t_plain) guide.html > guide-plain-alpha.txt
	cat guide-plain-alpha.txt | ./cutof -f cutof-guide.conf > guide-plain.txt

sbasic.1: guide.texi cutof texi2man.gema
	cat guide.texi | gema -f texi2man.gema | gema -f texi2man.gema | cutof > sbasic.1
	cp sbasic.1 /usr/share/man/man1/

m-Console.txt: guide-plain.txt cutof cutof-guide-wt.conf
	cat guide-plain.txt | cutof -f cutof-guide-wt.conf > sbasic.1

# another way to procude clean text
#makeinfo --no-headers guide.texi

clean:
#	rm -f guide.html guide.pdf guide.dvi guide.txt guide-palm.txt guide-plain.txt 
	mv guide.texi temp.texi
	rm -f guide*.*
	mv temp.texi guide.texi
	rm -f sbasic.info sbasic.1 m[0-9G]-*.txt
	rm -f txt2c cutof


