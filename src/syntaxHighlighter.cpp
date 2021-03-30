#include "editor.hpp"
#include "syntaxHighlighter.hpp"
#include <ncurses.h>
#include <iostream>
#include <vector>

syntaxHighlighter::syntaxHighlighter() {
	this->initMoreColors();
	this->registerDefaults();
}

void pushString(std::vector<std::pair<size_t, std::string>>& vector, const std::string& line, const std::string& string) {
	vector.push_back({line.find(string), string});
}

std::vector<std::string> syntaxHighlighter::splitBySpecialChars(const std::string& line, const std::string& extension) {
	std::vector<std::pair<size_t, std::string>> positions;
	std::string newExtension = extension;
	replaceStringInPlace(newExtension, ".", "");
	// Custom, not necessary ones
	if (syntaxHighlighter::hasFeaturemap(newExtension)) {
		for (auto it: this->featuremaps[newExtension]) {
			if (it.second.first) {
				if (it.first == "multiline_comment") {
					int del = it.second.second.find("\t");
					std::string open = it.second.second.substr(0, del);
					std::string close = it.second.second.substr(del+1);
					pushString(positions, line, open);
					pushString(positions, line, close);
				} else {
					pushString(positions, line, it.second.second);
				}
			}
		}
	}
	// Defaults (must-have) ones
	pushString(positions, line, "\t");
	pushString(positions, line, " ");
	pushString(positions, line, "(");
	pushString(positions, line, "[");
	pushString(positions, line, "{");
	pushString(positions, line, ")");
	pushString(positions, line, "]");
	pushString(positions, line, "}");
	size_t initialPos = 0;
	std::vector<std::string> finalVector;
	finalVector.clear();
	while(true) {
		int num = 0;
		for ( int i = 0; i < static_cast<int>(positions.size()); i++ ) {
			if (positions[i].first == std::string::npos) {
				num += 1;
			}
		}
		if (num == static_cast<int>(positions.size())) {
			int max = 0;
			for (int i = 0; i < static_cast<int>(positions.size() + 1); i++ ) {
				if (positions[i].first > positions[i+1].first) {
					if (positions[i].first > positions[max].first) {
						max = i;
					}
				} else if (positions[i+1].first > positions[max].first) {
					max = i+1;
				}
			}
			finalVector.push_back(line.substr(initialPos,std::min(positions[max].first, line.size()) - initialPos + 1));
			// TODO: Fix this behaviour
			// Not sure why this is even happening
			// Still slowing down calculations
			// So it should be fixed
			for ( int i = 0; i < static_cast<int>(finalVector.size()); i++ ) {
				if (finalVector[i] == "") {
					finalVector.erase(finalVector.begin() + i);
				}
			}
			return finalVector;
		}
		int min = 0;
		for (int i = 0; i < static_cast<int>(positions.size() - 1); i++ ) {
			if (positions[i].first < positions[i+1].first) {
				if (positions[i].first < positions[min].first) {
					min = i;
				}
			} else if (positions[i+1].first < positions[min].first) {
				min = i+1;
			}
		}
		finalVector.push_back(line.substr(initialPos, positions[min].first - initialPos));
		finalVector.push_back(positions[min].second);
		if (positions[min].second.size() == 1) {
			initialPos = positions[min].first + 1;
		} else {
			initialPos = positions[min].first + positions[min].second.size();
		}
		positions[min].first = line.find(positions[min].second, initialPos);
	}
}

featuremap syntaxHighlighter::createFeaturemap(std::vector<bool> status = {true, true, true, true, true}, std::vector<std::string> symbols = {"#", ";", "//", "->", "/*	*/"}) {
	std::vector<std::string> names = {"preprocessor", "statement_end", "comment", "arrow_pointer", "multiline_comment"};
	if (!(names.size() == status.size() && status.size() == symbols.size() && symbols.size() == names.size())) {
		return {};
	} 
	featuremap final{names.size()};
	for (int i = 0; i <= static_cast<int>(names.size() - 1); i++ ) {
		final[names.at(i)] = {status.at(i), symbols.at(i)};
	}
	return final;
}

colormap syntaxHighlighter::createColormap(std::vector<std::string> names = {"not_supported"}, std::vector<int> colors = {0}, std::vector<int> backup = {0}) {
	if (names[0] == "not_supported") {
		return {};
	}
	if (!(names.size() == colors.size() && colors.size() == backup.size() && backup.size() == names.size())) {
		return {};
	} 
	colormap final{names.size()};
	for (int i = 0; i <= static_cast<int>(names.size() - 1); i++ ) {
		final[names[i]] = {colors[i], backup[i]};
	}
	return final;
}

void syntaxHighlighter::registerMaps(std::string extension, std::vector<std::string> keywordmap, featuremap featuremap, colormap colormap) {
	this->featuremaps[extension] = featuremap;
	this->colormaps[extension] = colormap;
	this->keywordmaps[extension] = keywordmap;
}

void syntaxHighlighter::parseLine(const std::string& line, int lineNr, const std::string& extension, Editor editor) {
	resetLineSpecificFlags();
	printw("%3d ", lineNr + 1);
	std::vector<std::string> list = splitBySpecialChars(line, extension);
	for(size_t i = 0; i < list.size(); i++ ) {
		parseSymbol(list[i], std::string(extension).replace(0, 1, ""), editor);
	}
}

void syntaxHighlighter::parseSymbol(const std::string& symbol, const std::string& extension, Editor editor) {
	int virtualCol = 0;
	int min = editor.getTextEditorWidth();
	if (symbol == "\t") {
		const int tabSize = editor.getTabSize() - (virtualCol) % editor.getTabSize();
		for (int original = virtualCol; virtualCol < original + tabSize; virtualCol++) {
			if (virtualCol >= editor.getScrollX() && virtualCol - editor.getScrollX() < min) {
				printw("cring");
			}
		}
	} else {
		if (isPreprocessor) {
			this->isPreprocessor = false;
			this->afterPreprocessor = true;
			attrPrint(symbol, "preprocessor", extension);
		} else if (isComment || isMultilineComment) {
			if (symbol == splitBySpecialChars(this->featuremaps.at(extension).at("multiline_comment").second, extension)[2]) {
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
		} else if (hasColormap(extension) || hasFeaturemap(extension)) {
			// For C/C++
			std::string original = symbol;
			std::string mutableSymbol = symbol;
			replaceStringInPlace(mutableSymbol, "*", "");
			replaceStringInPlace(mutableSymbol, "&", "");
			if (isKeyword(mutableSymbol, extension)) {
				attrPrint(mutableSymbol, "keyword", extension);
				replaceStringInPlace(original, mutableSymbol, "");
				printw(original.c_str());
			} else if (isString) {
				if (symbol == "\"" || symbol == "\'") {
					isString = false;
				}
				attrPrint(symbol, "string_color", extension);
			} else if (symbol == "\"" || symbol == "\'") {
				isString = true;
				attrPrint(symbol, "string_color", extension);
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
				std::vector<std::string> multilineCommentSymbols = splitBySpecialChars(this->featuremaps.at(extension).at("multiline_comment").second, extension);
				if (symbol == multilineCommentSymbols[0]) {
					isMultilineComment = true;
					simpleAttrPrint(symbol, PAIR_SYNTAX_GREEN);
				} else if (symbol == multilineCommentSymbols[2]) {
					if (isMultilineComment) {
						isMultilineComment = false;
						simpleAttrPrint(symbol, PAIR_SYNTAX_GREEN);
					} else {
						printw(symbol.c_str());
					}
				} else {
					printw(symbol.c_str());
				}
			}
		}
	}
}

colormap syntaxHighlighter::parseColormap(std::ifstream colormap_file) {
	// TODO: this function
	// Those are to suppress warnings
	colormap_file.close();
	colormap newColormap;
	return newColormap;
}

void syntaxHighlighter::registerDefaults() {
	colormap cColormap = createColormap(
		{"preprocessor", "after_preprocessor", "keyword", "string_color", "statement_end"},
		{PAIR_SYNTAX_RED, PAIR_SYNTAX_MAGENTA, PAIR_SYNTAX_CYAN, PAIR_SYNTAX_RED, PAIR_SYNTAX_YELLOW},
		{PAIR_SYNTAX_RED, PAIR_SYNTAX_MAGENTA, PAIR_SYNTAX_CYAN, PAIR_SYNTAX_RED, PAIR_SYNTAX_YELLOW}
	);

	keywordmap cKeywordmap = {
		"int",
		"float",
		"void",
		"nullptr",
		"char",
		"long",
		"unsigned",
		"signed",
		"short",
		"string",
		"using",
		"namespace",
		"class",
		"private",
		"protected",
		"public"
	};

	featuremap cFeaturemap = createFeaturemap();

	registerMaps("c", cKeywordmap, cFeaturemap, cColormap);
	registerMaps("h", cKeywordmap, cFeaturemap, cColormap);
	registerMaps("cpp", cKeywordmap, cFeaturemap, cColormap);
	registerMaps("hpp", cKeywordmap, cFeaturemap, cColormap);

	colormap pythonColormap = createColormap(
		{"preprocessor", "after_preprocessor", "keyword", "string_color", "statement_end"},
		{PAIR_SYNTAX_CYAN, PAIR_SYNTAX_YELLOW, PAIR_SYNTAX_BLUE, PAIR_SYNTAX_GREEN, PAIR_SYNTAX_RED},
		{PAIR_SYNTAX_CYAN, PAIR_SYNTAX_YELLOW, PAIR_SYNTAX_BLUE, PAIR_SYNTAX_GREEN, PAIR_SYNTAX_RED}
	);

	keywordmap pythonKeywordmap = {
		"True",
		"False",
		"None",
		"int",
		"float",
		"string",
		"char",
		"list"
	};

	featuremap pythonFeaturemap = createFeaturemap(
		{true, false, true, false, true},
		{"import", "", "#", "", "'''\t'''"}
	);

	registerMaps("py", pythonKeywordmap, pythonFeaturemap, pythonColormap);
}