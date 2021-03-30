#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "fileEditor.hpp"
#include "action.hpp"
#include <string>
#include <ncurses.h>
#include <algorithm>
#include <functional>
#include <stack>
#include <memory>
#include <utility>

class Editor {
public:
	Editor(const std::string& filePath, int tabSize = 4);
	
	bool close();
	
	void draw();
	void drawStatus();
	int getInput();

	inline void setScrollH(int val) {
		int max = 0;
		for (int lineNr = scrollY; lineNr < scrollY + getTextEditorHeight() && lineNr < file.linesAmount(); lineNr++) {
			int val = getVirtualLineLength(lineNr);
			if(val > max) {
				max = val;
			}
		}
		scrollX = std::clamp(val, 0, max - 1);
	}
	inline void setScrollV(int val) {
		scrollY = std::clamp(val, 0, file.linesAmount() - 1);
	}
	inline void scrollH(int amount) { setScrollH(scrollX + amount); }
	inline void scrollV(int amount) { setScrollV(scrollY + amount); }
	inline void scrollUp(int amount = 1) { scrollV(-amount); }
	inline void scrollDown(int amount = 1) { scrollV(amount); }
	inline void scrollRight(int amount = 1) { scrollH(amount); }
	inline void scrollLeft(int amount = 1) { scrollH(-amount); }

	void put(int ch, bool record = true);
	void deleteCharL(bool record = true);
	void deleteCharR(bool record = true);
	void newLine();
		
	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();
	void moveBeginningOfLine();
	void moveEndOfLine();
    void moveBeginningOfText();
    void moveEndOfText();

	void find();
	void saveFile();
	
	void setStatus(const std::string& message);
	void setStatus(const std::string& message, int colorPair);
	void resetStatus();

	void initColorPairs() const;
	void applyColorPairToStatusBar(int colorPair);
	
    inline bool isAlive() const {
        return alive;
    }
	inline int getOnScreenCursorX() const {
		return getcurx(stdscr);
	}
	inline int getOnScreenCursorY() const {
		return getcury(stdscr);
	}
	inline int getTextEditorWidth() const {
		return getmaxx(stdscr) - 4;
	}
	inline int getTextEditorHeight() const {
		return getmaxy(stdscr) - 2;
	}
	inline void setCaretLocation(int x, int y) {
		caret.y = std::clamp(y, 0, (int)file.linesAmount() - 1);
		if(x == caret.x) {
			if (getVirtualLineLength() < caret.savedX) {
				caret.x = getVirtualLineLength();
				file.setCaretLocation(file.getLineSize(), caret.y);
			} else {
				file.setCaretLocation(getFileCaretColumn(caret.savedX), caret.y);
				caret.x = getVirtualCaretColumn(file.getCaretX(), caret.y);
			}
		} else {
			x = std::clamp(x, 0, (int)getVirtualLineLength(caret.y));
			file.setCaretLocation(getFileCaretColumn(x), caret.y);
			caret.x = caret.savedX = getVirtualCaretColumn(file.getCaretX(), file.getCaretY());
		}
		
		scrollToCaret();
	}
	inline void scrollToCaret() {
		if(caret.x < scrollX) {
			scrollLeft((scrollX) - (caret.x));
		}
		if(caret.x > getTextEditorWidth() - 1 + scrollX) {
			scrollRight((caret.x) - (getTextEditorWidth() - 1 + scrollX));
		}
		if(caret.y < scrollY) {
			scrollUp((scrollY) - (caret.y));
		}
		if(caret.y > getTextEditorHeight() - 1 + scrollY) {
			scrollDown((caret.y) - (getTextEditorHeight() - 1 + scrollY));
		}
	}
	
private:
	bool alive;
	FileEditor file;
	const int TAB_SIZE;
	
	Caret caret;
	int scrollX;
	int scrollY;
    
	std::string statusText;
	bool customStatusText{false};

	// Color control variable:
	int colorPair{1};
	
	std::stack<Action> undo;
	std::stack<Action> redo;
	
	int currentAction{};
	int prevAction{};
	
	std::string getInputInStatus(std::string statusText, int colorPair, const std::string& preset = "");
	
	inline char getCharAtCaret() {
		return file.getLine(caret.y)[getFileCaretColumn() - 1];
	}
	
	inline int getVirtualCaretColumnToCaret() {
		return getVirtualCaretColumnToCaret(file.getCaretY());
	}
	
	inline int getVirtualCaretColumnToCaret(int row) {
		int size{};
		const std::string& line = file.getLine(row);
		for (int col = 0; col < file.getCaretX(); col++) {
			if(line[col] != '\t') {
				size++;
			} else {
				size += TAB_SIZE - (size) % TAB_SIZE;
			}
		}
		return size;
	}
	
	inline int getVirtualLineLength() {
		return getVirtualLineLength(caret.y);
	}
	inline int getVirtualLineLength(int y) {
		return getVirtualCaretColumn(file.getLineSize(y), y);
	}	
	inline int getVirtualCaretColumn(int x, int y) {
		int size{};
		const std::string& line = file.getLine(y);
		for (int col = 0; col < x; col++) {
			if(line[col] != '\t') {
				size++;
			} else {
				size += TAB_SIZE - (size) % TAB_SIZE;
			}
		}
		return size;
	}
	
	inline int getFileCaretColumn() {
		return getFileCaretColumn(getVirtualCaretColumnToCaret());
	}
	inline int getFileCaretColumn(int virtualColumn) {
		int size{};
		const std::string& line = file.getLine(caret.y);
		for (int col = 0; col < virtualColumn;) {
			if(line[size] == '\t') {
				col += TAB_SIZE - (col) % TAB_SIZE;
			} else {
				col++;
			}
			if(col <= virtualColumn) {
				size++;
			}
		}
		return size;
	}
};

#endif
