#include "editor.hpp"
#include <ncurses.h>
#include <unordered_map>

// Color pairs defines:
#define PAIR_STANDARD 1
#define PAIR_ERROR 2
#define PAIR_WARNING 3
#define PAIR_INFO 4

Editor::Editor(const std::string& filePath, int tabSize) 
	: file(filePath), 
	TAB_SIZE(tabSize),
	scrollX(0),
	scrollY(0),
	width(getmaxx(stdscr) - 4), 
	height(getmaxy(stdscr) - 2),
    alive(true),
	caret() {
	resetStatus();
	if (!file.hasWritePermission()) {
		setStatus(" File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_WARNING);
	}
	initColorPairs();
}

void Editor::draw() {
	clear();
	for (int lineNr = scrollY; lineNr < scrollY + height && lineNr < file.linesAmount(); lineNr++) {
		std::string line { file.getLine(lineNr) };
		move(lineNr - scrollY, 0);
		int min = getTextEditorWidth();
		if((int)line.length() - scrollX < min) 
			min = line.length() - scrollX;
		if((int)line.length() < scrollX) 
			line = "";
		else
			line = line.substr(scrollX, min);
		printw("%3d %s", lineNr + 1, line.c_str());
	}

	// turn on and set color for status
	attron(A_STANDOUT);
	attron(COLOR_PAIR(this->colorPair));
	
	// print status at bottom of screen
	mvprintw(getmaxy(stdscr) - 1, 0, this->statusText.c_str());

	attroff(COLOR_PAIR(this->colorPair));
	// Reset color pair:
	this->colorPair = PAIR_STANDARD;
	attroff(A_STANDOUT);

	// std::string cursorText = file.getLine(file.getCaretY()).substr(0, file.getCaretX());
	// mvprintw(file.getCaretY() - scrollY, 4, cursorText.c_str());
	
	move(caret.y, caret.x + 4);
	refresh();
}

void Editor::getInput() {
	int input = getch();

	if((input >= 32 && input < 127) || input == KEY_STAB || input == 9) {
		put(static_cast<char>(input));
		if(!file.hasWritePermission()) {
			setStatus(" Warning: File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_WARNING);
		}
	}
	else
	{
		switch(input)
		{
			case KEY_UP:
				moveUp();
				break;
			case KEY_DOWN:
				moveDown();
				break;
			case KEY_LEFT:
				moveLeft();
				break;
			case KEY_RIGHT:
				moveRight();
				break;
			case 5:
			case KEY_END:
				moveEndOfLine();
				break;
			case KEY_HOME:
			case 1:
				moveBeginningOfLine();
				break;
			case 25: // CTRL+Y (for qwertz layout)
			case 26: // CTRL+Z (for qwerty layout)
				moveBeginningOfText();
				break;
			case 24: // CTRL+X
				moveEndOfText();
				break;
			case KEY_ENTER:
			case 10:
				newLine();
				break;
			case KEY_BACKSPACE:
			case 127:
				deleteCharL();
				break;
			// NOTE: let's do both KEY_DL and KEY_DC to be safe (famous last words)
			case 330:
			case KEY_DL:
				deleteCharR();
				break;
			case 19:
				saveFile();
				break;
			case 3:
				file.close();
				// NOTE: Maybe instead of exiting without saving, ask the user if he wants to save
				endwin();
				exit(0);
				break;
		}
	}

	// Reset status on user input if no custom status was applied, if there is a custom status, let it display first and then reset
	if(!customStatusText) {
		resetStatus();
	}
	customStatusText = false;
	
#ifndef NDEBUG
		setStatus(this->statusText + "\tinput: " + std::to_string(input), this->colorPair);
		customStatusText = false;
#endif
}

void Editor::put(char ch) {
	file.put(ch);
	moveRight();
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
	// if(scrollY > 0)
		scrollX--;
}
void Editor::scrollRight() {
	// if(scrollX + getTextEditorWidth() < file.getLineSize() - 1)
		scrollX++;
}

void Editor::moveUp() {
	if(caret.y - 1 >= 0) {
		caret.y--;
		file.moveUp();
		if (getVirtualLineLength() < caret.savedX) {
			caret.x = getVirtualLineLength();
			file.setCaretLocation(file.getLineSize(), file.getCaretY());
		} else {
			file.setCaretLocation(getFileCaretColumn(caret.savedX), file.getCaretY());
			caret.x = getVirtualCaretColumn(file.getCaretX(), file.getCaretY());
		} 
	} else {
		caret.y = 0;
		caret.x = caret.savedX = 0;
		file.setCaretLocation(0, 0);
	}
	
	if(scrollY - file.getCaretY() == 0)
		scrollUp();
}
void Editor::moveDown() {
	if(caret.y + 1 < file.linesAmount()) {
		caret.y++;
		file.moveDown();
		if (getVirtualLineLength() < caret.savedX) {
			caret.x = getVirtualLineLength();
			file.setCaretLocation(file.getLineSize(), file.getCaretY());
		} else {
			file.setCaretLocation(getFileCaretColumn(caret.savedX), file.getCaretY());
			caret.x = getVirtualCaretColumn(file.getCaretX(), file.getCaretY());
		} 
	} else {
		caret.y = file.linesAmount() - 1;
		caret.x = caret.savedX = getVirtualCaretColumn(file.getLineSize(), file.linesAmount() - 1);
		file.setCaretLocation(file.getLineSize(), file.linesAmount() - 1);
	}
	
	if((scrollY + height) - file.getCaretY() - 1 == 0)
		scrollDown();	
}
void Editor::moveLeft() {
	if(getOnScreenCursorX() == 0)
		scrollLeft();
	
	if(caret.x > 0) {
		file.moveLeft();
		caret.x = caret.savedX = getVirtualCaretColumn(getFileCaretColumn(), caret.y);
	} else if(caret.y > 0) {
		moveUp();
		caret.x = caret.savedX = getVirtualCaretColumn(file.getLineSize(caret.y), caret.y);
		file.setCaretLocation(file.getLineSize(caret.y), file.getCaretY());
	}
}
void Editor::moveRight() {
	if(getOnScreenCursorX() >= getTextEditorWidth() - 1) 
		scrollRight();
	
	if (caret.x < getVirtualCaretColumn(file.getLineSize(caret.y), caret.y)) {
		file.moveRight();
		caret.x = caret.savedX = getVirtualCaretColumn(getFileCaretColumn(), caret.y);
	} else if (caret.y < file.linesAmount() - 1) {
		moveDown();
		caret.x = caret.savedX = 0;
		file.setCaretLocation(0, file.getCaretY());
	}
}

void Editor::moveBeginningOfLine() {
	file.setCaretLocation(0, file.getCaretY());
}
void Editor::moveBeginningOfText() {
	if(file.getCaretX() == 0 && file.getCaretY() == 0) return;

	file.moveLeft();
	while(file.getLine()[file.getCaretX() - 1] != ' ' && file.getLine()[file.getCaretX() - 1] != '\t' && file.getCaretX() != 0) {
		file.moveLeft();
	}
}
void Editor::moveEndOfText() {
	if(file.getCaretX() == file.getLineSize() && file.getCaretY() == file.linesAmount()) return;

	file.moveRight();
	while(file.getLine()[file.getCaretX()] != ' ' && file.getLine()[file.getCaretX()] != '\t' && file.getCaretX() != file.getLineSize()) {
		file.moveRight();
	}
}
void Editor::moveEndOfLine() {
	file.setCaretLocation(file.getLineSize(), file.getCaretY());
}
void Editor::newLine() {
	file.newLine();
    moveDown();
	setCaretLocation(0, caret.y);
}
void Editor::deleteCharL() {
	try {
		if(caret.x == 0) {
			moveLeft();
			file.del(true);
		} else {
			file.del(false);
			moveLeft();
		}
    	if(file.getCaretY() - scrollY < 0) {
        	scrollY--;
    	}
	} catch(std::string e) {
		setStatus(e, PAIR_ERROR);
	}
}
void Editor::deleteCharR() {
	try {
		file.del(true);
	} catch(std::string e) {
		setStatus(e, PAIR_ERROR);
	}
}

void Editor::saveFile() {
	if (file.hasWritePermission()) {
		setStatus(" File \'" + file.getFullFilename() + "\' has been saved. ", PAIR_INFO);
		file.save();
	} else {
		setStatus(" File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_ERROR);
	}
}

void Editor::setStatus(const std::string& message) {
	setStatus(message, PAIR_STANDARD);
}
void Editor::setStatus(const std::string& message, int colorPair) {
	this->statusText = message;
	this->colorPair = colorPair;
	customStatusText = true;
}
void Editor::resetStatus() {
	char buffer[256];
	sprintf(
		buffer, 
		" File: %s\tRow %2d, Col %2d | c.x %2d, c.y %2d, c.sx %2d | f.x %2d, f.y %2d", 
		file.getFullFilename().c_str(), 
		file.getCaretY() + 1, file.getCaretX() + 1, 
		caret.x, caret.y, caret.savedX, 
		file.getCaretX(), file.getCaretY()
	);
	setStatus(buffer);
}

// Defines color pairs
void Editor::initColorPairs() const {
	init_pair(PAIR_ERROR, COLOR_WHITE, COLOR_RED);
	init_pair(PAIR_STANDARD, COLOR_WHITE, COLOR_BLACK);
	init_pair(PAIR_WARNING, COLOR_WHITE, COLOR_RED);
	init_pair(PAIR_INFO, COLOR_WHITE, COLOR_BLUE);
}
