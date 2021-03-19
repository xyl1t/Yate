#include "fileEditor.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <bitset>
#include <filesystem>
namespace  fs = std::filesystem;
#include <ncurses.h>

// If windows, include windows API.
#ifdef _WIN32
#include "Windows.h"
#endif
// If linux, include linux's system library.
#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/stat.h>
#endif

FileEditor::FileEditor(const std::string& path) 
	: caret{}, 
	path{path},
	fullFilename {},
	filename {},
	extension {},
	lines{},
	writePermission{true} {
	if (path != "") {
		fs::path temp = fs::path{path};
		std::string path;
		path = fs::absolute(temp).string();
		this->path = path;
		for (int i = path.size() - 1; i >= 0; i--) {
			if(path[i] == '/' || path[i] == '\\') break;
			fullFilename += path[i];
		}
		std::reverse(fullFilename.begin(), fullFilename.end());
	} else {
		endwin();
		std::cout << ("No file was supplied.\n");
		exit(1);
	}

	struct stat file_stat;
	stat(path.c_str(), &file_stat);
	uid_t current_uid = getuid();
	gid_t current_gid = getgid();
	fs::perms active_perms = fs::status(path).permissions();
	
	#if defined(__linux__) || defined(__APPLE__)
	if (!(((active_perms & fs::perms::owner_read)  != fs::perms::none && file_stat.st_uid == current_uid) ||
		  ((active_perms & fs::perms::group_read)  != fs::perms::none && file_stat.st_gid == current_gid) ||
		  ((active_perms & fs::perms::others_read) != fs::perms::none))) {
		endwin();
		std::cout << "Can't edit " << fullFilename << " not enough permissions.\n";
		exit(1);
	}
	this->writePermission = ((active_perms & fs::perms::owner_write)  != fs::perms::none && file_stat.st_uid == current_uid) ||
							((active_perms & fs::perms::group_write)  != fs::perms::none && file_stat.st_gid == current_gid) ||
							((active_perms & fs::perms::others_write) != fs::perms::none);
	#elif
	if (!(((active_perms & fs::perms::owner_read)  != fs::perms::none) ||
		  ((active_perms & fs::perms::group_read)  != fs::perms::none) ||
		  ((active_perms & fs::perms::others_read) != fs::perms::none))) {
		endwin();
		std::cout << "Can't edit " << fullFilename << " not enough permissions.\n";
		exit(1);
	}
	this->writePermission = ((active_perms & fs::perms::owner_write)  != fs::perms::none) ||
							((active_perms & fs::perms::group_write)  != fs::perms::none) ||
							((active_perms & fs::perms::others_write) != fs::perms::none);
	#endif
	
	filename = fullFilename.substr(0, fullFilename.find('.'));
	extension = fullFilename.substr(filename.size());
	std::ifstream file {path};
	if (!file) {
		endwin();
		std::cout << "Error occured while trying to open " << fullFilename << ".\n";
#ifndef NDEBUG
		std::cerr << "Error bits are: "
			<< "\nfailbit: " << file.fail() 
			<< "\neofbit: " << file.eof()
			<< "\nbadbit: " << file.bad() << std::endl;  
#endif
		exit(1);
	}
	while(file)
	{
		std::string line{""};
		std::getline(file, line);
		if(!file) break;
		lines.push_back(line);
	}
}

#if 0
void FileEditor::moveUp() {
	if(caret.y - 1 >= 0) {
		caret.y -= 1;
		if(getLineSize(caret.y) < caret.maxX) {
			caret.x = getLineSize(caret.y);
		}
		else
			caret.x = caret.maxX;
	}
	else if(caret.y - 1 < 0) {
		caret.y = 0;
		caret.x = caret.maxX = 0;
	}
}
void FileEditor::moveDown() {
	if(caret.y + 1 < linesAmount()) {
		caret.y += 1;
		if(getLineSize(caret.y) < caret.maxX) 
			caret.x = getLineSize(caret.y);
		else 
			caret.x = caret.maxX;
	}
	else if(caret.y + 1 >= linesAmount() - 1) {
		caret.y = linesAmount() - 1;
		caret.x = caret.maxX = lines[linesAmount() - 1].size();
	}
}
void FileEditor::moveLeft() {
	if(caret.x > 0) {
		caret.x = caret.maxX = caret.x - 1;
	} 
	else if(caret.y > 0) {
		moveUp();
		caret.x = caret.maxX = getLineSize(caret.y);
	}
}
void FileEditor::moveRight() {
	if(caret.x < getLineSize(caret.y)) {
		caret.x = caret.maxX = caret.x + 1;
	} 
	else if(caret.y < linesAmount() - 1) {
		moveDown();
		caret.x = caret.maxX = 0;
	}
}
#endif
void FileEditor::newLine() {
	std::string& current = lines[caret.y];
	std::string rest = current.substr(caret.x, getLineSize(caret.y));
	current.erase(caret.x, getLineSize(caret.y)); 
	lines.insert(lines.begin() + caret.y + 1, rest);
}
void FileEditor::del(bool right) {
	// Anti-bug spray:
	if ((caret.y == 0 && caret.x == 0) && linesAmount() <= 1 && getLineSize(caret.y) <= 1) {
		throw std::string(" No char to delete. ");
	}
	if (!(right) && caret.x == 0 && caret.y == 0) {
		throw std::string(" No char to delete. ");
	}
	if (right && caret.x == getLineSize() && caret.y == linesAmount() - 1) {
		throw std::string(" No char to delete. ");
	}

	if(right) {
		if(caret.x == getLineSize(caret.y)) {
			int lineNr = caret.y;
			std::string line = lines[lineNr + 1];
			lines.erase(lines.begin() + lineNr + 1);
			lines[lineNr].append(line);
		}
		else {
			lines[caret.y].erase(lines[caret.y].begin() + (caret.x));
		}
	}
	else {
		if (caret.x == 0 && caret.y == 0 && linesAmount() <= 1) {
			throw std::string(" No char to delete. ");
		}
		if(caret.x == 0) {
			int lineNr = caret.y;
			moveLeft();
			std::string line = lines[lineNr];
			lines.erase(lines.begin() + lineNr);
			lines[lineNr - 1].append(line);
		}
		else {
			lines[caret.y].erase(lines[caret.y].begin() + (caret.x - 1));
			moveLeft();
		}
	}
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
