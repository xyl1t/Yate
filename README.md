[![](https://img.shields.io/badge/License-MIT-informational.svg)](https://github.com/xyl1t/Yate/blob/master/LICENSE)
[![](https://img.shields.io/badge/Dependency-C%2B%2B17-critical)](https://en.cppreference.com/w/cpp/compiler_support/17)
[![](https://img.shields.io/badge/Dependency-CMake-critical)](https://cmake.org/)
[![](https://img.shields.io/badge/Dependency-ncurses-critical)](https://invisible-island.net/ncurses/)
# Yate
Yate stands for "Yet another text editor"
It's an extremely simple text editor for the terminal. 
WIP
# Features
Besides being able to edit text with yate, it also has following features
* undo/redo  
  Press `ctrl+u` for undo, and `ctrl+r` for redo.
* search  
  Press `ctrl+f` (f as in "find"), if there is more than one match you can move to the next match by clicking the up arrow or move to the previous match by clicking the down arrow. Press `enter` to confirm or `ESC`/`ctrl+c` to cancel.
* saving a file by pressing `ctrl+s`
* movement shortcuts  
  Besides the usual arrow keys, keys such as `Page up`, `Page down`, `Home` and `End` also work as expected, but on top of that, you can also press `ctrl+x` to jump the the next word and `ctrl+z` to jump to the previous. If you don't want to move your cursor but just want to scroll right or left, you can do that with `ctrl+k` and `ctrl+l`.
* Warning the user when attempting to close a modified file
# Building
Dependencies: 
* A C++17 compatible compiler
* The ncurses library
* [Cmake](https://cmake.org/)
```bash
mkdir build
cd build
cmake ..
make
./yate <file>
```
In order to close yate, press `ctrl+c`
# License
This project is licensed under the [MIT License](https://github.com/xyl1t/Yate/blob/master/LICENSE)
