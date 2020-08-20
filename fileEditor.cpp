#include "fileEditor.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>

FileEditor::FileEditor(const std::string& path) 
	: carret{}, 
	path{path}, 
	lines{},
	fullFilename {},
	filename {},
	extension {} {
	for (int i = path.size() - 1; i >= 0; i--) {
		fullFilename += path[i];
		if(path[i + 1] == '/' || path[i + 1] == '\\') break;
	}
	std::reverse(fullFilename.begin(), fullFilename.end());
	filename = fullFilename.substr(0, fullFilename.find('.'));
	extension = fullFilename.substr(filename.size());
	std::ifstream file {path};
	while(file)
	{
		std::string line{""};
		std::getline(file, line);
		lines.push_back(line);
	}
}

void FileEditor::moveUp() {
	if(carret.y - 1 >= 0) {
		carret.y -= 1;
		if(lines[carret.y].size() < carret.maxX)
			carret.x = lines[carret.y].size();
		else
			carret.x = carret.maxX;
	}
	else if(carret.y - 1 < 0) {
		carret.y = 0;
		carret.x = carret.maxX = 0;
	}
}
void FileEditor::moveDown() {
	if(carret.y + 1 < lines.size()) {
		carret.y += 1;
		if(lines[carret.y].size() < carret.maxX) 
			carret.x = lines[carret.y].size();
		else 
			carret.x = carret.maxX;
	}
	else if(carret.y + 1 >= lines.size() - 1) {
		carret.y = lines.size() - 1;
		carret.x = carret.maxX = lines[lines.size() - 1].size();
	}
}
void FileEditor::moveLeft() {
	if(carret.x > 0) {
		carret.x = carret.maxX = carret.x - 1;
	} 
	else if(carret.y > 0) {
		carret.y--;
		carret.x = carret.maxX = lines[carret.y].size();
	}
}
void FileEditor::moveRight() {
	if(carret.x < lines[carret.y].size()) {
		carret.x = carret.maxX = carret.x + 1;
	} 
	else if(carret.y < lines.size() - 1) {
		carret.y++;
		carret.x = carret.maxX = 0;
	}
}

void FileEditor::newLine() {
	std::string& current = lines[carret.y];
	std::string rest = current.substr(carret.x, lines[carret.y].size());
	current.erase(carret.x, lines[carret.y].size()); 
	lines.insert(lines.begin() + carret.y + 1, rest);
}
void FileEditor::del(bool right) {
	if(right) {
		if(carret.x == lines[carret.y].size()) {
			int lineNr = carret.y;
			std::string line = lines[lineNr + 1];
			lines.erase(lines.begin() + lineNr + 1);
			lines[lineNr].append(line);
		}
		else {
			lines[carret.y].erase(lines[carret.y].begin() + (carret.x));
		}
	}
	else {
		if(carret.x == 0) {
			int lineNr = carret.y;
			moveLeft();
			std::string line = lines[lineNr];
			lines.erase(lines.begin() + lineNr);
			lines[lineNr - 1].append(line);
		}
		else {
			lines[carret.y].erase(lines[carret.y].begin() + (carret.x - 1));
			moveLeft();
		}
	}
	// if(carret.x == 0) {
	// 	int lineNr = carret.y;
	// 	std::string line;
	// 	moveLeft();
	// 	if(lines[lineNr].size() == 0) {
	// 		std::string line = lines[lineNr];
	// 		lines.erase(lines.begin() + lineNr);
	// 		lines[lineNr - 1].append(line);
	// 	}
	// }
}

void FileEditor::save() {
	std::ofstream file { path };
	for(const auto& line : lines) {
		file << line << "\n";
	}
}
void FileEditor::close() {
	lines.clear();
	lines.shrink_to_fit();
}

/*

void FileEditor::moveUp(int n) {
	if(carret.y - n >= 0) {
		carret.y -= n;
		if(lines[carret.y].size() < carret.maxX)
			carret.x = lines[carret.y].size();
		else
			carret.x = carret.maxX;
	}
	else if(carret.y - n < 0) {
		carret.y = 0;
		carret.x = carret.maxX = 0;
	}
}
void FileEditor::moveDown(int n) {
	if(carret.y + n < lines.size() - 1 + n) {
		carret.y += n;
		if(lines[carret.y].size() < carret.maxX) 
			carret.x = lines[carret.y].size();
		else 
			carret.x = carret.maxX;
	}
	else if(carret.y + n >= lines.size() - 1) {
		carret.y = lines.size() - 1;
		carret.x = carret.maxX = lines[lines.size() - 1].size();
	}
}
void FileEditor::moveLeft(int n) {
	for(int i = 0; i < n; i++) {
		if(carret.x > 0) {
			carret.x = carret.maxX = carret.x - 1;
		} 
		else if(carret.y > 0) {
			carret.y--;
			carret.x = carret.maxX = lines[carret.y].size();
		}
	}
}
void FileEditor::moveRight(int n) {
	for(int i = 0; i < n; i++) {
		if(carret.x < lines[carret.y].size()) {
			carret.x = carret.maxX = carret.x + 1;
		} 
		else if(carret.y < lines.size() - 1) {
			carret.y++;
			carret.x = carret.maxX = 0;
		}
	}
}
*/

// std::ostream& operator<<(const FileEditor& o);
// std::ostream& FileEditor::operator<<(const std::string& o) {
// 	textStream << o;
// 	return textStream;
// }
// std::ostream& operator<<(const char& o);
// std::ostream& operator<<(const long& o);
// std::ostream& operator<<(const double& o);