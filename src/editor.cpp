#include "editor.hpp"
#include <ncurses.h>
#include <unordered_map>
#include <algorithm>
#include <vector>
// Color pairs defines:
#define PAIR_STANDARD 1
#define PAIR_ERROR 2
#define PAIR_WARNING 3
#define PAIR_INFO 4

Editor::Editor(const std::string& filePath, int tabSize) 
    : alive(true),
	file(filePath), 
	TAB_SIZE(tabSize),
	caret(),
	scrollX(0),
	scrollY(0),
	customStatusText(false),
	undo{},
	redo{} 
{
	initColorPairs();
	resetStatus();
	customStatusText = false;
	if (!file.hasWritePermission()) {
		setStatus(" File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_WARNING);
	}
	if (!file.getInfoMessage().empty()) {
		setStatus((std::string)(" " + file.getInfoMessage() + " "), PAIR_WARNING);
	}
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
	for (int lineNr = scrollY; lineNr < scrollY + getTextEditorHeight() && lineNr < file.linesAmount(); lineNr++) {
		std::string line { file.getLine(lineNr) };
		int min = getTextEditorWidth();
		int virtualCol = 0;
		
		move(lineNr - scrollY, 0);
		printw("%3d ", lineNr + 1);
		for (char ch : line) {
			if(ch == '\t') {
				const int tabSize = TAB_SIZE - (virtualCol) % TAB_SIZE;
				for(int original = virtualCol; virtualCol < original + tabSize; virtualCol++) {
					if(virtualCol >= scrollX && virtualCol - scrollX < min) {
						printw(" ");
					}
				}
			} else {
				if(virtualCol >= scrollX && virtualCol - scrollX < min) {
					printw("%c", ch);
				}
				virtualCol++;
			}
		}
	}

	// turn on and set color for status
	attron(A_STANDOUT);
	attron(COLOR_PAIR(this->colorPair));
	
	// print status at bottom of screen
	this->statusText.resize(getmaxx(stdscr), ' ');
	mvprintw(getmaxy(stdscr) - 1, 0, "%s", this->statusText.c_str());

	attroff(COLOR_PAIR(this->colorPair));
	// Reset color pair:
	this->colorPair = PAIR_STANDARD;
	attroff(A_STANDOUT);

	// std::string cursorText = file.getLine(file.getCaretY()).substr(0, file.getCaretX());
	// mvprintw(file.getCaretY() - scrollY, 4, cursorText.c_str());
	
	move(caret.y - scrollY, caret.x - scrollX + 4);
	refresh();
}

int Editor::getInput() {
	prevAction = currentAction;
	currentAction = getch();
	
	auto actionCount = undo.size() + redo.size();
	
	if(Action::isInput(currentAction)) {
		put(currentAction);
		if(!file.hasWritePermission()) {
			setStatus(" Warning: File \'" + file.getFullFilename() + "\' doesn't have write permissions. ", PAIR_WARNING);
		}
	}
	else
	{
		switch(currentAction)
		{
			case 21: // CTRL+U
				if(!undo.empty() && undo.top().actionType == Actions::separator.actionType) {
					redo.emplace(undo.top());
					undo.pop();
				}
				while(!undo.empty()) {
					Action act{undo.top()};
					if(act.actionType != Actions::separator.actionType) {
						act.undoAction();
						redo.emplace(act);
					}
					undo.pop();
					if (undo.empty() || undo.top().actionType == Actions::separator.actionType) {
						break;
					}
				}
				break;
			
			case 18: // CTRL+R
				if(!redo.empty() && redo.top().actionType == Actions::separator.actionType) {
					undo.emplace(redo.top());
					redo.pop();
				}
				while(!redo.empty()) {
					Action act{redo.top()};
					if(act.actionType != Actions::separator.actionType) {
						act.doAction();
						undo.emplace(act);
					}
					redo.pop();
					if (redo.empty() || redo.top().actionType == Actions::separator.actionType) {
						break;
					}
				}
				break;
			
			case 11: // CTRL+K
				scrollLeft();
				break;
			case 12:  // CTRL+L
				scrollRight();
				break;

			case KEY_PPAGE:
				setCaretLocation(caret.x, caret.y - (getTextEditorHeight() - 1));
				if(!undo.empty() && undo.top().actionType != Actions::separator.actionType)
					undo.push(Actions::separator);
				break;
			case KEY_NPAGE:
				setCaretLocation(caret.x, caret.y + (getTextEditorHeight() - 1));
				if(!undo.empty() && undo.top().actionType != Actions::separator.actionType)
					undo.push(Actions::separator);
				break;
			case KEY_UP:
				moveUp();
				break;
			case KEY_DOWN:
				moveDown();
				if(!undo.empty() && undo.top().actionType != Actions::separator.actionType)
					undo.push(Actions::separator);
				break;
			case KEY_LEFT:
				moveLeft();
				if(!undo.empty() && undo.top().actionType != Actions::separator.actionType)
					undo.push(Actions::separator);
				break;
			case KEY_RIGHT:
				moveRight();
				if(!undo.empty() && undo.top().actionType != Actions::separator.actionType)
					undo.push(Actions::separator);
				break;
			case 5:
			case KEY_END:
				moveEndOfLine(); 
				if(!undo.empty() && undo.top().actionType != Actions::separator.actionType)
					undo.push(Actions::separator);
				break;
			case KEY_HOME:
			case 1:
				moveBeginningOfLine();
				if(!undo.empty() && undo.top().actionType != Actions::separator.actionType)
					undo.push(Actions::separator);
				break;
			case 25: // CTRL+Y (for qwertz layout)
			case 26: // CTRL+Z (for qwerty layout)
				moveBeginningOfText(); 
				if(!undo.empty() && undo.top().actionType != Actions::separator.actionType)
					undo.push(Actions::separator);
				break;
			case 24: // CTRL+X
				moveEndOfText(); 
				if(!undo.empty() && undo.top().actionType != Actions::separator.actionType)
					undo.push(Actions::separator);
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
	if(!(currentAction == 11 || currentAction == 12))
		scrollToCaret();
	
	// if the undo stack changed, that means something changed in the file, therefor flush the redo stack
	if (undo.size() + redo.size() != actionCount) {
		while (!redo.empty()) redo.pop();
	}
	
	// Reset status on user input if no custom status was applied, if there is a custom status, let it display first and then reset
	if(!customStatusText) {
		resetStatus();
	}
	customStatusText = false;
	
#ifndef NDEBUG
		setStatus(this->statusText + "\tinput: " + std::to_string(currentAction), this->colorPair);
		customStatusText = false;
#endif
	
	return currentAction;
}

void Editor::put(int ch, bool record) {
	ActionType actType = ActionType::Input;
	int beforeX = caret.x;
	int beforeY = caret.y;
	
	if(ch != KEY_ENTER && ch != 10) {
		file.put((char)ch);
		moveRight();
	} else {
		newLine();
		actType = ActionType::NewLine;
	}
	
	if (record) {
		if(!undo.empty() && undo.top().actionType != Actions::separator.actionType && undo.top().actionType != ActionType::Input) {
			undo.emplace(Actions::separator);
		} 
		
		undo.emplace((Action){
			Action::isNewLine(currentAction) ? ActionType::NewLine : ActionType::Input,
			currentAction, caret.x, caret.y,
			[this, x = caret.x, y = caret.y] {
				setCaretLocation(x, y);
				deleteCharL(false);
			},
			[this, act = currentAction, x = beforeX, y = beforeY] {
				setCaretLocation(x, y);
				put(static_cast<char>(act), false);
			}
		});

		if(ch == ' ') {
			undo.emplace(Actions::separator);
		}
		
		if(ch == '\n') {
			undo.emplace(Actions::separator);
		}
	}
}
void Editor::newLine() {
	file.newLine();
    moveDown();
	setCaretLocation(0, caret.y);
}
void Editor::deleteCharL(bool record) {
	try {
		char c{};
		if(caret.x == 0) {
			moveLeft();
			c = '\n';
			file.del(true);
		} else {
			c = file.getLine(caret.y)[getFileCaretColumn(caret.x - 1)];
			file.del(false);
			moveLeft();
		}
    	if(file.getCaretY() - scrollY < 0) {
        	scrollY--;
    	}
		caret.savedX = caret.x;
		
		if (record) {
			if(!undo.empty() && undo.top().actionType != Actions::separator.actionType  && undo.top().actionType != ActionType::DeletionL) {
				undo.emplace(Actions::separator);
			} 
			
			undo.emplace((Action) {
				ActionType::DeletionL,
				currentAction, caret.x, caret.y,
				[this, c, x = caret.x, y = caret.y] {
					setCaretLocation(x, y);
					put(c, false);
				},
				[this, c, x = caret.x, y = caret.y] {
					if(c != '\n')
						setCaretLocation(x + 1, y);
					else 
						setCaretLocation(0, y + 1);
					draw();
					deleteCharL(false);
				}
			});
			
			if (c == '\n') {
				undo.emplace(Actions::separator);
			}
		}
	} catch(std::string e) {
		setStatus(e, PAIR_ERROR);
	}
	
}
void Editor::deleteCharR(bool record) {
	try {
		char c = file.getLine(caret.y)[getFileCaretColumn()];
		if(caret.x == getVirtualLineLength()) c = '\n';
		
		file.del(true);
		caret.savedX = caret.x;

		if (record) {
			if(!undo.empty() && undo.top().actionType != Actions::separator.actionType && undo.top().actionType != ActionType::DeletionR) {
				undo.emplace(Actions::separator);
			} 
			
			undo.emplace((Action){
				ActionType::DeletionR,
				currentAction, caret.x, caret.y,
				[this, c, x = caret.x, y = caret.y] {
					setCaretLocation(x, y);
					put(c, false);
					setCaretLocation(x, y);
				},
				[this, x = caret.x, y = caret.y] {
					setCaretLocation(x, y);
					deleteCharR(false);
				}
			});
			
			if (c == '\n') {
				undo.emplace(Actions::separator);
			}
		}
	} catch(std::string e) {
		setStatus(e, PAIR_ERROR);
	}
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
			setStatus(" File not saved because no name specified ", PAIR_WARNING);
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
		" File: %s | c.x %2d, c.y %2d ", 
		file.getFullFilename().c_str(), 
		caret.x, caret.y
	);
	s = buffer;
	
	std::stack<Action> u = undo;
	
	while (!u.empty()) {
		Action act = u.top();
		if(act.action == ' ') {
			s += '_';
		} else if(act.action == 0) {
			s += '|';
		} else if(act.action == '\n') {
			s += "\\n";
		} else if(act.actionType == ActionType::DeletionL || act.actionType == ActionType::DeletionR) {
			s += "\\d";
		} else {
			s += act.action;
		}
		u.pop();
	}
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
	init_pair(PAIR_WARNING, COLOR_WHITE, COLOR_RED);
	init_pair(PAIR_INFO, COLOR_WHITE, COLOR_BLUE);
}
