#ifndef FILE_EDITOR_HPP
#define FILE_EDITOR_HPP

#include <sstream>
#include <fstream>
#include <string>
#include <vector>

struct Carret {
	int x;
	int y;
	int maxX;
};

class FileEditor {
public: 
	FileEditor(const std::string& path);
	
	inline void setCarretLocation(size_t x, size_t y) {
		carret.y = (y > lines.size()) ? lines.size() - 1 : y;
		carret.x = carret.maxX = (x > lines[y].size()) ? lines[y].size() : x;
	}
	
	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();
	
	inline void put(const char& ch) {
		lines[carret.y].insert(carret.x, 1, ch);
		moveRight();
	}
	inline void put(const std::string& str) {
		lines[carret.y].insert(carret.x, str);
		setCarretLocation(carret.x + str.size(), carret.y);
	}
	void del(bool right);
	
	void newLine();
	
	inline const std::string& getLine() const {
		return lines[carret.y];
	}
	inline const std::string& getLine(size_t lineNr) const {
		return lines[lineNr];
	}
	inline int getLineSize() const {
		return lines[carret.y].size();
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
	
	inline int getCarretX() const {
		return carret.x;
	}
	inline int getCarretY() const {
		return carret.y;
	}
	
	void save();
	void close();
	
	inline bool hasWritePermission() const {
		return writePermission;
	}

	
private:
	Carret carret;
	std::string path;
	std::string fullFilename;
	std::string filename;
	std::string extension;
	std::vector<std::string> lines;
	bool writePermission{true};
};

#endif