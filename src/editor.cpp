#include "editor.hpp"
#include <ncurses.h>
#include <unordered_map>

// Color pairs defines:
#define PAIR_STANDARD 1
#define PAIR_ERROR 2
#define PAIR_WARNING 3
#define PAIR_INFO 4

Editor::Editor(const std::string& filePath) 
	: file(filePath), 
	scrollX(0),
	scrollY(0),
	width(getmaxx(stdscr)), 
	height(getmaxy(stdscr) - 2),
    alive(true) {

	initColorPairs();
}

void Editor::draw() {
	clear();
	for (int lineNr = scrollY; lineNr < scrollY + height && lineNr < file.linesAmount(); lineNr++) {
		const std::string& line { file.getLine(lineNr) };
		move(lineNr - scrollY, 0);
		printw("%3d %s", lineNr + 1, line.c_str());
	}

	attron(A_STANDOUT);
	attron(COLOR_PAIR(this->colorPair));

	// Pre-print hook
	hook("pre-print");

	if (this->standard_status) {
		mvprintw(getmaxy(stdscr) - 1, 0, " File: %s\tRow %2d, Col %2d ", file.getFullFilename().c_str(), file.getCarretY() + 1, file.getCarretX() + 1);
	} else {
		mvprintw(getmaxy(stdscr) - 1, 0, this->custom_message.c_str());
		// Post-print hook
		hook("post-print");
	}

	attroff(COLOR_PAIR(this->colorPair));
	// Reset color pair:
	this->colorPair = PAIR_STANDARD;
	attroff(A_STANDOUT);

	std::string cursorText = file.getLine(file.getCarretY()).substr(0, file.getCarretX());
    mvprintw(file.getCarretY() - scrollY, 4, cursorText.c_str());
	refresh();
	// Post-refresh hook
	hook("post-refresh");
}

void Editor::getInput() {
	int input = getch();

	// Raw-input hook
	hook("raw-input");

	// Reset status, if input recieved
	this->standard_status = true;

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
    } else if(input == 19) { // Ctrl-S
		printColoredStatus(" File: \'" + file.getFullFilename() + "\'  has been saved. ", PAIR_INFO);
		file.save();
	} else if(input == 3) { // Ctrl-C
		file.close();
		// NOTE: Maybe instead of exiting without saving, ask the user if he wants to save
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
	try{
		file.del(false);
    	if(file.getCarretY() - scrollY < 0) {
        	scrollY--;
    	}
	} catch(std::string e) {
		printColoredStatus(e, PAIR_ERROR);
	}
}
void Editor::deleteCharR() {
	try{
		file.del(true);
	} catch(std::string e) {
		printColoredStatus(e, PAIR_ERROR);
	}
}

// Reworked, more clean, status function
void Editor::printColoredStatus(const std::string& message, int colorPair) {
	this->custom_message = message;
	this->standard_status = false;
	this->colorPair = colorPair;
}

// Defines color pairs
void Editor::initColorPairs() {
	init_pair(PAIR_ERROR, COLOR_WHITE, COLOR_RED);
	init_pair(PAIR_STANDARD, COLOR_WHITE, COLOR_BLACK);
	init_pair(PAIR_WARNING, COLOR_WHITE, COLOR_YELLOW);
	init_pair(PAIR_INFO, COLOR_WHITE, COLOR_BLUE);
}

// Placeholder for complex statuses (or maybe more), not sure if we'll need it, but let's keep it for now.
void Editor::hook(const std::string& stage) {
	/* TODO: make this function
	Plan:
		1. Iterate over all instances of "complexStatus" class in "statuses.cpp"
		2. Get a list of functions to current stage
		3. Pass `this` object to those functions
		4. Invoke them

	Basically, it's a more complex status, 
	that can execute functions, 
	with Editor's functions and variables,
	and can do litreall much more.
	(Sounds more like a mod api to be honest,
	but i believe it may be useful for integrating into code later)
	*/
}