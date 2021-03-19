#include <ncurses.h>
#include "fileEditor.hpp"
#include "editor.hpp"
#include <iostream>

// Yate: Yet Another Text Editor
// Originally created by Xylit (@Xyl1t)
// Many thanks to @Niki4Tap and @EntireTwix for contributing!

// TODO:
// * Fix vertical movement when tabs are present
// * Create a file when no file is specified
// * Word highlighting 
//   * Basic keyword highlighting 
//   * Begin and end parenthesis 
//   * Custom profile with format: 
//     extension: <file extension>
//     <word>: <color>
// * Check for permissions on windows

int main(int argc, char** argv) {
	std::string path {};
	if(argc > 1) {
		path = std::string(argv[1]);
	}
	int tabSize = 4;
	
	initscr();
	start_color();
	raw();
	refresh();
	noecho();
	keypad(stdscr, true);
	set_tabsize(tabSize); // TODO: make this an option: -t <tab_size>
	
	Editor editor { path, tabSize };
	
	while(editor.isAlive()) {
		editor.draw();
		editor.getInput();
	}
	editor.saveFile();
	
	endwin();
	
	return 0;
}
