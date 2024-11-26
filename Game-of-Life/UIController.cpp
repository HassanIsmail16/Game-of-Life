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
	float button_width = this->panel_width - 2 * margin;
	float button_half_width = this->panel_width / 2 - 1.5 * margin;
	float x_first = window_width - this->panel_width + margin;
	float x_second = x_first + margin + button_half_width;

	this->buttons.emplace_back(
		new UI::Button(
			x_first,
			margin,
			button_half_width,
			height,
			"Play",
			UI::Button::ID::Play
		)
	);

	this->buttons.emplace_back(
		new UI::Button(
			x_second,
			margin,
			button_half_width,
			height,
			"Next",
			UI::Button::ID::Next
		)
	);

	this->buttons.emplace_back(
		new UI::Button(
			x_first,
			2 * margin + height,
			button_half_width,
			height,
			"Recenter",
			UI::Button::ID::Recenter
		)
	);
	
	this->buttons.emplace_back(
		new UI::Button(
			x_second,
			2 * margin + height,
			button_half_width,
			height,
			"Clear",
			UI::Button::ID::Clear
		)
	);

	this->buttons.emplace_back(
		new UI::Button(
			x_first,
			3 * margin + 2 * height,
			button_half_width,
			height,
			"Load",
			UI::Button::ID::Load
		)
	);

	this->buttons.emplace_back(
		new UI::Button(
			x_second,
			3 * margin + 2 * height,
			button_half_width,
			height,
			"Export",
			UI::Button::ID::Export
		)
	);


	this->buttons.emplace_back(
		new UI::Button(
			x_first,
			4 * margin + 3 * height,
			button_half_width,
			height,
			"Randomize",
			UI::Button::ID::Randomize
		)
	);

	this->buttons.emplace_back(
		new UI::Button(
			x_first,
			5 * margin + 4 * height,
			button_width,
			height,
			"Help",
			UI::Button::ID::Help
		)
	);

	TTF_Font* font = TTF_OpenFont("arialbd.ttf", 16);

	this->textboxes.emplace_back(
		new UI::NumericTextBox(
			x_first,
			6 * margin + 5 * height + 15,
			button_width,
			height,
			5,
			1e5,
			this->universe->getWidth(),
			font,
			"Grid Width",
			UI::NumericTextBox::ID::Width
		)
	);

	this->textboxes.emplace_back(
		new UI::NumericTextBox(
			x_first,
			7 * margin + 6 * height + 35,
			button_width,
			height,
			5,
			1e5,
			this->universe->getHeight(),
			font,
			"Grid Height",
			UI::NumericTextBox::ID::Height
		)
	);

	this->textboxes.emplace_back(
		new UI::NumericTextBox(
			x_second,
			4 * margin + 3 * height + 27,
			button_half_width,
			height / 2,
			0,
			100,
			20,
			font,
			"% alive",
			UI::NumericTextBox::ID::Percent
		)
	);

	this->buttons.emplace_back(
		new UI::Button(
			x_first,
			8 * margin + 7 * height + 32.5,
			button_width,
			height,
			"Confirm",
			UI::Button::ID::Confirm
		)
	);

	this->speed_slider = new UI::Slider(
		x_first,
		9 * margin + 8.5 * height + 5,
		button_width,
		height / 2
	);


}

bool isDialogOpen = false;

void UIController::handleInput(const SDL_Event& event) {
	if (this->isHelpWindowOpen()) {
		return;
	}

	if (this->grid_view->isDrawing()) {
		if (event.button.button == SDL_BUTTON_LEFT) {
			return;
		} else {
			this->grid_view->stopDrawing();
		}
	}

	static uint32_t dialog_close_time = 0;
	const uint32_t DIALOG_COOLDOWN = 200;

	if (SDL_GetTicks() - dialog_close_time < DIALOG_COOLDOWN) {
		return;
	}

	int mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);

	UI::Button* hovered_button = nullptr;

	for (auto& button : this->buttons) {
		if (button->isLocked()) continue;
		if (button->isHovered(mouse_x, mouse_y)) {
			button->setColor(SDL_Color{49, 81, 30, 255});
			hovered_button = button;
		} else {
			button->setColor(SDL_Color{133, 159, 62, 255});
		}
	}

	uint32_t current_time = SDL_GetTicks();
	double elapsed_time = (current_time - this->last_button_press) / 1000.0;

	if (hovered_button && event.button.button == SDL_BUTTON_LEFT && elapsed_time >= this->action_time) {
		if (hovered_button->getID() == UI::Button::ID::Play) {
			this->handlePlayStopButton();
			hovered_button->setText("Stop");
			hovered_button->setID(UI::Button::ID::Stop);
			this->lockDestructiveButtons(true);
		} else if (hovered_button->getID() == UI::Button::ID::Stop) {
			this->handlePlayStopButton();
			hovered_button->setText("Play");
			hovered_button->setID(UI::Button::ID::Play);
			this->lockDestructiveButtons(false);
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
			if (!this->help_window_open) {
				this->help_window_open = true;
				this->openHelpWindow();
			}
		} else if (hovered_button->getID() == UI::Button::ID::Clear) {
			this->universe->reset();
		} else if (hovered_button->getID() == UI::Button::ID::Confirm) {
			this->universe->setGridSize(this->textboxes[0]->getValue(), this->textboxes[1]->getValue());
			this->grid_view->recenter();
		} else if (hovered_button->getID() == UI::Button::ID::Randomize) {
			int percent = this->textboxes[2]->getValue();
			this->universe->initialize(this->universe->getWidth(), this->universe->getHeight(), percent);
		}

		this->last_button_press = current_time;
	}


	for (auto textbox : this->textboxes) {
		if (event.button.button == SDL_BUTTON_LEFT && elapsed_time >= this->action_time) {
			if (textbox->isHovered(mouse_x, mouse_y)) {
				textbox->setColor({49, 81, 30, 255});
				textbox->setFocused(true);
			} else if (textbox->isFocused()) {
				textbox->setColor({133, 159, 61, 255});
				textbox->setFocused(false);
				std::string current_text = textbox->getText();
				int value = current_text.empty() ? 5 : std::stoi(current_text);

				// Modify the clamping logic to handle the percentage textbox differently
				if (textbox == this->textboxes[2]) {  // Assuming the percentage textbox is the third one
					value = std::clamp(value, 0, 100);
				} else {
					value = std::clamp(value, 5, 100000);
				}

				// Update textbox with clamped value
				textbox->setText(std::to_string(value));
				textbox->setFocused(false);
				// Adjust values after unfocusing
				adjustGridSizeTextboxValues();
			}
		}

		if (textbox->isFocused() && event.type == SDL_TEXTINPUT) {
			if (isdigit(event.text.text[0])) {
				// Get current text
				std::string current_text = textbox->getText();

				// Additional check for percentage textbox
				if (textbox == this->textboxes[2]) {  // Assuming the percentage textbox is the third one
					// Limit to 3 digits for percentage
					if (current_text.length() < 3) {
						std::string new_text = current_text + event.text.text[0];
						int new_value = std::stoi(new_text);

						// Ensure value doesn't exceed 100
						if (new_value <= 100) {
							textbox->setText(new_text);
						}
					}
				} else {
					// Existing logic for other textboxes
					int other_index = (textbox == this->textboxes[0]) ? 1 : 0;
					std::string other_text = this->textboxes[other_index]->getText();
					int max_digits = 7 - other_text.length();
					max_digits = std::max(max_digits, 1);

					if (current_text.length() < max_digits) {
						std::string new_text = current_text + event.text.text[0];
						int new_value = new_text.empty() ? 0 : std::stoi(new_text);
						textbox->setText(new_text);
					}
				}
			}
		}

		if (textbox->isFocused() && event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_BACKSPACE) {
				textbox->pop_back();
			} else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
				std::string current_text = textbox->getText();
				int value = current_text.empty() ? 5 : std::stoi(current_text);

				// Modify the clamping logic to handle the percentage textbox differently
				if (textbox == this->textboxes[2]) {  // Assuming the percentage textbox is the third one
					value = std::clamp(value, 0, 100);
				} else {
					value = std::clamp(value, 5, 100000);
				}

				// Update textbox with clamped value
				textbox->setText(std::to_string(value));
				textbox->setFocused(false);
				// Adjust values after confirming
				adjustGridSizeTextboxValues();
			}
		}
	}

	if (this->speed_slider->isKnobHovered(mouse_x, mouse_y)) {
		this->speed_slider->setKnobColor(SDL_Color{ 49, 81, 30, 255 });
	} else {
		this->speed_slider->setKnobColor(SDL_Color{ 133, 159, 61, 255 });
	}

	if (event.type == SDL_MOUSEMOTION) {
		if (event.motion.state & SDL_BUTTON_LMASK) {
			if (this->speed_slider->isBodyHovered(mouse_x, mouse_y)) {
				this->speed_slider->setKnobPosition(mouse_x);
				this->generations_per_second = this->speed_slider->getValue();
			}
		}
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

	SDL_SetRenderDrawColor(renderer, 249, 252, 223, 255);
	SDL_RenderFillRect(renderer, &panel);


	for (auto& button : this->buttons) {
		button->render(renderer);
	}

	for (auto& textbox : this->textboxes) {
		textbox->render(renderer);
	}

	this->speed_slider->render(renderer);
}

int UIController::getPanelWidth() {
	return this->panel_width;
}

void UIController::play() {
	while (this->is_playing.load()) {
		if (this->grid_resize_requested.load()) {
			this->universe->setGridSize(this->requested_width, this->requested_height);
			this->grid_resize_requested.store(false);
		}

		auto start_time = std::chrono::steady_clock::now();

		this->universe->nextGeneration();

		std::this_thread::sleep_for(std::chrono::milliseconds(
			static_cast<int>(1000 / this->generations_per_second))
		);

		auto end_time = std::chrono::steady_clock::now();
		auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
		if (elapsed_time < (1000 / this->generations_per_second)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>((1000 / this->generations_per_second)) - elapsed_time));
		}
	}
}

void UIController::handlePlayStopButton() {
	if (this->is_playing.load()) {
		this->is_playing.store(false);
	} else {
		this->is_playing.store(true);
		std::thread play_thread([this]() {
			this->play();
		});
		play_thread.detach();
	}
}

bool UIController::isHelpWindowOpen() {
	return this->help_window_open;
}

void UIController::renderHelpWindow() {
	SDL_RenderClear(this->help_renderer);
	help_RenderContent();
	SDL_RenderPresent(this->help_renderer);
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

void UIController::lockDestructiveButtons(bool locked) {
	for (auto button : this->buttons) {
		if (button->getID() == UI::Button::ID::Load || button->getID() == UI::Button::ID::Export
			|| button->getID() == UI::Button::ID::Next || button->getID() == UI::Button::ID::Randomize
			|| button->getID() == UI::Button::ID::Confirm) {
			button->setLocked(locked);
		}
	}
}

void UIController::openHelpWindow() {
	this->help_window = SDL_CreateWindow("Help", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 400, SDL_WINDOW_SHOWN);
	this->help_renderer = SDL_CreateRenderer(this->help_window, -1, SDL_RENDERER_ACCELERATED);
}

void UIController::closeHelpWindow() {
	SDL_DestroyRenderer(this->help_renderer);
	SDL_DestroyWindow(this->help_window);
	this->help_renderer = nullptr;
	this->help_window = nullptr;
}

void UIController::help_RenderText(const char* text, int x, int y) {
	if (text == nullptr || strlen(text) == 0) {
		return;  // Exit if the text is empty
	}
	TTF_Font* font = TTF_OpenFont("arial.ttf", 14);
	SDL_Color color = {255, 255, 255}; // White color
	SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(help_renderer, surface);
	SDL_Rect dstRect = {x, y, surface->w, surface->h};
	SDL_RenderCopy(help_renderer, texture, nullptr, &dstRect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	TTF_CloseFont(font);
}

void UIController::help_RenderContent() {
	const char* helpText[] = {
		"- Make a cell alive by left clicking in its position",
		"- Make a cell die by right clicking in its position",
		"- Pan around the grid by moving the mouse while holding down the scroll button",
		"- Zoom in and out of the grid by moving your mouse's scrollwheel up and down",
		"- Increase and decrease your brush size by pressing ']' and '[' or moving the scroll wheel up and down while holding ctrl",
		"- Press the play button to run the simulation",
		"- Control the playback speed using the slider at the bottom of the side panel",
		"",
		"- If a cell is alive and has fewer than 2 alive neighbors it'll die",
		"- If a cell is alive and has 2 or 3 alive neighbors it'll live",
		"- If a cell is alive and has more than 3 alive neighbors it'll die",
		"- If a cell is dead and has exactly three alive neighbors it'll become alive"
	};

	int yPos = 20;
	for (int i = 0; i < 12; ++i) {
		this->help_RenderText(helpText[i], 20, yPos);
		yPos += 30; // Move down for the next line
	}

}

void UIController::help_handleInput(SDL_Event& event) {
	// Handle more event types for the help window
	switch (event.type) {
		case SDL_QUIT:
		closeHelpWindow();
		this->help_window_open = false;
		break;

		case SDL_KEYDOWN:
		// Allow closing help window with ESC key
		if (event.key.keysym.sym == SDLK_ESCAPE) {
			closeHelpWindow();
			this->help_window_open = false;
		}
		break;

		case SDL_WINDOWEVENT:
		if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
			closeHelpWindow();
			this->help_window_open = false;
		}
		break;
	}
}


void UIController::adjustGridSizeTextboxValues() {
	int width = this->textboxes[0]->getValue();
	int height = this->textboxes[1]->getValue();

	// Ensure product doesn't exceed 10,000,000
	while ((long long) width * height > 10000000) {
		if (width > height) {
			width--;
		} else {
			height--;
		}
		this->textboxes[0]->setText(std::to_string(width));
		this->textboxes[1]->setText(std::to_string(height));
	}
}