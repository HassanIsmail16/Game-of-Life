#pragma once
#include "GridView.h"
#include "UIController.h"

class GridController {
public:
	GridController(UIController* ui_ctrl, GridView* grid_view, Universe* universe): ui_ctrl(ui_ctrl), grid_view(grid_view), universe(universe) {}
	void handleInput(const SDL_Event& event, int width, int height);

private:
	void handleMouseInsideUI(int mouse_x, int mouse_y);
	void handleMouseWheel(const SDL_Event& event, int mouse_x, int mouse_y, int width, int height);
	void handleMouseButton(const SDL_Event& event, int mouse_x, int mouse_y);
	void handleMouseMotion(const SDL_Event& event, int mouse_x, int mouse_y, int width, int height);
	void handleKeyPress(const SDL_Event& event);

	GridView* grid_view;
	UIController* ui_ctrl;
	Universe* universe;
};

