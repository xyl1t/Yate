#include <ncurses.h>
#include "fileEditor.hpp"
#include "editor.hpp"
#include <iostream>

// Yate: Yet Another Text Editor
// Originally created by Xylit (@Xyl1t)
// Many thanks to @Niki4Tap and @EntireTwix for contributing!

// TODO:
// * Word highlighting 
//   * Basic keyword highlighting 
//   * Begin and end parenthesis 
//   * Custom profile with format: 
//     extension: <file extension>
//     <word>: <color>
// * Options
//   --path-to-profile    specify the path to .yateprofile
// * Check for filenames such as %s, %c etc
// * Simple searching
// * Check for permissions on windows

int main(int argc, char** argv) {

	int tabSize = 4;
	std::string path {};
	for(int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		std::stringstream argStream {argv[i]};
		if(arg.rfind("-t", 0) == 0) {
			char junk{};
			argStream >> junk >> junk >> junk;
			argStream >> tabSize;
			std::stringstream argVal {argv[i + 1]};
			argVal >> tabSize;
			i++;
		} else {
			argStream >> path;
		}
	}
	
	initscr();
	start_color();
	raw();
	refresh();
	noecho();
	keypad(stdscr, true);
	
	Editor editor { path, tabSize };
	
	while(editor.isAlive()) {
		editor.draw();
		editor.getInput();
	}
	
	endwin();
	exit(0);
	
	return 0;
}
