#ifndef SYNTAX_HIGHLITHER_HPP
#define SYNTAX_HIGHLITHER_HPP
#include <string>
#include <sstream>
#include <unordered_map>
#include <ncurses.h>
#include "editor.hpp"
#define TEMP_COLOR_CUSTOM_MAGENTA 100
#define TEMP_COLOR_CUSTOM_RED 101
#define TEMP_COLOR_CUSTOM_YELLOW 102
#define TEMP_COLOR_CUSTOM_CYAN 103
#define TEMP_COLOR_CUSTOM_GREEN 104

#define COLOR_CUSTOM_MAGENTA 1000
#define COLOR_CUSTOM_RED 1001
#define COLOR_CUSTOM_YELLOW 1002
#define COLOR_CUSTOM_CYAN 1003
#define COLOR_CUSTOM_GREEN 1004

class syntaxHighlighter {
	public:

	inline bool haveFeaturemap(const std::string& extension) const {
		if (featuremaps.find(extension) == featuremaps.end()) {
			return true;
		}
		return false;
	}

	inline std::string supportCommandEnding(const std::string& extension) {
		if (haveFeaturemap(extension)) {
			if (featuremaps.at(extension).at("command_ending").first) return featuremaps.at(extension).at("command_ending").second;
		}
		return "";
	}

	inline std::string supportArrowPointer(const std::string& extension) {
		if (haveFeaturemap(extension)) {
			if (featuremaps.at(extension).at("arrow_pointer").first) return featuremaps.at(extension).at("command_ending").second;
		}
		return "";
	}

	inline std::string supportComment(const std::string& extension) {
		if (haveFeaturemap(extension)) {
			if (featuremaps.at(extension).at("comment").first) return featuremaps.at(extension).at("command_ending").second;
		}
		return "";
	}

	inline std::string supportMultilineComment(const std::string& extension) {
		if (haveFeaturemap(extension)) {
			if (featuremaps.at(extension).at("multiline_comment").first) return featuremaps.at(extension).at("command_ending").second;
		}
		return "";
	}

	inline void initMoreColors() const {
		init_color(TEMP_COLOR_CUSTOM_MAGENTA, 1000, 0, 1000);
		init_pair(COLOR_CUSTOM_MAGENTA, TEMP_COLOR_CUSTOM_MAGENTA, COLOR_BLACK);
		init_color(TEMP_COLOR_CUSTOM_RED, 1000, 0, 0);
		init_pair(COLOR_CUSTOM_RED, TEMP_COLOR_CUSTOM_RED, COLOR_BLACK);
		init_color(TEMP_COLOR_CUSTOM_YELLOW, 1000, 1000, 0);
		init_pair(COLOR_CUSTOM_YELLOW, TEMP_COLOR_CUSTOM_YELLOW, COLOR_BLACK);
		init_color(TEMP_COLOR_CUSTOM_CYAN, 0, 1000, 1000);
		init_pair(COLOR_CUSTOM_CYAN, TEMP_COLOR_CUSTOM_CYAN, COLOR_BLACK);
		init_color(TEMP_COLOR_CUSTOM_GREEN, 0, 1000, 0);
		init_pair(COLOR_CUSTOM_GREEN, TEMP_COLOR_CUSTOM_GREEN, COLOR_BLACK);
	}

	void parseLine(const std::string& line, int lineNr);
	void parseSymbol(const std::string& symbol);
	std::unordered_map<std::string, int> parseHashmap(std::ifstream hashmap);

	inline bool haveHashmap(const std::string& extension) const {
		if (hashmaps.find(extension) == hashmaps.end()) {
			return true;
		}
		return false;
	}

	inline int getColorByKeyword(const std::string& keyword, const std::string& extension) const {
		if (haveHashmap(extension)) {
			// NOTE: For some reason hashmap[extension] doesn't work?
			if (can_change_color()) {
				return hashmaps.at(extension).at(keyword).first;
			} else {
				return hashmaps.at(extension).at(keyword).second;
			}
		} else {
			return COLOR_WHITE;
		}
	}

	// Proudly (?) stolen (and modified a bit) from stackoverflow: https://stackoverflow.com/questions/5888022/split-string-by-single-spaces
	std::vector<std::string> splitByTabsAndSpaces(const std::string &line) {
		size_t pos = line.find(" ");
		size_t pos2 = line.find("\t");
		size_t initialPos = 0;
		std::vector<std::string> finalVector;
		finalVector.clear();

		// Decompose statement
		while(pos != std::string::npos || pos2 != std::string::npos) {
			if (pos < pos2) {
				finalVector.push_back(line.substr(initialPos, pos - initialPos));
				finalVector.push_back(" ");
				initialPos = pos + 1;
				pos = line.find(" ", initialPos);
			} else {
				finalVector.push_back(line.substr(initialPos, pos2 - initialPos));
				finalVector.push_back("\t");
				initialPos = pos2 + 1;
				pos2 = line.find("\t", initialPos);
			}
		}

		// Add the last one
		if (pos > pos2) {
			finalVector.push_back(line.substr(initialPos,std::min(pos, line.size()) - initialPos + 1));
		} else {
			finalVector.push_back(line.substr(initialPos,std::min(pos2, line.size()) - initialPos + 1));
		}

		return finalVector;
	}
	
	private:
	// Hashmaps
	std::unordered_map<std::string, std::pair<int, int>> cppHashmap;
	std::unordered_map<std::string, std::pair<int, int>> cHashmap{
		{"preprocessor", std::pair(COLOR_CUSTOM_RED, COLOR_RED)},
		{"after_preprocessor", std::pair(COLOR_CUSTOM_MAGENTA, COLOR_YELLOW)},
		{"keyword", std::pair(COLOR_CUSTOM_CYAN, COLOR_CYAN)},
		{"__reserved_string_color__", std::pair(COLOR_CUSTOM_RED, COLOR_RED)},
		{"semicolon",  std::pair(COLOR_CUSTOM_YELLOW, COLOR_YELLOW)}
	};
	std::unordered_map<std::string, std::pair<int, int>> pythonHashmap;
	std::unordered_map<std::string, std::pair<int, int>> javaHashmap;
	std::unordered_map<std::string, std::pair<int, int>> haskellHashmap;
	std::unordered_map<std::string, std::pair<int, int>> htmlHashmap;
	std::unordered_map<std::string, std::pair<int, int>> jsHashmap;
	std::unordered_map<std::string, std::pair<int, int>> tsHashmap;
	std::unordered_map<std::string, std::pair<int, int>> cssHashmap;
	std::unordered_map<std::string, std::pair<int, int>> rubyHashmap;
	std::unordered_map<std::string, std::pair<int, int>> rustHashmap;
	std::unordered_map<std::string, std::pair<int, int>> perlHashmap;
	std::unordered_map<std::string, std::pair<int, int>> bashHashmap;
	std::unordered_map<std::string, std::pair<int, int>> csharpHashmap;

	// Language-specific symbols
	std::unordered_map<std::string, std::pair<bool, std::string>> cppFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> cFeatures{
		{"command_ending", std::pair(true, ";")},
		{"comment", std::pair(true, "//")},
		{"arrow_pointer", std::pair(true, "->")},
		{"multiline_comment", std::pair(true, "/*	*/")}
	};
	std::unordered_map<std::string, std::pair<bool, std::string>> pythonFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> javaFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> haskellFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> htmlFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> jsFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> tsFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> cssFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> rubyFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> rustFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> perlFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> bashFeatures;
	std::unordered_map<std::string, std::pair<bool, std::string>> csharpFeatures;

	std::unordered_map<std::string, std::unordered_map<std::string, std::pair<bool, std::string>>> featuremaps{
		{"cpp", cppFeatures},
		{"c", cFeatures},
		{"python", pythonFeatures},
		{"java", javaFeatures},
		{"haskell", haskellFeatures},
		{"html", htmlFeatures},
		{"js", jsFeatures},
		{"ts", tsFeatures},
		{"css", cssFeatures},
		{"ruby", rubyFeatures},
		{"rust", rustFeatures},
		{"perl", perlFeatures},
		{"bash", bashFeatures},
		{"csharp", csharpFeatures}
	};

	// Normal definitions
	std::unordered_map<std::string, std::unordered_map<std::string, std::pair<int, int>>> hashmaps{
		{"cpp", cppHashmap},
		{"c", cHashmap},
		{"python", pythonHashmap},
		{"java", javaHashmap},
		{"haskell", haskellHashmap},
		{"html", htmlHashmap},
		{"js", jsHashmap},
		{"ts", tsHashmap},
		{"css", cssHashmap},
		{"ruby", rubyHashmap},
		{"rust", rustHashmap},
		{"perl", perlHashmap},
		{"bash", bashHashmap},
		{"csharp", csharpHashmap}
	};
	std::string currentWord;

	// Boolean mess for syntax highlighting (sorry)
	bool isPreprocessorDirective{false};
	bool isAfterPreprocessor{false};
	bool isString{false};
	bool isMultilineComment{false};
	bool isComment{false};
	bool afterKeyword{false};
	//bool isSemicolon{false}; Idk why i included this check, i'll just leave it here, in case wi'll need it
	bool parenthesisFirstLayer{false};
	bool parenthesisSecondLayer{false};
	bool parenthesisThirdLayer{false};
	bool curlyFirstLayer{false};
	bool curlySecondLayer{false};
	bool curlyThirdLayer{false};
	bool squareFirstLayer{false};
	bool squareSecondLayer{false};
	bool squareThirdLayer{false};
};
#endif