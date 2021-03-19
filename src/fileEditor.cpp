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
	: carret{}, 
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
	
	#if defined(__linux__) || defined(__APPLE__)
	struct stat file_stat;
	stat(path.c_str(), &file_stat);
	uid_t current_uid = getuid();
	gid_t current_gid = getgid();
	fs::perms active_perms = fs::status(path).permissions();
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

	// TODO: File checking is applying only to linux systems, need to implement this on mac os and windows.
}

void FileEditor::moveUp() {
	if(carret.y - 1 >= 0) {
		carret.y -= 1;
		if(getLineSize(carret.y) < carret.maxX) {
			carret.x = getLineSize(carret.y);
		}
		else
			carret.x = carret.maxX;
	}
	else if(carret.y - 1 < 0) {
		carret.y = 0;
		carret.x = carret.maxX = 0;
	}
}
void FileEditor::moveDown() {
	if(carret.y + 1 < linesAmount()) {
		carret.y += 1;
		if(getLineSize(carret.y) < carret.maxX) 
			carret.x = getLineSize(carret.y);
		else 
			carret.x = carret.maxX;
	}
	else if(carret.y + 1 >= linesAmount() - 1) {
		carret.y = linesAmount() - 1;
		carret.x = carret.maxX = lines[linesAmount() - 1].size();
	}
}
void FileEditor::moveLeft() {
	if(carret.x > 0) {
		carret.x = carret.maxX = carret.x - 1;
	} 
	else if(carret.y > 0) {
		carret.y--;
		carret.x = carret.maxX = getLineSize(carret.y);
	}
}
void FileEditor::moveRight() {
	if(carret.x < getLineSize(carret.y)) {
		carret.x = carret.maxX = carret.x + 1;
	} 
	else if(carret.y < linesAmount() - 1) {
		carret.y++;
		carret.x = carret.maxX = 0;
	}
}

void FileEditor::newLine() {
	std::string& current = lines[carret.y];
	std::string rest = current.substr(carret.x, getLineSize(carret.y));
	current.erase(carret.x, getLineSize(carret.y)); 
	lines.insert(lines.begin() + carret.y + 1, rest);
}
void FileEditor::del(bool right) {
	// Anti-bug spray:
	if ((carret.y == 0 && carret.x == 0) && linesAmount() <= 1 && getLineSize(carret.y) <= 1) {
		throw std::string(" No char to delete. ");
	}
	if (!(right) && carret.x == 0 && carret.y == 0) {
		throw std::string(" No char to delete. ");
	}
	if (right && carret.x == getLineSize() && carret.y == linesAmount() - 1) {
		throw std::string(" No char to delete. ");
	}

	if(right) {
		if(carret.x == getLineSize(carret.y)) {
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
		if (carret.x == 0 && carret.y == 0 && linesAmount() <= 1) {
			throw std::string(" No char to delete. ");
		}
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
