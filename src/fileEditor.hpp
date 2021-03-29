#ifndef FILE_EDITOR_HPP
#define FILE_EDITOR_HPP

#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "caret.hpp"

class FileEditor {
public: 
	FileEditor(const std::string& path);
	
	inline void setCaretLocation(int x, int y) {
		caret.y = std::clamp(y, 0, (int)lines.size() - 1);
		caret.x = std::clamp(x, 0, (int)lines[caret.y].size());
	}
	inline void moveCaret(int x, int y) {
		setCaretLocation(caret.x + x, caret.y + y);
	}
	
	inline void moveUp() {
		moveCaret(0, -1);
	}
	inline void moveDown() {
		moveCaret(0, 1);
	}
	inline void moveLeft(){ 
		moveCaret(-1, 0);
	}
	inline void moveRight(){ 
		moveCaret(1, 0);
	}
	
	inline void put(char ch) {
		lines[caret.y].insert(caret.x, 1, ch);
	}
	inline void put(const std::string& str) {
		lines[caret.y].insert(caret.x, str);
		setCaretLocation(caret.x + str.size(), caret.y);
	}
	void del(bool right);
	
	void newLine();
	
	bool hasFileContentChanged() {
		if(!hasWritePermission()) {
			return false;
		}
		if(path == "") {
			if (lines.size() > 1 || !lines[0].empty()) return true; 
			else return false;
		} 
		
		std::fstream file {path};
		size_t row = 0;
		while(file) {
			std::string line{""};
			std::getline(file, line);
			if(!file) break;
			if(line.length() != lines[row].length()) return true;
			for(size_t i = 0; i < line.length(); i++) {
				if(line[i] != lines[row][i]) return true;
			}
			row++;
			if(row > lines.size()) return true;
		}
		if(row != lines.size()) return true;

		return false;
	}
	
	inline const std::string& getLine() const {
		return lines[caret.y];
	}
	inline const std::string& getLine(size_t lineNr) const {
		return lines[lineNr];
	}
	inline int getLineSize() const {
		return lines[caret.y].size();
	}
	inline int getLineSize(size_t lineNr) const {
		return lines[lineNr].size();
	}
	inline const std::string& getPath() const {
		return path;
	}
	inline const std::string& getFullFilename() const {
		return fullFilename;
	}
	inline const std::string& getFilename() const {
		return filename;
	}
	inline const std::string& getFileExtension() const {
		return extension;
	}
	inline int linesAmount() const {
		return lines.size();
	}
	
	inline int getCaretX() const {
		return caret.x;
	}
	inline int getCaretY() const {
		return caret.y;
	}
	inline const Caret& getCarret() const {
		return this->caret;
	}
	inline const std::string& getInfoMessage() const {
		return infoMessage;
	}
	
	void save();
	void saveAs(const std::string& path);
	void close();
	
	inline bool hasWritePermission() const {
		return writePermission;
	}

	
private:
	Caret caret;
	std::string path;
	std::string fullFilename;
	std::string filename;
	std::string extension;
	std::vector<std::string> lines;
	bool writePermission;
	std::string infoMessage;
	
	void setPath(const std::string& _path);
};

#endif
