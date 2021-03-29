#include "action.hpp"
#include "editor.hpp"
#include <algorithm>

Action::Action(ActionType actionType, int action, int x, int y, std::function<void()> undoAction, std::function<void()> doAction) 
	: actionType(actionType),
	action(action),
	x(x), y(y),
	undoAction(undoAction),
	doAction(doAction) {
}

// void Action::undoAction() {
// 	if (isCharInput(action)) {
// 		editor.setCaretLocation(x + data.size(), y);
// 		for (int i = 0; i < (int)data.size(); i++) {
// 			editor.deleteCharL();
// 		}
// 	} else if (isNewLine(action)) {
// 		editor.setCaretLocation(x, y);
// 		editor.deleteCharL();
// 	} else if (isDeletion(action)) {
// 		editor.setCaretLocation(x, y);
// 		std::for_each(data.rbegin(), data.rend(), [&](int character) {
// 			editor.put((char)character);
// 		});
// 	}
// }
// void Action::doAction() {
// 	if (isCharInput(action)) {
// 		editor.setCaretLocation(x, y);
// 		for (int character : data) {
// 			editor.put((char)character);
// 		}
// 	} else if (isNewLine(action)) {
// 		editor.setCaretLocation(x, y);
// 		editor.newLine();
// 	} else if (isDeletion(action)) {
// 		editor.setCaretLocation(x + data.size(), y);
// 		for (int i = 0; i < (int)data.size(); i++) {
// 			editor.deleteCharL();
// 		}
// 	}

// }