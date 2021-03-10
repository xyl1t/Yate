#include "editor.hpp"
#include <ncurses.h>
#include <unordered_map>

// Color pairs defines:
#define PAIR_STANDART 1
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

	// NOTE: Define your statuses here:
	addSimpleStatus("saved", "File: \'" + file.getPath() + "\'\twas saved.");

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

	if (this->standart) {
		mvprintw(getmaxy(stdscr) - 1, 0, " File: %s\tRow %2d, Col %2d ", file.getFullFilename().c_str(), file.getCarretY() + 1, file.getCarretX() + 1);
	} else {
		mvprintw(getmaxy(stdscr) - 1, 0, this->statuses.extract(this->custom).mapped().c_str());
	}

	attroff(COLOR_PAIR(this->colorPair));
	// Reset color pair:
	this->colorPair = PAIR_STANDART;
	attroff(A_STANDOUT);

	std::string cursorText = file.getLine(file.getCarretY()).substr(0, file.getCarretX());
    mvprintw(file.getCarretY() - scrollY, 4, cursorText.c_str());
	refresh();
}

void Editor::getInput() {
	int input = getch();

	// Reset status, if input recieved
	this->standart = true;

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
		enableSimpleStatus("saved");
		applyColorPairToStatusBar(PAIR_INFO);
		file.save();
	} else if(input == 3) { // Ctrl-C
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

// Adds simple status, that can be later enabled with enableSimpleStatus(), dissaprears on character input
void Editor::addSimpleStatus(const std::string& name, const std::string& status_message) {
	this->statuses.insert({name, status_message});
}

// Enables status, that was defined with addSimpleStatus()
void Editor::enableSimpleStatus(const std::string& name) {
	this->standart = false;
	this->custom = name;
}

// Defines color pairs
void Editor::initColorPairs() {
	init_pair(PAIR_ERROR, COLOR_RED, COLOR_BLACK);
	init_pair(PAIR_STANDART, COLOR_WHITE, COLOR_BLACK);
	init_pair(PAIR_WARNING, COLOR_YELLOW, COLOR_BLACK);
	init_pair(PAIR_INFO, COLOR_BLUE, COLOR_BLACK);
}

// Enables color pair provided by parameter in the status bar
void Editor::applyColorPairToStatusBar(const int& colorPair) {
	this->colorPair = colorPair;
}