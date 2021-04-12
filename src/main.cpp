// TODO: Redefine key macros 
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
// * Check for permissions on windows

int main(int argc, char** argv) {

#if defined(YATE_WINDOWS) && NDEBUG
	FreeConsole();
#endif

	int terminalWidth = 0;
	int terminalHeight = 0;
	int tabSize = 4;
	std::string path {};
	for(int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		std::stringstream argStream {argv[i]};
		if(arg.rfind("-t", 0) == 0) {
			char junk{};
			argStream >> junk >> junk >> junk;
			std::stringstream argVal {argv[i + 1]};
			argVal >> tabSize;
			i++;
		} else if (arg.rfind("-r", 0) == 0) {
			char junk{};
			argStream >> junk >> junk >> junk;
			std::stringstream argVal {argv[i + 1]};
			argVal >> terminalWidth;
			i++;
		} else if (arg.rfind("-c", 0) == 0) {
			char junk{};
			argStream >> junk >> junk >> junk;
			std::stringstream argVal {argv[i + 1]};
			argVal >> terminalHeight;
			i++;
		} else if (arg.rfind("-h", 0) == 0) {
			std::cout << R"STR(Usage: yate [file] [options]
All possible options are:
  -t    tab size
  -r    amount of rows in terminal (only for windows)
  -c    amount of columns in terminal (only for windows)
  -h    shows this help screen

Yate key bindings:
Main: 
  ctrl+s: save file
  ctrl+c: exit the program
Movement:
  arrow keys: move one letter up/down/left/right
  page-up:    move caret up by screen size
  page-down:  move caret down by screen size
  home:       put caret at end the beginning of the line
  end:        put caret at the end of the line
  ctrl+x:     put caret to beginning of word
  ctrl+z:     put caret to end of word
  ctrl+k:     scroll screen left
  ctrl+l:     scroll screen right
Misc: 
  ctrl+f: find word
    up-arrow: go to previous match
    down-arrow: go to next match
    enter: confirm caret location
    esc, ctrl+c: cancel
  ctrl+u: undo
  ctrl+d: redo
)STR";	
			return 0;
		} else {
			argStream >> path;
		}
	}
	initscr();
	start_color();
	raw();
	refresh();
	noecho();
	resize_term(terminalHeight, terminalWidth);
	keypad(stdscr, true);
#if defined(YATE_WINDOWS)
	SetWindowTextA(GetActiveWindow(),"Yate"); // set window title
#else 
	set_escdelay(0);
#endif
	
	
	Editor editor { path, tabSize };
	
	while(editor.isAlive()) {
		editor.draw();
		editor.getInput();
	}
	
	endwin();
	
	return 0;
}
