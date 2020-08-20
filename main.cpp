#include <ncurses.h>
#include "fileEditor.hpp"
#include "editor.hpp"
// #include <iostream>

/********************************
 * Todo:						*
 * Finish new line, delete etc 	*
 * Handle most user input		*
 ********************************/

int main(int argc, char** argv) {
	std::string path { "" };
	if(argc > 1) {
		path = std::string(argv[1]);
	}
	
	initscr();
	refresh();
	noecho();

	keypad(stdscr, true);

	// raw();
	// int input;
	// while((input = getch()) != 'g'){
	// 	printw("%d %c\n", input, input);	
	// }
	// endwin();
	// return 0;
	
	Editor editor { path };
	
	while(editor.isAlive()) {
		editor.draw();
		editor.getInput();
	}
	editor.saveFile();
		
	endwin();

	return 0;
}