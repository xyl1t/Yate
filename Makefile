# -*- Makefile -*-

all: yate

yate: main.o fileEditor.o editor.o
	clang++ -std=c++17 main.o editor.o fileEditor.o -o yate -lncurses

main.o: main.cpp
	clang++ -std=c++17 -c main.cpp

fileEditor.o: fileEditor.cpp fileEditor.hpp
	clang++ -std=c++17 -c fileEditor.cpp

editor.o: editor.cpp editor.hpp
	clang++ -std=c++17 -c editor.cpp

clean:
	rm -f *.o yate