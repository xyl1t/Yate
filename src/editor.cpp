#include "editor.hpp"
#include "syntaxHighlighter.hpp"
#include <ncurses.h>
#include <unordered_map>
#include <algorithm>

Editor::Editor(const std::string& filePath, int tabSize) 
	: file(filePath), 
	caret(),
	TAB_SIZE(tabSize),
	scrollX(0),
	scrollY(0),
	width(getmaxx(stdscr) - 4), 
	height(getmaxy(stdscr) - 2),
    alive(true),
	customStatusText(false) {
	initColorPairs();
	resetStatus();
	customStatusText = false;
	if (!file.hasWritePermission()) {
		setStatus(" File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_ERROR);
	}
	syntaxHighlighter syntaxHG{};
}

bool Editor::close() {
	// NOTE: Maybe instead of exiting without saving, ask the user if he wants to save
	if(file.hasFileContentChanged()) {
		std::string status {" Exit without saving? [Y/N] "};
		setStatus(status, PAIR_WARNING);
		draw();
		int input{getch()};
		if(input == 'y' || input == 'Y') {
			this->alive = false;
			file.close();
			return true;
		} 
		resetStatus();
		draw();
		return false;
	}
	this->alive = false;
	file.close();
	return true;
}

void Editor::draw() {
	clear();
	
	syntaxHG.resetAllFlags();
	for (int lineNr = scrollY; lineNr < scrollY + height && lineNr < file.linesAmount(); lineNr++) {
		std::string line { file.getLine(lineNr) };
		int virtualCol = 0;
		int min = getTextEditorWidth();
		
		syntaxHG.resetLineSpecificFlags();
		std::vector<std::string> splittedLine = syntaxHG.splitBySpecialChars(line, this->file.getFileExtension());

		move(lineNr - scrollY, 0);

		printw("%3d ", lineNr + 1);
		for (std::string symbol : splittedLine) {
			if (symbol == "\t") {
				const int tabSize = TAB_SIZE - (virtualCol) % TAB_SIZE;
				for(int original = virtualCol; virtualCol < original + tabSize; virtualCol++) {
					if(virtualCol >= scrollX && virtualCol - scrollX < min) {
						printw(" ");
					}
				}
			} else {
				if(virtualCol >= scrollX && virtualCol - scrollX < min) {
					syntaxHG.parseSymbol(symbol, this->file.getFileExtension(), *this);
				}
				virtualCol += symbol.size();
			}
		}
	}

	// Turn on and set color for status
	attron(A_STANDOUT);
	if (has_colors()) {
		attron(COLOR_PAIR(this->colorPair));
	}
	
	// Print status at bottom of screen
	this->statusText.resize(getmaxx(stdscr), ' ');
	mvprintw(getmaxy(stdscr) - 1, 0, this->statusText.c_str());

	if (has_colors()) {
		attroff(COLOR_PAIR(this->colorPair));
		// Reset color pair:
		this->colorPair = PAIR_STANDARD;
	}
	attroff(A_STANDOUT);
	
	move(caret.y - scrollY, caret.x - scrollX + 4);
	refresh();
}

int Editor::getInput() {
	int input = getch();

	if((input >= 32 && input < 127) || input == KEY_STAB || input == 9) {
		put(static_cast<char>(input));
		if(!file.hasWritePermission()) {
			setStatus(" Warning: File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_ERROR);
		}
	}
	else
	{
		switch(input)
		{
			case KEY_PPAGE:
			setCaretLocation(caret.x, caret.y - (getTextEditorHeight() - 1));
				break;
			case KEY_NPAGE:
			setCaretLocation(caret.x, caret.y + (getTextEditorHeight() - 1));
				break;
			case 11:
			scrollLeft();
				break;
			case 12:
			scrollRight();
				break;
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
				moveBeginningOfText(); // TODO: FIX
				break;
			case 24: // CTRL+X
				moveEndOfText(); // TODO: FIX
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
				close();
				break;
		}
	}
	if(!(input == 11 || input == 12))
		scrollToCaret();

	// Reset status on user input if no custom status was applied, if there is a custom status, let it display first and then reset
	if(!customStatusText) {
		resetStatus();
	}
	customStatusText = false;
	
#ifndef NDEBUG
		setStatus(this->statusText + "\tinput: " + std::to_string(input), PAIR_STANDARD);
		setStatus(this->statusText + "\tCOLORS: " + std::to_string(COLORS), PAIR_STANDARD);
#endif

	return input;
}

void Editor::put(char ch) {
	file.put(ch);
	moveRight();
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
	scrollToCaret();
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
		file.setCaretLocation(file.getLineSize(), file.linesAmount() - 1);
		caret.y = file.linesAmount() - 1;
		caret.x = caret.savedX = getVirtualCaretColumn(file.getLineSize(), file.linesAmount() - 1);
	}
	scrollToCaret();
}
void Editor::moveRight() {
	int prev = caret.x;
	if (caret.x < getVirtualCaretColumn(file.getLineSize(caret.y), caret.y)) {
		file.moveRight();
		caret.x = caret.savedX = getVirtualCaretColumn(getFileCaretColumn(), caret.y);
	} else if (caret.y < file.linesAmount() - 1) {
		moveDown();
		setScrollH(0);
		caret.x = caret.savedX = 0;
		file.setCaretLocation(0, file.getCaretY());
	}
	
	if(caret.x - scrollX + 1 > getTextEditorWidth()) 
		scrollRight(caret.x - prev); // NOTE: calculating difference in case if there is a tab
}
void Editor::moveLeft() {
	int prev = caret.x;
	if(caret.x > 0) {
		file.moveLeft();
		caret.x = caret.savedX = getVirtualCaretColumn(getFileCaretColumn(), caret.y);
	} else if(caret.y > 0) {
		moveUp();
		int virtualX = getVirtualCaretColumn(file.getLineSize(caret.y), caret.y);
		setScrollH(virtualX - getTextEditorWidth() + 2);
		caret.x = caret.savedX = virtualX;
		file.setCaretLocation(file.getLineSize(caret.y), file.getCaretY());
	}
	
	if(caret.x - scrollX < 0)
		scrollLeft(prev - caret.x); // NOTE: calculating difference in case if there is a tab
}

void Editor::moveBeginningOfLine() {
	setCaretLocation(0, caret.y);
}
void Editor::moveEndOfLine() {
	setCaretLocation(getVirtualLineLength(), caret.y);
}
void Editor::moveBeginningOfText() {
	if(caret.x == 0 && caret.y == 0) return;

	moveLeft();
	while(file.getLine()[file.getCaretX() - 1] != ' ' && file.getLine()[file.getCaretX() - 1] != '\t' && file.getCaretX() != 0) {
		moveLeft();
	}
}
void Editor::moveEndOfText() {
	if(caret.x == getVirtualLineLength() && caret.y == file.linesAmount()) return;

	moveRight();
	while(file.getLine()[file.getCaretX()] != ' ' && file.getLine()[file.getCaretX()] != '\t' && file.getCaretX() != file.getLineSize()) {
		moveRight();
	}
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
		caret.savedX = caret.x;
	} catch(std::string e) {
		setStatus(e, PAIR_ERROR);
	}
}
void Editor::deleteCharR() {
	try {
		file.del(true);
		caret.savedX = caret.x;
	} catch(std::string e) {
		setStatus(e, PAIR_ERROR);
	}
}

void Editor::saveFile() {
	if(file.getPath() != "") {
		if (file.hasWritePermission()) {
			file.save();
			setStatus(" File \'" + file.getFullFilename() + "\' has been saved. ", PAIR_INFO);
		} else {
			setStatus(" File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_ERROR);
		}
	} else {
		std::string status {" Specify file name: "};
		std::string fileName{};
		setStatus(status, PAIR_INFO);
		draw();
		int input{};
		Caret saveCaret{};
		move(getmaxy(stdscr) - 1, saveCaret.x + status.length());
		while (true) {
			input = getch();
			if(input == 10) break;
			if(input >= 32 && input < 127) {
				fileName.insert(saveCaret.x, 1, (char)input);
				saveCaret.x++;
			}
			if(input == KEY_RIGHT) {
				if(saveCaret.x < (int)fileName.length()) saveCaret.x++;
			}
			if(input == KEY_LEFT) {
				if(saveCaret.x > 0) saveCaret.x--;
			}
			if(input == 127 && !fileName.empty()) { // BACKSPACE
				fileName.erase(saveCaret.x - 1, 1);
				if(saveCaret.x > 0) saveCaret.x--;
			} 
			if(input == 330 && !fileName.empty()) { // DEL
				fileName.erase(saveCaret.x, 1);
			}
			if(input == 27 || input == 3) { // ESCAPE or ctrl+c
				resetStatus();
				draw();
				return;
			}
			setStatus((std::string{status + fileName + " "}).c_str(), PAIR_INFO);
			draw();
			move(getmaxy(stdscr) - 1, saveCaret.x + status.length());
		}
		
		if(fileName.empty()) {
			setStatus(" File not saved because no name specified ", PAIR_INFO);
		} else {
			file.saveAs(fileName);
			setStatus(" File \'" + file.getFullFilename() + "\' has been saved. ", PAIR_INFO);
		}
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
	std::string s{};
	#ifndef NDEBUG
	sprintf(
		buffer, 
		" File: %s | c.x %2d, c.y %2d, c.sx %2d | f.x %2d, f.y %2d | s.x %2d, s.y %2d | COLORS: %2d", 
		file.getFullFilename().c_str(), 
		caret.x, caret.y, caret.savedX, 
		file.getCaretX(), file.getCaretY(),
		scrollX, scrollY,
		COLORS
	);
	s = buffer;
	#else
	sprintf(
		buffer, 
		" File: %s", 
		file.getFullFilename() != "" ? file.getFullFilename().c_str() : "not specified"
	);
	std::string left {buffer};
	sprintf(
		buffer, 
		"Row: %2d, Col: %2d ", 
		caret.y + 1, caret.x + 1
	);
	std::string right {buffer};
	s = left;
	s.resize(getmaxx(stdscr), ' ');
	s.insert(s.length() - right.size(), right);
	#endif
	setStatus(s);
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
