#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <SDL_image.h>
#include "GridView.h"


class UIController {
public:
	UIController(Universe* universe, int window_width, int window_height, GridView* grid_view);
	void handleInput(const SDL_Event& event);
	void onWindowResize(int window_width, int window_height);
	bool isInsidePanel(int mouse_x, int mouse_y);
	void render(SDL_Renderer* renderer);
	int getPanelWidth();
	void play();
	void handlePlayStopButton();
	
	GridView* grid_view; // TODO: encapsulate

	bool isHelpWindowOpen();
	void renderHelpWindow();

	void help_handleInput(SDL_Event& event);
private:
	std::wstring openLoadFileDialog();
	std::wstring openSaveFileDialog();
	std::string convertWStringToString(const std::wstring& wstr);
	void lockDestructiveButtons(bool locked);


	void openHelpWindow();
	void closeHelpWindow();
	void help_RenderText(const char* text, int x, int y);
	void help_RenderContent();
	
	void adjustGridSizeTextboxValues();

	Universe* universe;
	std::vector<UI::Button*> buttons; // start, next, load, export, recenter, help
	UI::Slider* speed_slider;
	std::vector<UI::NumericTextBox*> textboxes;

	double action_time = 0.5;

	int panel_width;
	int window_width;
	int window_height;

	uint32_t last_button_press = 0;
	bool ignore_next_event = false;
	bool isDialogOpen = false;

	double generations_per_second = 5.0f;
	std::atomic<bool> is_playing = false;

	std::atomic<bool> grid_resize_requested{false};
	int requested_width{0};
	int requested_height{0};

	SDL_Window* help_window = nullptr;
	SDL_Renderer* help_renderer = nullptr;
	bool help_window_open = false;
};

