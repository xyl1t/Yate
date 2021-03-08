# -*- Makefile -*-

CXX := g++

all: yate

yate: main.o fileEditor.o editor.o
	$(CXX) -std=c++17 main.o editor.o fileEditor.o -o yate -lncurses

main.o: main.cpp
	$(CXX) -std=c++17 -c main.cpp

fileEditor.o: fileEditor.cpp fileEditor.hpp
	$(CXX) -std=c++17 -c fileEditor.cpp

editor.o: editor.cpp editor.hpp
	$(CXX) -std=c++17 -c editor.cpp

clean:
	rm -f *.o yate