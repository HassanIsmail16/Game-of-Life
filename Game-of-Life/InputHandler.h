#pragma once
#include "GridView.h"
#include "UIController.h"

class InputHandler {
public:
	InputHandler(UIController* ui_ctrl, GridView* grid_view, Universe* universe): ui_ctrl(ui_ctrl), grid_view(grid_view), universe(universe) {}

	void handleInput(const SDL_Event& event, int width, int height);

private:
	GridView* grid_view;
	UIController* ui_ctrl;
	Universe* universe;
};

