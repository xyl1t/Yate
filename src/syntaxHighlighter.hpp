#ifndef SYNTAX_HIGHLITHER_HPP
#define SYNTAX_HIGHLITHER_HPP
#include <string>
#include <sstream>
#include <unordered_map>
#include <ncurses.h>
#include "editor.hpp"

#define PAIR_SYNTAX_RED 11
#define PAIR_SYNTAX_WHITE 12
#define PAIR_SYNTAX_CYAN 13
#define PAIR_SYNTAX_MAGENTA 14
#define PAIR_SYNTAX_YELLOW 15
#define PAIR_SYNTAX_GREEN 16
#define PAIR_SYNTAX_BLUE 17

class syntaxHighlighter {
	public:

	inline void resetAllFlags() {
		isPreprocessor = false;
		afterPreprocessor = false;
		isString = false;
		isMultilineComment = false;
		isComment = false;
		afterArrowPointer = false;
	}

	inline void resetLineSpecificFlags() {
		isComment = false;
		afterArrowPointer = false;
		afterPreprocessor = false;
	}

	inline bool hasFeaturemap(const std::string& extension) const {
		if (featuremaps.find(extension) != featuremaps.end()) {
			return true;
		}
		return false;
	}

	/********************************
	*	FEATUREMAPS FUNCTION ACCESS	*
	*********************************/

	inline std::string supportCommandEnding(const std::string& extension) const {
		if (hasFeaturemap(extension)) {
			if (featuremaps.at(extension).at("command_ending").first) return featuremaps.at(extension).at("command_ending").second;
		}
		return "";
	}

	inline std::string supportArrowPointer(const std::string& extension) const {
		if (hasFeaturemap(extension)) {
			if (featuremaps.at(extension).at("arrow_pointer").first) return featuremaps.at(extension).at("arrow_pointer").second;
		}
		return "";
	}

	inline std::string supportComment(const std::string& extension) const {
		if (hasFeaturemap(extension)) {
			if (featuremaps.at(extension).at("comment").first) return featuremaps.at(extension).at("comment").second;
		}
		return "";
	}

	inline std::string supportMultilineComment(const std::string& extension) const {
		if (hasFeaturemap(extension)) {
			if (featuremaps.at(extension).at("multiline_comment").first) return featuremaps.at(extension).at("multiline_comment").second;
		}
		return "";
	}


	// Init function for more colors, not used right now
	inline void initMoreColors() const {
		if(COLORS > 8 && can_change_color()) {
			//Define your colors here
		}
	}

	// Parses a line and prints it out
	void parseLine(const std::string& line, int lineNr, const std::string& extension);
	// Parses a symbol and prints it out
	void parseSymbol(const std::string& symbol, const std::string& extension);
	// Parses a hashmap and stores it
	std::unordered_map<std::string, int> parseHashmap(std::ifstream hashmap);

	inline bool hasHashmap(const std::string& extension) const {
		if (hashmaps.find(extension) != hashmaps.end()) {
			return true;
		}
		return false;
	}

	inline int getColorByKeyword(const std::string& keyword, const std::string& extension) const {
		if (hasHashmap(extension)) {
			// NOTE: For some reason hashmap[extension] doesn't work?
			return hashmaps.at(extension).at(keyword).first;
		} else {
			return COLOR_WHITE;
		}
	}

	inline int getBackupColorByKeyword(const std::string& keyword, const std::string& extension) const {
		if (hasHashmap(extension)) {
			// NOTE: For some reason hashmap[extension] doesn't work?
			return hashmaps.at(extension).at(keyword).second;
		} else {
			return COLOR_WHITE;
		}
	}

	// Proudly (?) stolen (and modified a bit) from stackoverflow: https://stackoverflow.com/questions/5888022/split-string-by-single-spaces
	std::vector<std::string> splitBySpecialChars(const std::string& line) const {
		std::vector<std::pair<size_t, std::string>> positions;
		positions.push_back(std::pair<size_t, std::string>(line.find(" "), " "));
		positions.push_back(std::pair<size_t, std::string>(line.find("\t"), "\t"));
		positions.push_back(std::pair<size_t, std::string>(line.find(";"), ";"));
		positions.push_back(std::pair<size_t, std::string>(line.find("("), "("));
		positions.push_back(std::pair<size_t, std::string>(line.find("["), "["));
		positions.push_back(std::pair<size_t, std::string>(line.find("{"), "{"));
		positions.push_back(std::pair<size_t, std::string>(line.find(")"), ")"));
		positions.push_back(std::pair<size_t, std::string>(line.find("]"), "]"));
		positions.push_back(std::pair<size_t, std::string>(line.find("}"), "}"));
		positions.push_back(std::pair<size_t, std::string>(line.find("->"), "->"));
		positions.push_back(std::pair<size_t, std::string>(line.find("\""), "\""));
		size_t initialPos = 0;
		std::vector<std::string> finalVector;
		finalVector.clear();

		// Decompose statement (Became very messy, TODO: fix)
		while(positions[0].first != std::string::npos || positions[1].first != std::string::npos || positions[2].first != std::string::npos || positions[3].first != std::string::npos || positions[4].first != std::string::npos || positions[5].first != std::string::npos || positions[6].first != std::string::npos || positions[7].first != std::string::npos || positions[8].first != std::string::npos || positions[9].first != std::string::npos || positions[10].first != std::string::npos) {
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
			if (min == 9) {
				initialPos = positions[min].first + 2;
			} else {
				initialPos = positions[min].first + 1;
			}
			positions[min].first = line.find(positions[min].second, initialPos);
		}

		// Add the last one
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

		return finalVector;
	}
	
	private:

	/****************************
	*	STANDARD	HASHMAPS	*
	*****************************/
	
	const std::unordered_map<std::string, std::pair<int, int>> cppHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> cHashmap{
		{"preprocessor", std::pair<int, int>(PAIR_SYNTAX_RED, PAIR_SYNTAX_RED)},
		{"after_preprocessor", std::pair<int, int>(PAIR_SYNTAX_YELLOW, PAIR_SYNTAX_YELLOW)},
		{"keyword", std::pair<int, int>(PAIR_SYNTAX_CYAN, PAIR_SYNTAX_CYAN)},
		{"__reserved_string_color__", std::pair<int, int>(PAIR_SYNTAX_RED, PAIR_SYNTAX_RED)},
		{"semicolon",  std::pair<int, int>(PAIR_SYNTAX_YELLOW, PAIR_SYNTAX_YELLOW)}
	};
	const std::unordered_map<std::string, std::pair<int, int>> pythonHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> javaHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> haskellHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> htmlHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> jsHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> tsHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> cssHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> rubyHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> rustHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> perlHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> bashHashmap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> csharpHashmap{
		{}
	};

	/****************************
	*	STANDARD   FEATUREMAPS	*
	*****************************/

	// Language-specific symbols
	const std::unordered_map<std::string, std::pair<bool, std::string>> cppFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> cFeatures{
		{"command_ending", std::pair(true, ";")},
		{"comment", std::pair(true, "//")},
		{"arrow_pointer", std::pair(true, "->")},
		{"multiline_comment", std::pair(true, "/*	*/")}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> pythonFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> javaFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> haskellFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> htmlFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> jsFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> tsFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> cssFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> rubyFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> rustFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> perlFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> bashFeatures{
		{}
	};
	const std::unordered_map<std::string, std::pair<bool, std::string>> csharpFeatures{
		{}
	};

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
	bool isPreprocessor{false};
	bool afterPreprocessor{false};
	bool isString{false};
	bool isMultilineComment{false};
	bool isComment{false};
	bool afterArrowPointer{false};
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