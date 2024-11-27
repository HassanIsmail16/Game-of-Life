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

#pragma region Rendering & UI
public:
	// ---- methods ----
	void render(SDL_Renderer* renderer);

private:
	// ---- attributes ----
	GridView* grid_view;
	Universe* universe;
	std::vector<UI::Button*> buttons; // start, next, load, export, recenter, help
	UI::Slider* speed_slider;
	std::vector<UI::NumericTextBox*> textboxes;


	int panel_width;
	int window_width;
	int window_height;
#pragma endregion

#pragma region Input Handling
public:
	// ---- methods ----
	void handleInput(const SDL_Event& event);
	bool isInsidePanel(int mouse_x, int mouse_y);

private:
	void handleButtonInputs(const SDL_Event& event, int mouse_x, int mouse_y, uint32_t current_time, double elapsed_time);
	void handleButtonAction(UI::Button* button);
	void handleTextBoxInputs(const SDL_Event& event, int mouse_x, int mouse_y, uint32_t current_time, double elapsed_time);
	void handleTextBoxFocus(const SDL_Event& event, int mouse_x, int mouse_y, UI::NumericTextBox* textbox, double elapsed_time);
	void handleTextBoxUnfocus(UI::NumericTextBox* textbox);
	int clampTextBoxValue(UI::NumericTextBox* textbox, int value);
	void handleTextInput(const SDL_Event& event, UI::NumericTextBox* textbox);
	void handleKeyDown(const SDL_Event& event, UI::NumericTextBox* textbox);
	void adjustGridSizeTextboxValues();
	void handleSliderInputs(const SDL_Event& event, int mouse_x, int mouse_y);
	void updateSliderKnobColor(int mouse_x, int mouse_y);
	void handleSliderDrag(const SDL_Event& event, int mouse_x, int mouse_y);

	// ---- attributes ----
	double action_time = 0.5;
	uint32_t last_button_press = 0;
	bool ignore_next_event = false;
#pragma endregion

#pragma region help
public:	
	enum class IconType {
		None,
		Left,
		Right,
		Pan,
		Scroll,
		Play,
		Speed,
		Cell
	};

	// ---- methods ----
	bool isHelpWindowOpen();
	void renderHelpWindow();
	void help_handleInput(SDL_Event& event);

private:
	void openHelpWindow();
	void closeHelpWindow();
	void help_RenderText(const char* text, int x, int y);
	void help_RenderContent();
	void help_RenderIcon(IconType type, int x, int y);

	// ---- attributes ----
	SDL_Window* help_window = nullptr;
	SDL_Renderer* help_renderer = nullptr;
	bool help_window_open = false;
#pragma endregion

#pragma region initialization
	void init(Universe* universe, int window_width, int window_height, GridView* grid_view);
	void initializeUIComponents();
	void initializeButtons(int margin, float height, float button_width, float button_half_width, float x_first, float x_second);
	void initializeTextBoxes(int margin, float height, float button_width, float button_half_width, float x_first, float x_second);
	void initializeSlider(int margin, float height, float button_width, float button_half_width, float x_first, float x_second);
#pragma endregion

#pragma region file dialog
private:
	// ---- methods ----
	std::wstring openLoadFileDialog();
	std::wstring openSaveFileDialog();
	std::string convertWStringToString(const std::wstring& wstr);

	// ---- attributes ----
	bool is_dialog_open = false;
	static uint32_t dialog_close_time;
	const uint32_t DIALOG_COOLDOWN = 200;
#pragma endregion

#pragma region play/stop
private:
	// ---- methods ----
	void lockDestructiveButtons(bool locked);
	void play();
	void handlePlayStopButton();

	// ---- attributes ----
	double generations_per_second = 5.0f;
	std::atomic<bool> is_playing = false;

	std::atomic<bool> grid_resize_requested{false};
	int requested_width{0};
	int requested_height{0};

#pragma endregion
};

