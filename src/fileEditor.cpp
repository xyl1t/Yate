#include "fileEditor.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <bitset>
#include <filesystem>
namespace  fs = std::filesystem;

#ifdef YATE_WINDOWS
#include "pdcurses.h"
#include "Windows.h"
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <ncurses.h>
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
		setPath(path);

		fs::perms active_perms = fs::status(path).permissions();
		
#if defined(__linux__) || defined(__APPLE__)
		struct stat file_stat;
		stat(path.c_str(), &file_stat);
	
		uid_t current_uid = getuid();
		gid_t current_gid = getgid();

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
#else
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
		
		std::ifstream file {path};
		if (!file.good() && file.bad()) {
			endwin();
			std::cout << "Error occured while trying to open " << fullFilename << ".\n";
#ifndef NDEBUG
			std::cerr << "Error bits are: "
				<< "\nfailbit: " << file.fail() 
				<< "\neofbit: " << file.eof()
				<< "\nbadbit: " << file.bad() << std::endl;  
#endif
			exit(1);
		} else if(!file.good()) {
			writePermission = true;
			lines.push_back("");
		}
		else {
			while(file)
			{
				std::string line{""};
				std::getline(file, line);
				if(!file) break;
				lines.push_back(line);
			}
		}
	} else {
		writePermission = true;
		lines.push_back("");
	}
}

void FileEditor::newLine() {
	std::string& current = lines[caret.y];
	std::string rest = current.substr(caret.x, getLineSize(caret.y));
	current.erase(caret.x, getLineSize(caret.y)); 
	lines.insert(lines.begin() + caret.y + 1, rest);
}
void FileEditor::del(bool right) {
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
			std::string line = lines[lineNr];
			lines.erase(lines.begin() + lineNr);
			lines[lineNr - 1].append(line);
		}
		else {
			lines[caret.y].erase(lines[caret.y].begin() + (caret.x - 1));
		}
	}
}

void FileEditor::save() {
	if (path.empty()) throw std::logic_error("Cannot save file with empty path, use saveAs() instead.");
	std::ofstream file { path };
	for(const auto& line : lines) {
		file << line << "\n";
	}
}
void FileEditor::saveAs(const std::string& path) {
	setPath(path);
	std::ofstream file { path };
	for(const auto& line : lines) {
		file << line << "\n";
	}
}

void FileEditor::setPath(const std::string& _path) {
	fs::path temp = fs::path{_path};
	this->path = fs::absolute(temp).string();
	for (int i = path.size() - 1; i >= 0; i--) {
		if(path[i] == '/' || path[i] == '\\') break;
		fullFilename += path[i];
	}
	std::reverse(fullFilename.begin(), fullFilename.end());
	filename = fullFilename.substr(0, fullFilename.find('.'));
	extension = fullFilename.substr(filename.size() + 1);
}

void FileEditor::close() {
	lines.clear();
	lines.shrink_to_fit();
}
