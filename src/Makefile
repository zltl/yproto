
build: flex bison
	gcc -g *.c -o yproto -lfl

flex: flex.l
	flex flex.l

bison: parser.y
	bison --debug -d parser.y

.PHONY: clean

clean:
	rm -rf parser.tab.c parser.tab.h lex.yy.c yproto
