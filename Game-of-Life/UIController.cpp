#include "UIController.h"
#include <iostream>
#include <algorithm>
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <locale>
#include <codecvt>

UIController::UIController(Universe* universe, int window_width, int window_height, GridView* grid_view) {
	this->universe = universe;
	this->window_width = window_width;
	this->window_height = window_height;
	this->grid_view = grid_view;
	this->panel_width = window_width / 4;
	int margin = std::min(this->panel_width / 20, window_height / 20);
	float height = window_height / 8 - 2 * margin;

	this->buttons.emplace_back(
		new UI::Button(
			window_width - this->panel_width + margin,
			margin,
			this->panel_width - 2 * margin,
			height,
			"Play",
			UI::Button::ID::Play
		)
	);
	this->buttons.emplace_back(
		new UI::Button(
			window_width - this->panel_width + margin,
			2 * margin + height,
			this->panel_width - 2 * margin,
			height,
			"Next",
			UI::Button::ID::Next
		)
	);

	this->buttons.emplace_back(
		new UI::Button(
			window_width - this->panel_width + margin,
			3 * margin + 2 * height,
			this->panel_width - 2 * margin,
			height,
			"Recenter",
			UI::Button::ID::Recenter
		)
	);
	
	this->buttons.emplace_back(
		new UI::Button(
			window_width - this->panel_width + margin,
			4 * margin + 3 * height,
			this->panel_width - 2 * margin,
			height,
			"Load",
			UI::Button::ID::Load
		)
	);

	this->buttons.emplace_back(
		new UI::Button(
			window_width - this->panel_width + margin,
			5 * margin + 4 * height,
			this->panel_width - 2 * margin,
			height,
			"Export",
			UI::Button::ID::Export
		)
	);

	this->buttons.emplace_back(
		new UI::Button(
			window_width - this->panel_width + margin,
			6 * margin + 5 * height,
			this->panel_width - 2 * margin,
			height,
			"Help",
			UI::Button::ID::Help
		)
	);

	this->buttons.emplace_back(
		new UI::Button(
			window_width - this->panel_width + margin,
			7 * margin + 6 * height,
			this->panel_width - 2 * margin,
			height,
			"Help",
			UI::Button::ID::Clear
		)
	);
}

bool isDialogOpen = false;

void UIController::handleInput(const SDL_Event& event) {
	static uint32_t dialog_close_time = 0;
	const uint32_t DIALOG_COOLDOWN = 200;

	if (SDL_GetTicks() - dialog_close_time < DIALOG_COOLDOWN) {
		return;
	}

	int mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);

	UI::Button* hovered_button = nullptr;

	for (auto& button : this->buttons) {
		if (button->isHovered(mouse_x, mouse_y)) {
			button->setColor(SDL_Color{0, 0, 0, 255});
			hovered_button = button;
		} else {
			button->setColor(SDL_Color{250, 0, 250, 255});
		}
	}

	uint32_t current_time = SDL_GetTicks();
	double elapsed_time = (current_time - this->last_button_press) / 1000.0;

	if (hovered_button && event.button.button == SDL_BUTTON_LEFT && elapsed_time >= this->action_time) {
		if (hovered_button->getID() == UI::Button::ID::Play) {

			hovered_button->setID(UI::Button::ID::Stop);
		} else if (hovered_button->getID() == UI::Button::ID::Stop) {

			hovered_button->setID(UI::Button::ID::Play);
		} else if (hovered_button->getID() == UI::Button::ID::Next) {
			this->universe->nextGeneration();
		} else if (hovered_button->getID() == UI::Button::ID::Recenter) {
			this->grid_view->recenter();
		} else if (hovered_button->getID() == UI::Button::ID::Load) {
			std::string filename = this->convertWStringToString(this->openLoadFileDialog());
			dialog_close_time = SDL_GetTicks();
			this->universe->loadFromFile(filename);
		} else if (hovered_button->getID() == UI::Button::ID::Export) {
			std::string filename = this->convertWStringToString(this->openSaveFileDialog());
			dialog_close_time = SDL_GetTicks();
			this->universe->exportToFile(filename);
		} else if (hovered_button->getID() == UI::Button::ID::Help) {

		} else if (hovered_button->getID() == UI::Button::ID::Clear) {
			this->universe->reset();
		}

		this->last_button_press = current_time;
	}
}

void UIController::onWindowResize(int window_width, int window_height) {
	this->window_width = window_width;
	this->window_height = window_height;
	this->panel_width = window_width / 4;
}

bool UIController::isInsidePanel(int mouse_x, int mouse_y) {
	return mouse_x >= (this->window_width - this->panel_width) && mouse_x < this->window_width;
}

void UIController::render(SDL_Renderer* renderer) {
	SDL_Rect panel = {
		this->window_width - this->panel_width,
		0,
		this->panel_width,
		this->window_height
	};

	SDL_SetRenderDrawColor(renderer, 150, 150, 200, 255);
	SDL_RenderFillRect(renderer, &panel);


	for (auto button : this->buttons) {
		button->render(renderer);
	}
}

int UIController::getPanelWidth() {
	return this->panel_width;
}

std::wstring UIController::openLoadFileDialog() {
	OPENFILENAME ofn;
	wchar_t szFile[260] = {0}; // Buffer for file name (wide string)

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr; // No specific window (can pass SDL window handle if needed)
	ofn.lpstrFile = szFile;  // Wide-character file name buffer
	ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
	ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0"; // Wide-character filter
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box
	if (GetOpenFileName(&ofn) == TRUE) {
		return std::wstring(szFile); // Convert wide string to std::wstring
	} else {
		return L""; // Dialog canceled or failed
	}
}

std::wstring UIController::openSaveFileDialog() {
	OPENFILENAME ofn;
	wchar_t file_name[100] = {0};

	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFile = file_name;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(file_name) / sizeof(wchar_t);
	ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";  
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = L"txt";  
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;  

	if (GetSaveFileName(&ofn) == TRUE) {
		std::wstring file(file_name);

		if (file.find(L".txt") == std::wstring::npos) {
			file += L".txt";
		}

		return file;
	}

	return L"";
}




std::string UIController::convertWStringToString(const std::wstring& wstr) {
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string str(len, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], len, nullptr, nullptr);
	return str;
}



