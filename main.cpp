#include <ncurses.h>
#include "fileEditor.hpp"
#include "editor.hpp"

/********************************
 * Todo:						*
 * Fix Tab movement 			*
 ********************************/

int main(int argc, char** argv) {
	std::string path { "" };
	if(argc > 1) {
		path = std::string(argv[1]);
	}
	
	initscr();
	start_color();
	raw();
	refresh();
	noecho();
	keypad(stdscr, true);
	
	Editor editor { path };
	
	while(editor.isAlive()) {
		editor.draw();
		editor.getInput();
	}
	editor.saveFile();
		
	endwin();

	return 0;
}