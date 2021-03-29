#ifndef ACTION_HPP
#define ACTION_HPP

#include <ncurses.h>
#include <functional>
class Editor;

enum class ActionType {
	None = 0,
	Input,
	NewLine,
	DeletionL,
	DeletionR,
};

struct Action {
	ActionType actionType;
	int action;
	int x;
	int y;
	std::function<void()> undoAction;
	std::function<void()> doAction;
	
	Action(ActionType actionType, int action, int x, int y, std::function<void()> undoAction, std::function<void()> doAction);

	void operator()();

	
	static inline bool isInput(int action) {
		return (isCharInput(action) || action == 32 || action == KEY_STAB || action == 9 || action == KEY_ENTER || action == 10);
	}
	static inline bool isCharInput(int action) {
		return (action > 32 && action < 127);
	}
	static inline bool isMovement(int action) {
		return (action == KEY_PPAGE || action == KEY_NPAGE || action == 11 || action == 12 || action == KEY_UP || action == KEY_DOWN || action == KEY_LEFT || action == KEY_RIGHT || action == 5 || action == KEY_END || action == KEY_HOME || action == 1 || action == 25 || action == 26 || action == 24);
	}
	static inline bool isDeletion(int action) {
		return action == 127 || action == KEY_BACKSPACE || action == 330 || action == KEY_DL;
	}
	static inline bool isNewLine(int action) {
		return action == 10 || action == KEY_ENTER;
	}
	static inline bool isEqual(int action1, int action2) {
		return (isInput(action1) == isInput(action2)) || (isMovement(action1) == isMovement(action2)) || (isDeletion(action1) == isDeletion(action2)) || (isNewLine(action1) == isNewLine(action2));
	}
};

#endif