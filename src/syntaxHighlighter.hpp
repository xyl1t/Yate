#ifndef SYNTAX_HIGHLITHER_HPP
#define SYNTAX_HIGHLITHER_HPP
#include <string>
#include <sstream>
#include <unordered_map>
#include <ncurses.h>
#include <vector>
#include "editor.hpp"

// Color pairs defines:
#define PAIR_STANDARD 1
#define PAIR_ERROR 2
#define PAIR_WARNING 2
#define PAIR_INFO 3
#define PAIR_OPEN_CLOSE_SYMBOL 4
// Basic color pairs for syntax highlighting:
#define PAIR_SYNTAX_RED 11
#define PAIR_SYNTAX_WHITE 12
#define PAIR_SYNTAX_CYAN 13
#define PAIR_SYNTAX_MAGENTA 14
#define PAIR_SYNTAX_YELLOW 15
#define PAIR_SYNTAX_GREEN 16
#define PAIR_SYNTAX_BLUE 17

class Editor;

using featuremap = std::unordered_map<std::string, std::pair<bool, std::string>>;
using colormap = std::unordered_map<std::string, std::pair<int, int>>;
using keywordmap = std::vector<std::string>;
using colorpair = std::vector<std::pair<int, char>>;

class syntaxHighlighter {
	public:

	syntaxHighlighter();

	colormap createColormap(std::vector<std::string> names, std::vector<int> colors, std::vector<int> backup);

	featuremap createFeaturemap(std::vector<bool> status, std::vector<std::string> symbols);

	void registerMaps(std::string extension, keywordmap keywords, featuremap featuremap, colormap colormap);

	inline colorpair createColopairFromAttributes(const std::string& symbol, int custom_pair, int pair) {
		colorpair temp = colorpair();
		if (can_change_color() && COLORS > 8) {
			for (int i = 0; i < static_cast<int>(symbol.size()); i++ ) {
				temp.push_back({custom_pair, symbol[i]});
			}
		} else {
			for (int i = 0; i < static_cast<int>(symbol.size()); i++ ) {
				temp.push_back({pair, symbol[i]});
			}
		}
		return temp;
	}

	// Another stolen piece of code: https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
	inline void replaceStringInPlace(std::string& string, const std::string& substring, const std::string& replace) {
		size_t pos = 0;
		while ((pos = string.find(substring, pos)) != std::string::npos) {
			string.replace(pos, substring.length(), replace);
			pos += replace.length();
		}
	}

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

	inline bool isKeyword(const std::string& symbol, const std::string& extension) {
		std::string new_symbol = std::string(symbol);
		if (keywordmaps.find(extension) != keywordmaps.end()) {
			keywordmap newKeywordmap = keywordmaps.at(extension);
			for (int i = 0; i < static_cast<int>(newKeywordmap.size()); i++ ) {
				if ( new_symbol == newKeywordmap[i] ) return true;
			}
		}
		return false;
	}

	inline bool hasFeaturemap(const std::string& extension) const {
		if (featuremaps.find(extension) != featuremaps.end()) {
			return true;
		}
		return false;
	}

	// Init function for more colors, not used right now
	inline void initMoreColors() const {
		if(COLORS > 8 && can_change_color()) {
			//Define your colors here
		}
	}

	// Parses a line and returns a pair
	std::vector<colorpair> parseLine(const std::string& line, int lineNr, const std::string& extension, Editor editor);
	// Parses a symbol and returns a pair
	colorpair parseSymbol(const std::string& symbol, const std::string& extension, Editor editor);
	// Parses a colormap and stores it
	colormap parseColormap(std::ifstream colormap);

	inline bool hasColormap(const std::string& extension) const {
		if (colormaps.find(extension) != colormaps.end()) {
			return true;
		}
		return false;
	}

	inline int getColorByKeyword(const std::string& keyword, const std::string& extension) const {
		if (hasColormap(extension)) {
			return colormaps.at(extension).at(keyword).first;
		} else {
			return COLOR_WHITE;
		}
	}

	inline int getBackupColorByKeyword(const std::string& keyword, const std::string& extension) const {
		if (hasColormap(extension)) {
			return colormaps.at(extension).at(keyword).second;
		} else {
			return COLOR_WHITE;
		}
	}

	// Proudly (?) stolen (and modified a bit) from stackoverflow: https://stackoverflow.com/questions/5888022/split-string-by-single-spaces
	std::vector<std::string> splitBySpecialChars(const std::string& line, const std::string& extension);
	
	void registerDefaults();

	private:
	const keywordmap javaKeywords{
		""
	};

	const keywordmap haskellKeywords{
		""
	};

	const keywordmap htmlKeywords{
		""
	};

	const keywordmap jsKeywords{
		""
	};

	const keywordmap tsKeywords{
		""
	};

	const keywordmap cssKeywords{
		""
	};

	const keywordmap rubyKeywords{
		""
	};

	const keywordmap rustKeywords{
		""
	};

	const keywordmap perlKeywords{
		""
	};

	const keywordmap bashKeywords{
		""
	};

	const keywordmap csharpKeywords{
		""
	};

	std::unordered_map<std::string, keywordmap> keywordmaps{
		{"java", javaKeywords},
		{"haskell", haskellKeywords},
		{"html", htmlKeywords},
		{"js", jsKeywords},
		{"ts", tsKeywords},
		{"css", cssKeywords},
		{"ruby", rubyKeywords},
		{"rust", rustKeywords},
		{"perl", perlKeywords},
		{"bash", bashKeywords},
		{"csharp", csharpKeywords}
	};
	const std::unordered_map<std::string, std::pair<int, int>> javaColormap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> haskellColormap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> htmlColormap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> jsColormap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> tsColormap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> cssColormap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> rubyColormap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> rustColormap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> perlColormap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> bashColormap{
		{}
	};
	const std::unordered_map<std::string, std::pair<int, int>> csharpColormap{
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

	std::unordered_map<std::string, featuremap> featuremaps{
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

	std::unordered_map<std::string, colormap> colormaps{
		{"java", javaColormap},
		{"haskell", haskellColormap},
		{"html", htmlColormap},
		{"js", jsColormap},
		{"ts", tsColormap},
		{"css", cssColormap},
		{"ruby", rubyColormap},
		{"rust", rustColormap},
		{"perl", perlColormap},
		{"bash", bashColormap},
		{"csharp", csharpColormap}
	};
	std::string nextSymbol{""};
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