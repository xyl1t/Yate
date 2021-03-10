#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "fileEditor.hpp"
#include <string>
#include <unordered_map>

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

	void moveBeginningOfLine();
    void moveBeginningOfText();
	void moveEndOfLine();
    void newLine();
	void deleteCharL();
    void deleteCharR();
	
	void addSimpleStatus(const std::string& name, const std::string& status_message);
	void enableSimpleStatus(const std::string& name);

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

	std::unordered_map<std::string, std::string> statuses;

	// Status control variables:
	bool standard_status{true};
	std::string name_of_status{""};

	// Color control variable:
	int colorPair{1};
};

#endif