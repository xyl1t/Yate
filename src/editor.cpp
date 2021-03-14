#include "editor.hpp"
#include <ncurses.h>
#include <unordered_map>

Editor::Editor(const std::string& filePath) 
	: file(filePath),
	scrollX(0),
	scrollY(0),
	width(getmaxx(stdscr)), 
	height(getmaxy(stdscr) - 2),
    alive(true) {
	resetStatus();
	if (!file.hasWritePermission()) {
		setStatus(" File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_ERROR);
	}
	initColorPairs();
}

void Editor::draw() {
	clear();
	syntaxHG.resetAllFlags();
	for (int lineNr = scrollY; lineNr < scrollY + height && lineNr < file.linesAmount(); lineNr++) {
		std::string_view line { file.getLine(lineNr) };
		move(lineNr - scrollY, 0);
		if (has_colors()) {
			syntaxHG.parseLine(line.data(), lineNr, file.getFileExtension());
		} else {
			printw("%3d %s", lineNr + 1, line.data());
		}
	}

	// Turn on and set color for status
	attron(A_STANDOUT);
	if (has_colors()) {
		attron(COLOR_PAIR(this->colorPair));
	}
	
	// Print status at bottom of screen
	mvprintw(getmaxy(stdscr) - 1, 0, this->statusText.c_str());

	if (has_colors()) {
		attroff(COLOR_PAIR(this->colorPair));
		// Reset color pair:
		this->colorPair = PAIR_STANDARD;
	}
	attroff(A_STANDOUT);

	std::string cursorText = file.getLine(file.getCarretY()).substr(0, file.getCarretX());
    mvprintw(file.getCarretY() - scrollY, 4, cursorText.c_str());
	refresh();
}

void Editor::getInput() {
	int input = getch();
	
	// Reset status, if input recieved
	resetStatus();

	if(input >= 32 && input < 127) {
		file.put(static_cast<char>(input));
		if(!file.hasWritePermission()) {
			setStatus(" Warning: File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_ERROR);
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
			case 9:
			case KEY_STAB:
				file.put(static_cast<char>(input));
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
#ifndef NDEBUG
		setStatus(this->statusText + "\tinput: " + std::to_string(input), PAIR_STANDARD);
		setStatus(this->statusText + "\tCOLORS: " + std::to_string(COLORS), PAIR_STANDARD);
#endif
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
	if(scrollX + width < file.getLineSize() - 1)
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
	if(file.getCarretX() >= file.getLineSize())
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
	if(file.getCarretX() == 0 && file.getCarretY() == 0) return;

	file.moveLeft();
	while(file.getLine()[file.getCarretX() - 1] != ' ' && file.getLine()[file.getCarretX() - 1] != '\t' && file.getCarretX() != 0) {
		file.moveLeft();
	}
}
void Editor::moveEndOfText() {
	if(file.getCarretX() == file.getLineSize() && file.getCarretY() == file.linesAmount()) return;

	file.moveRight();
	while(file.getLine()[file.getCarretX()] != ' ' && file.getLine()[file.getCarretX()] != '\t' && file.getCarretX() != file.getLineSize()) {
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
	try{
		file.del(false);
    	if(file.getCarretY() - scrollY < 0) {
        	scrollY--;
    	}
	} catch(std::string e) {
		setStatus(e, PAIR_ERROR);
	}
}
void Editor::deleteCharR() {
	try{
		file.del(true);
	} catch(std::string e) {
		setStatus(e, PAIR_ERROR);
	}
}

void Editor::saveFile() {
	if (!file.hasWritePermission()) {
		setStatus(" File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_ERROR);
	} else {
		setStatus(" File \'" + file.getFullFilename() + "\' has been saved. ", PAIR_INFO);
		file.save();
	}
}

void Editor::setStatus(const std::string& message) {
	setStatus(message, PAIR_STANDARD);
}
void Editor::setStatus(const std::string& message, int colorPair) {
	this->statusText = message;
	this->colorPair = colorPair;
}
void Editor::resetStatus() {
	char buffer[256];
	sprintf(buffer, " File: %s\tRow %2d, Col %2d ", file.getFullFilename().c_str(), file.getCarretY() + 1, file.getCarretX() + 1);
	setStatus(buffer);
}

// Defines color pairs
void Editor::initColorPairs() const {
	init_pair(PAIR_ERROR, COLOR_WHITE, COLOR_RED);
	init_pair(PAIR_STANDARD, COLOR_WHITE, COLOR_BLACK);
	init_pair(PAIR_INFO, COLOR_WHITE, COLOR_CYAN);
	init_pair(PAIR_OPEN_CLOSE_SYMBOL, COLOR_WHITE, COLOR_GREEN);
	// For syntax highlighting
	init_pair(PAIR_SYNTAX_BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(PAIR_SYNTAX_WHITE, COLOR_WHITE, COLOR_BLACK);
	init_pair(PAIR_SYNTAX_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(PAIR_SYNTAX_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(PAIR_SYNTAX_YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(PAIR_SYNTAX_GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(PAIR_SYNTAX_RED, COLOR_RED, COLOR_BLACK);

}
