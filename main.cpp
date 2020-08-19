#include <ncurses.h>
#include "fileEditor.hpp"
#include "editor.hpp"
// #include <iostream>

/*******************************
 * Todo:                       *
 * Finish Editor class!        *
 *******************************/

int main(int argc, char** argv) {
	std::string path { "" };
	if(argc > 1) {
		path = std::string(argv[1]);
	}
	
	initscr();
	refresh();
	noecho();
	keypad(stdscr, true);
	
	Editor editor { path };
	
	while(true) {
		editor.draw();
		editor.getInput();
	}
	
		
	getchar();
	endwin();

	return 0;
}