#include "editor.hpp"
#include <ncurses.h>

Editor::Editor(const std::string& filePath) 
	: file(filePath), 
	scrollX(0),
	scrollY(0),
	width(getmaxx(stdscr)), 
	height(getmaxy(stdscr) - 2),
    alive(true) {
	
}

void Editor::draw() {
	clear();
	for (int lineNr = scrollY; lineNr < scrollY + height && lineNr < file.linesAmount(); lineNr++) {
		const std::string& line { file.getLine(lineNr) };
		move(lineNr - scrollY, 0);
		printw("%3d %s", lineNr + 1, line.c_str());
	}
	attron(A_STANDOUT);
	mvprintw(getmaxy(stdscr) - 1, 0, " File: %s\tRow %2d, Col %2d ", file.getFullFilename().c_str(), file.getCarretY() + 1, file.getCarretX() + 1);
	attroff(A_STANDOUT);
	std::string cursorText = file.getLine(file.getCarretY()).substr(0, file.getCarretX());
    mvprintw(file.getCarretY() - scrollY, 4, cursorText.c_str());
    
	refresh();
}

void Editor::getInput() {
	int input = getch();
	if(input >= 32 && input < 127) {
		file.put(static_cast<char>(input));
	}
	else if(input == KEY_UP) {
		moveUp();
	}
	else if(input == KEY_DOWN) {
		moveDown(); 
	}
	else if(input == KEY_LEFT) {
		moveLeft(); 
	}
	else if(input == KEY_RIGHT) {
		moveRight();
	}
	else if(input == KEY_END) {
		moveEndOfLine();
	}
	else if(input == KEY_HOME || input == 1) {
		moveBeginningOfLine();
	}
	else if(input == 2) {
		moveBeginningOfText();
	}
	else if(input == KEY_ENTER || input == 10) { // ENTER 
		newLine();
	}
	else if(input == KEY_BACKSPACE || input == 127) { // BACKSPACE
		deleteCharL();
	}
	// NOTE: let's do both KEY_DL and KEY_DC to be safe (famous last words)
	else if(input == 330 || input == KEY_DL || input == KEY_DC) { // DEL
		deleteCharR();
	}
	else if(input == 9 || input == KEY_STAB) { // TAB
		file.put(static_cast<char>(input));
    } else if(input == 19) {
		// TODO: Inform the user that file has been saved
		file.save();
	} else if(input == 3) {
		file.close();
		// NOTE: Maybe instead of exiting without saving, ask the user if he wants to save
		printf("No changes has been made to the file.\n");
		endwin();
		exit(0);
	}
}

void Editor::scrollUp() {
	if(scrollY > 0)
		scrollY--;
}
void Editor::scrollDown() {
	if(scrollY < file.linesAmount() - 1)
		scrollY++;
}
void Editor::scrollLeft() {
	if(scrollY > 0)
		scrollX--;
}
void Editor::scrollRight() {
	if(scrollX + width < file.getLine().size() - 1)
		scrollX++;
}

void Editor::moveUp() {
	if(scrollY - file.getCarretY() == 0)
		scrollUp();
	file.moveUp();
}
void Editor::moveDown() {
	if((scrollY + height) - file.getCarretY() - 1 == 0)
		scrollDown();
	file.moveDown();
}
void Editor::moveLeft() {
	if(file.getCarretX() >= file.getLine().size())
		scrollLeft();
	file.moveLeft();
}
void Editor::moveRight() {
	if(file.getCarretX() > 0) 
		scrollRight();
	file.moveRight();
}

void Editor::moveBeginningOfLine() {
	file.setCarretLocation(0, file.getCarretY());
}
void Editor::moveBeginningOfText() {
	file.setCarretLocation(0, file.getCarretY());
	while(file.getLine()[file.getCarretX()] == ' ' || file.getLine()[file.getCarretX()] == '\t' ) {
		file.moveRight();
	}
}
void Editor::moveEndOfLine() {
	file.setCarretLocation(file.getLineSize(), file.getCarretY());
}
void Editor::newLine() {
	file.newLine();
    moveDown();
	file.setCarretLocation(0, file.getCarretY());

}
void Editor::deleteCharL() {
	file.del(false);
    if(file.getCarretY() - scrollY < 0) {
        scrollY--;
    }
}
void Editor::deleteCharR() {
	file.del(true);
}