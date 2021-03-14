#include "editor.hpp"
#include "syntaxHighlighter.hpp"
#include <ncurses.h>
#include <iostream>
#include <vector>

void syntaxHighlighter::parseLine(const std::string& line, int lineNr, const std::string& extension) {
	resetLineSpecificFlags();
	printw("%3d ", lineNr + 1);
	std::vector<std::string> list = splitBySpecialChars(line);
	for(size_t i = 0; i < list.size(); i++ ) {
		parseSymbol(list[i], std::string(extension).replace(0, 1, ""));
	}
}

void syntaxHighlighter::parseSymbol(const std::string& symbol, const std::string& extension) {
	if (isComment || isMultilineComment) {
		if (symbol == splitBySpecialChars(supportMultilineComment(extension))[2]) {
			isMultilineComment = false;
		}
		if (!( COLORS > 8 && can_change_color())) {
			attron(COLOR_PAIR(PAIR_SYNTAX_GREEN));
		} else {
			// Any custom pair can go here, not only defaults:
			attron(COLOR_PAIR(PAIR_SYNTAX_GREEN));
		}
		printw(symbol.c_str());
		attroff(COLOR_PAIR(PAIR_SYNTAX_GREEN));
		
	} else if (isString) {
		if (symbol == "\"") {
			isString = false;
		}
		if (!( COLORS > 8 && can_change_color())) {
			attron(COLOR_PAIR(PAIR_SYNTAX_RED));
		} else {
			// Any custom pair can go here, not only defaults:
			attron(COLOR_PAIR(PAIR_SYNTAX_RED));
		}
		printw(symbol.c_str());
		attroff(COLOR_PAIR(PAIR_SYNTAX_RED));
	} else if (symbol == "\"") {
		isString = true;
		if (!( COLORS > 8 && can_change_color())) {
			attron(COLOR_PAIR(PAIR_SYNTAX_RED));
		} else {
			// Any custom pair can go here, not only defaults:
			attron(COLOR_PAIR(PAIR_SYNTAX_RED));
		}
		printw(symbol.c_str());
		attroff(COLOR_PAIR(PAIR_SYNTAX_RED));
	} else if (symbol == " " || symbol == "\t") {
		printw(symbol.c_str());
	} else if (symbol == "(") {
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
	} else if (hasFeaturemap(extension)) {
		if (supportCommandEnding(extension) != "" && symbol == supportCommandEnding(extension).c_str()) {
			if (!( COLORS > 8 && can_change_color())) {
				attron(COLOR_PAIR(PAIR_SYNTAX_YELLOW));
			} else {
				// Any custom pair can go here, not only defaults:
				attron(COLOR_PAIR(PAIR_SYNTAX_YELLOW));
			}
			printw(symbol.c_str());
			attroff(COLOR_PAIR(PAIR_SYNTAX_YELLOW));
			
		} else if (supportArrowPointer(extension) != "" && symbol == supportArrowPointer(extension).c_str()) {
			if (!( COLORS > 8 && can_change_color())) {
				attron(COLOR_PAIR(PAIR_SYNTAX_RED));
			} else {
				// Any custom pair can go here, not only defaults:
				attron(COLOR_PAIR(PAIR_SYNTAX_RED));
			}
			printw(symbol.c_str());
			attroff(COLOR_PAIR(PAIR_SYNTAX_RED));
			
		} else if (supportComment(extension) != "" && symbol == supportComment(extension).c_str()) {
			isComment = true;
			if (!( COLORS > 8 && can_change_color())) {
				attron(COLOR_PAIR(PAIR_SYNTAX_GREEN));
			} else {
				// Any custom pair can go here, not only defaults:
				attron(COLOR_PAIR(PAIR_SYNTAX_GREEN));
			}
			printw(symbol.c_str());
			attroff(COLOR_PAIR(PAIR_SYNTAX_GREEN));
		} else if (supportMultilineComment(extension) != "") {
			std::vector<std::string> multilineCommentSymbols = splitBySpecialChars(supportMultilineComment(extension));
			if (symbol == multilineCommentSymbols[0]) {
				isMultilineComment = true;
				if (!( COLORS > 8 && can_change_color())) {
					attron(COLOR_PAIR(PAIR_SYNTAX_GREEN));
				} else {
					// Any custom pair can go here, not only defaults:
					attron(COLOR_PAIR(PAIR_SYNTAX_GREEN));
				}
				printw(symbol.c_str());
				attroff(COLOR_PAIR(PAIR_SYNTAX_GREEN));
			} else if (symbol == multilineCommentSymbols[2]) {
				isMultilineComment = false;
				if (!( COLORS > 8 && can_change_color())) {
					attron(COLOR_PAIR(PAIR_SYNTAX_GREEN));
				} else {
					// Any custom pair can go here, not only defaults:
					attron(COLOR_PAIR(PAIR_SYNTAX_GREEN));
				}
				printw(symbol.c_str());
				attroff(COLOR_PAIR(PAIR_SYNTAX_GREEN));
			} else {
				printw(symbol.c_str());
			}
		}
	} else {
		printw(symbol.c_str());
	}
}

std::unordered_map<std::string, int> syntaxHighlighter::parseHashmap(std::ifstream hashmap) {
	// TODO: this function
	// Those are to suppress warnings
	hashmap.close();
	std::unordered_map<std::string, int> newHashmap;
	return newHashmap;
}