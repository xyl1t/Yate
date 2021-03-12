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

	void save();
	void moveBeginningOfLine();
    void moveBeginningOfText();
	void moveEndOfLine();
    void newLine();
	void deleteCharL();
    void deleteCharR();
	
	void setColoredStatus(const std::string& message, int colorPair);

	void initColorPairs();
	void applyColorPairToStatusBar(const int& colorPair);
	
    inline bool isAlive() {
        return alive;
    }
    
    inline void saveFile() {
        file.save();
    }
    
private:
	int scrollX;
	int scrollY;
	int width;
	int height;
	
	FileEditor file;
    
    bool alive;

	// Status control variables:
	bool standard_status{true};
	std::string custom_message{""};

	// Color control variable:
	int colorPair{1};
};

#endif