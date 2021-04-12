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
			
			std::cout << "Usage: yate [file] [options]\n";
			std::cout << "All possible options are:\n";
			std::cout << "  -t    tab size\n";
			std::cout << "  -r    amount of rows in terminal (only for windows)\n";
			std::cout << "  -c    amount of columns in terminal (only for windows)\n";
			std::cout << "  -h    shows this help screen\n";
			
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
