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
	if (isPreprocessor) {
		this->isPreprocessor = false;
		this->afterPreprocessor = true;
		attrPrint(symbol, "preprocessor", extension);
	} else if (isComment || isMultilineComment) {
		if (symbol == splitBySpecialChars(this->featuremaps.at(extension).at("multiline_comment").second)[2]) {
			isMultilineComment = false;
		}
		simpleAttrPrint(symbol, PAIR_SYNTAX_GREEN);
	} else if (afterPreprocessor) {
		attrPrint(symbol, "after_preprocessor", extension); 
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
		simpleAttrPrint(symbol, PAIR_OPEN_CLOSE_SYMBOL);
	} else if (symbol == ")") {
		if (parenthesisThirdLayer) {
			parenthesisThirdLayer = false;
		} else if (parenthesisSecondLayer) {
			parenthesisSecondLayer = false;
		} else {
			parenthesisFirstLayer = false;
		}
		simpleAttrPrint(symbol, PAIR_OPEN_CLOSE_SYMBOL);
	} else if (symbol == "[") {
		if (squareFirstLayer) {
			squareSecondLayer = true;
		} else if (squareSecondLayer) {
			squareThirdLayer = true;
		} else {
			squareFirstLayer = true;
		}
		
		simpleAttrPrint(symbol, PAIR_OPEN_CLOSE_SYMBOL);
	} else if (symbol == "]") {
		if (squareThirdLayer) {
			squareThirdLayer = false;
		} else if (squareSecondLayer) {
			squareSecondLayer = false;
		} else {
			squareFirstLayer = false;
		}
			
		simpleAttrPrint(symbol, PAIR_OPEN_CLOSE_SYMBOL);
	} else if (symbol == "{") {
		if (curlyFirstLayer) {
			curlySecondLayer = true;
		} else if (curlySecondLayer) {
			curlyThirdLayer = true;
		} else {
			curlyFirstLayer = true;
		}
		simpleAttrPrint(symbol, PAIR_OPEN_CLOSE_SYMBOL);
	} else if (symbol == "}") {
		if (curlyThirdLayer) {
			curlyThirdLayer = false;
		} else if (curlySecondLayer) {
			curlySecondLayer = false;
		} else {
			curlyFirstLayer = false;
		}
		simpleAttrPrint(symbol, PAIR_OPEN_CLOSE_SYMBOL);
	} else if (hasHashmap(extension) || hasFeaturemap(extension)) {
		if (isKeyword(symbol, extension)) {
			attrPrint(symbol, "keyword", extension);
		} else if (isString) {
			if (symbol == "\"" || symbol == "\'") {
				isString = false;
			}
			attrPrint(symbol, "__reserved_string_color__", extension);
		} else if (symbol == "\"" || symbol == "\'") {
			isString = true;
			attrPrint(symbol, "__reserved_string_color__", extension);
		} else if (this->featuremaps.at(extension).at("preprocessor").second != "" && symbol == this->featuremaps.at(extension).at("preprocessor").second.c_str()) {
			this->isPreprocessor = true;
			attrPrint(symbol, "preprocessor", extension);
			
		} else if (this->featuremaps.at(extension).at("statement_end").second != "" && symbol == this->featuremaps.at(extension).at("statement_end").second.c_str()) {
			attrPrint(symbol, "statement_end", extension);
			
		} else if (this->featuremaps.at(extension).at("arrow_pointer").second != "" && symbol == this->featuremaps.at(extension).at("arrow_pointer").second.c_str()) {
			simpleAttrPrint(symbol, PAIR_SYNTAX_RED);
			
		} else if (this->featuremaps.at(extension).at("comment").second != "" && symbol == this->featuremaps.at(extension).at("comment").second.c_str()) {
			isComment = true;
			simpleAttrPrint(symbol, PAIR_SYNTAX_GREEN);
		} else if (this->featuremaps.at(extension).at("multiline_comment").second != "") {
			std::vector<std::string> multilineCommentSymbols = splitBySpecialChars(this->featuremaps.at(extension).at("multiline_comment").second);
			if (symbol == multilineCommentSymbols[0]) {
				isMultilineComment = true;
				simpleAttrPrint(symbol, PAIR_SYNTAX_GREEN);
			} else if (symbol == multilineCommentSymbols[2]) {
				isMultilineComment = false;
				simpleAttrPrint(symbol, PAIR_SYNTAX_GREEN);
			} else {
				printw(symbol.c_str());
			}
		}
	}
}

std::unordered_map<std::string, int> syntaxHighlighter::parseHashmap(std::ifstream hashmap) {
	// TODO: this function
	// Those are to suppress warnings
	hashmap.close();
	std::unordered_map<std::string, int> newHashmap;
	return newHashmap;
}