#include "editor.hpp"
#include <ncurses.h>

Editor::Editor(const std::string& filePath) 
	: file(filePath), 
	scrollX(0),
	scrollY(0),
	width(getmaxx(stdscr)), 
	height(getmaxy(stdscr)) {
	
}

void Editor::draw() {
	for (int lineNr = scrollY; lineNr < scrollY + height && lineNr < file.linesAmount(); lineNr++) {
		std::string_view line { file.getLine(lineNr) };
		move(lineNr, 0);
		printw(line.data());
	}
	move(file.getCarretY() - scrollY, file.getCarretX() - scrollX);
	refresh();
}

void Editor::getInput() {
	int input = getch();
	switch(input) {
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
		
		case KEY_END:
		break;
		
		case KEY_HOME:
		break;
		
		case KEY_ENTER:
		break;

		case KEY_BACKSPACE:
		break;
		
		default:
			if(input >= 0 && input <= 0x7F) {
				file.put(static_cast<char>(input));
			}
		break;
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
	if((scrollY + height) - file.getCarretY() == 0)
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
