#include "action.hpp"
#include "editor.hpp"
#include <algorithm>

Action::Action(const Action& other) 
	: actionType(other.actionType),
	action(other.action),
	x(other.x), y(other.y),
	undoAction(other.undoAction),
	doAction(other.doAction) {
}

Action::Action(ActionType actionType, int action, int x, int y, std::function<void()> undoAction, std::function<void()> doAction) 
	: actionType(actionType),
	action(action),
	x(x), y(y),
	undoAction(undoAction),
	doAction(doAction) {
}

void Action::operator()() {
	doAction();
}