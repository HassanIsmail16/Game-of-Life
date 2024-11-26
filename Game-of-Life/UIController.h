#pragma once
#include <string>
#include <vector>
#include "GridView.h"

class UIController {
public:
	UIController(Universe* universe, int window_width, int window_height, GridView* grid_view);
	void handleInput(const SDL_Event& event);
	void onWindowResize(int window_width, int window_height);
	bool isInsidePanel(int mouse_x, int mouse_y);
	void render(SDL_Renderer* renderer);
	int getPanelWidth();
	GridView* grid_view; // TODO: encapsulate


private:
	std::wstring openLoadFileDialog();
	std::wstring openSaveFileDialog();
	std::string convertWStringToString(const std::wstring& wstr);

	Universe* universe;
	std::vector<UI::Button*> buttons; // start, next, load, export, recenter, help
	UI::Slider* speed_slider;
	// place text label here before slider

	double action_time = 0.5;

	int panel_width;
	int window_width;
	int window_height;

	uint32_t last_button_press = 0;
	bool ignore_next_event = false;
	bool isDialogOpen = false;
};

