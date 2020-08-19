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
	while(true)
	{
		std::string line{""};
		std::getline(file, line);
		if(!file) {
			break;
		}
		lines.push_back(line);
	}
}

void FileEditor::moveUp() {
	// if(carret.y > 0) {
		carret.y--;
	// 	if(lines[carret.y].size() < carret.maxX)
	// 		carret.x = lines[carret.y].size() - 1;
	// 	else
	// 		carret.x = carret.maxX;
	// }
}
void FileEditor::moveDown() {
	// if(carret.y < lines.size() - 1) {
		carret.y++;
	// 	if(lines[carret.y].size() < carret.maxX) 
	// 		carret.x = lines[carret.y].size() - 1;
	// 	else 
	// 		carret.x = carret.maxX;
	// }
}
void FileEditor::moveLeft() {
	// if(carret.x > 0) {
		carret.x--;
	// } 
    // else if(carret.y > 0) {
    //     carret.y--;
    //     carret.x = carret.maxX = lines[carret.y].size() - 1;
	// }
}
void FileEditor::moveRight() {
    // if(carret.x < lines[carret.y].size() - 1) {
		carret.x++;
	// } 
    // else if(carret.y < lines.size() - 1) {
    //     carret.y++;
    //     carret.x = 0;
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

// std::ostream& operator<<(const FileEditor& o);
// std::ostream& FileEditor::operator<<(const std::string& o) {
// 	textStream << o;
// 	return textStream;
// }
// std::ostream& operator<<(const char& o);
// std::ostream& operator<<(const long& o);
// std::ostream& operator<<(const double& o);