#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "fileEditor.hpp"
#include "syntaxHighlighter.hpp"
#include <string>

// Color pairs defines:
#define PAIR_STANDARD 1
#define PAIR_ERROR 2
#define PAIR_INFO 3
#define PAIR_OPEN_CLOSE_SYMBOL 4
#define PAIR_RESERVED_FOR_CUSTOM 5
// Basic color pairs for syntax highlighting:
#define PAIR_SYNTAX_RED 11
#define PAIR_SYNTAX_WHITE 12
#define PAIR_SYNTAX_CYAN 13
#define PAIR_SYNTAX_MAGENTA 14
#define PAIR_SYNTAX_YELLOW 15
#define PAIR_SYNTAX_GREEN 16
#define PAIR_SYNTAX_BLUE 17

class Editor {
public:
	Editor(const std::string& filePath);

	void draw();
	void getInput();
	
	void scrollUp();
	void scrollDown();
	void scrollLeft();
	void scrollRight();

	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();

	void saveFile();
	void moveBeginningOfLine();
	void moveEndOfLine();
    void moveBeginningOfText();
    void moveEndOfText();
    void newLine();
	void deleteCharL();
    void deleteCharR();
	
	void setStatus(const std::string& message);
	void setStatus(const std::string& message, int colorPair);
	void resetStatus();

	void initColorPairs() const;
	void applyColorPairToStatusBar(int colorPair);
	
    inline bool isAlive() const {
        return alive;
    }
    
private:
	FileEditor file;
	syntaxHighlighter syntaxHG{};
	
	int scrollX;
	int scrollY;
	int width;
	int height;
    
    bool alive;

	std::string statusText;

	// Color control variable:
	int colorPair{1};
};

#endif
