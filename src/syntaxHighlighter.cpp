#include "editor.hpp"
#include "syntaxHighlighter.hpp"
#include <ncurses.h>
#include <iostream>
#include <vector>

void syntaxHighlighter::parseLine(const std::string& line, int lineNr) {
	printw("%3d ", lineNr + 1);
	std::vector<std::string> list = splitByTabsAndSpaces(line);
	for(size_t i = 0; i < list.size(); i++ ) {
		parseSymbol(list[i]);
	}
}

void syntaxHighlighter::parseSymbol(const std::string& symbol) {
	if (isComment) {
		printw(symbol.c_str());
	}

	if (symbol == " " || symbol == "\t") {
		printw(symbol.c_str());
	}
	if (symbol == "(") {
		if (parenthesisFirstLayer) {
			parenthesisSecondLayer = true;
		} else if (parenthesisSecondLayer) {
			parenthesisThirdLayer = true;
		} else {
			parenthesisFirstLayer = true;
		}
			attron(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
		printw(symbol.c_str());
		attroff(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
	} else if (symbol == ")") {
		if (parenthesisThirdLayer) {
			parenthesisThirdLayer = false;
		} else if (parenthesisSecondLayer) {
			parenthesisSecondLayer = false;
		} else {
			parenthesisFirstLayer = false;
		}
			attron(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
		printw(symbol.c_str());
		attroff(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
	} else if (symbol == "[") {
		if (squareFirstLayer) {
			squareSecondLayer = true;
		} else if (squareSecondLayer) {
			squareThirdLayer = true;
		} else {
			squareFirstLayer = true;
		}
		
		attron(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
		printw(symbol.c_str());
		attroff(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
	} else if (symbol == "]") {
		if (squareThirdLayer) {
			squareThirdLayer = false;
		} else if (squareSecondLayer) {
			squareSecondLayer = false;
		} else {
			squareFirstLayer = false;
		}
			
		attron(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
		printw(symbol.c_str());
		attroff(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
	} else if (symbol == "{") {
		if (curlyFirstLayer) {
			curlySecondLayer = true;
		} else if (curlySecondLayer) {
			curlyThirdLayer = true;
		} else {
			curlyFirstLayer = true;
		}
			attron(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
		printw(symbol.c_str());
		attroff(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
	} else if (symbol == "}") {
		if (curlyThirdLayer) {
			curlyThirdLayer = false;
		} else if (curlySecondLayer) {
			curlySecondLayer = false;
		} else {
			curlyFirstLayer = false;
		}
		
		attron(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
		printw(symbol.c_str());
		attroff(COLOR_PAIR(PAIR_OPEN_CLOSE_SYMBOL));
	}
}

std::unordered_map<std::string, int> syntaxHighlighter::parseHashmap(std::ifstream hashmap) {
	// TODO: this function
}