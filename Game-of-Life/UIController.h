#pragma once
#include <string>
#include <vector>
#include "GridView.h"

class UIController {
public:
	UIController(int window_width, int window_height, GridView* grid_view) : window_width(window_width), window_height(window_height), grid_view(grid_view), panel_width(100) {}
	void onWindowResize(int window_width, int window_height);
	bool isInsidePanel(int mouse_x, int mouse_y);
	void render(SDL_Renderer* renderer);
	int getPanelWidth();
	GridView* grid_view; // TODO: encapsulate
private:
	std::vector<UI::Button*> buttons;
	std::vector<UI::Slider*> sliders;
	// place text label here before slider
	// place text label before radio buttons
	UI::RadioButton* draw_live_rb;
	UI::RadioButton* draw_dead_rb;
	UI::RadioButton* draw_toggle_rb;
	// place text label before size slider

	int panel_width;
	int window_width;
	int window_height;
};

