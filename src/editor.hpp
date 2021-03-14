#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "fileEditor.hpp"
#include "syntaxHighlighter.hpp"
#include <string>

// Color pairs defines:
#define PAIR_STANDARD 1
#define PAIR_ERROR 2
#define PAIR_WARNING 3
#define PAIR_INFO 4
#define PAIR_OPEN_CLOSE_SYMBOL 5

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
