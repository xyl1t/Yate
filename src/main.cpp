#if defined(YATE_WINDOWS)
#include "pdcurses.h"
#include "Windows.h"
#else
#include <ncurses.h>
#endif

#include "fileEditor.hpp"
#include "editor.hpp"
#include <iostream>

// Yate: Yet Another Text Editor
// Originally created by Xylit (@Xyl1t)
// Contributors: @Niki4Tap for revoking the project and working on it
//  @EntireTwix for making small, but important changes to CMakeLists.txt
//  @DCubix (Diego) for getting yate run for the first time on Windows.

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

#if defined(YATE_WINDOWS) && NDEBUG
	FreeConsole();
#endif

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
