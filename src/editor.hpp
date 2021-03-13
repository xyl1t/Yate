#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "fileEditor.hpp"
#include <string>

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
	
	void setColoredStatus(const std::string& message, int colorPair);

	void initColorPairs() const;
	void applyColorPairToStatusBar(int colorPair);
	
    inline bool isAlive() const {
        return alive;
    }
    
private:
	FileEditor file;
	
	int scrollX;
	int scrollY;
	int width;
	int height;
    
    bool alive;

	// Status control variables:
	bool standard_status{true};
	std::string custom_message{""};

	// Color control variable:
	int colorPair{1};
};

#endif
