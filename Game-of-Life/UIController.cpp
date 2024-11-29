#include "UIController.h"
#include <iostream>
#include <algorithm>
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <locale>
#include <codecvt>

unsigned int UIController::dialog_close_time = 0;
UIController::UIController(Universe* universe, int window_width, int window_height, GridView* grid_view) {
	this->init(universe, window_width, window_height, grid_view);
}

#pragma region Input Handling
void UIController::handleInput(const SDL_Event& event) {
	if (this->isHelpWindowOpen()) {
		return;
	} // supress UI events while help window is open

	if (this->grid_view->isDrawing()) {
		if (event.button.button == SDL_BUTTON_LEFT) {
			return;
		} else {
			this->grid_view->stopDrawing();
		}
	} // supress events while drawing

	if (SDL_GetTicks() - this->dialog_close_time < this->DIALOG_COOLDOWN) {
		return;
	} // supress events if a dialog was just closed

	int mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y); // get mouse position

	uint32_t current_time = SDL_GetTicks();
	double elapsed_time = (current_time - this->last_button_press) / 1000.0; // get time since last button press

	this->handleButtonInputs(event, mouse_x, mouse_y, current_time, elapsed_time);
	this->handleTextBoxInputs(event, mouse_x, mouse_y, current_time, elapsed_time);
	this->handleSliderInputs(event, mouse_x, mouse_y);
}

bool UIController::isInsidePanel(int mouse_x, int mouse_y) {
	return mouse_x >= (this->window_width - this->panel_width) && mouse_x < this->window_width;
}

// ---- button handling ----
void UIController::handleButtonInputs(const SDL_Event& event, int mouse_x, int mouse_y, uint32_t current_time, double elapsed_time) {
	UI::Button* hovered_button = nullptr;

	// update color on hover status
	for (auto& button : this->buttons) {
		if (button->isLocked()) continue; // skip if buttons is locked
		
		if (button->isHovered(mouse_x, mouse_y)) {
			button->setColor(SDL_Color{49, 81, 30, 255}); // dark green on hover
			hovered_button = button;
		} else {
			button->setColor(SDL_Color{133, 159, 62, 255}); // light green if not hovered
		}
	}

	// handle button on click
	if (hovered_button && event.button.button == SDL_BUTTON_LEFT && elapsed_time >= this->action_time) {
		this->handleButtonAction(hovered_button);
		this->last_button_press = current_time;
	}
}
void UIController::handleButtonAction(UI::Button* button) {
	switch (button->getID()) {
		case UI::Button::ID::Play:{
			this->handlePlayStopButton();
			button->setText("Stop");
			button->setID(UI::Button::ID::Stop);
			this->lockDestructiveButtons(true);
			break;	
		}

		case UI::Button::ID::Stop: {
			this->handlePlayStopButton();
			button->setText("Play");
			button->setID(UI::Button::ID::Play);
			this->lockDestructiveButtons(false);
			break;
		}

		case UI::Button::ID::Next: {
			this->universe->nextGeneration();
			break;
		}

		case UI::Button::ID::Recenter: {
			this->grid_view->recenter();
			break;
		}

		case UI::Button::ID::Load: {
			std::string filename = this->convertWStringToString(this->openLoadFileDialog());
			this->dialog_close_time = SDL_GetTicks();
			this->universe->loadFromFile(filename);
			this->grid_view->recenter();
			this->textboxes[0]->setText(std::to_string(this->universe->getWidth()));
			this->textboxes[1]->setText(std::to_string(this->universe->getHeight()));
			break;
		}

		case UI::Button::ID::Export: {
			std::string filename = this->convertWStringToString(this->openSaveFileDialog());
			this->dialog_close_time = SDL_GetTicks();
			this->universe->exportToFile(filename);
			break;
		}

		case UI::Button::ID::Help: {
			if (!this->help_window_open) {
				this->help_window_open = true;
				this->openHelpWindow();
			}
			break;
		}

		case UI::Button::ID::Clear: {
			this->universe->reset();
			break;
		}

		case UI::Button::ID::Confirm: {
			this->universe->setGridSize(this->textboxes[0]->getValue(), this->textboxes[1]->getValue());
			this->grid_view->recenter();
			break;
		}

		case UI::Button::ID::Randomize: {
			int percent = this->textboxes[2]->getValue();
			this->universe->initialize(this->universe->getWidth(), this->universe->getHeight(), percent);
			break;
		}

		default:
			break;
	}
}

// ---- textbox handling ----
void UIController::handleTextBoxInputs(const SDL_Event& event, int mouse_x, int mouse_y, uint32_t current_time, double elapsed_time) {
	for (auto textbox : this->textboxes) {
		// handle left click (focus)
		this->handleTextBoxFocus(event, mouse_x, mouse_y, textbox, elapsed_time);

		// handle text input
		if (textbox->isFocused() && event.type == SDL_TEXTINPUT) {
			this->handleTextInput(event, textbox);
		}

		// handle key down
		if (textbox->isFocused() && event.type == SDL_KEYDOWN) {
			this->handleKeyDown(event, textbox);
		}
	}
}

void UIController::handleTextBoxFocus(const SDL_Event& event, int mouse_x, int mouse_y, UI::NumericTextBox* textbox, double elapsed_time) {
	if (event.button.button == SDL_BUTTON_LEFT && elapsed_time >= this->action_time) {
		if (textbox->isHovered(mouse_x, mouse_y)) {
			textbox->setColor({49, 81, 30, 255});
			textbox->setFocused(true); // focus if clicked
		} else if (textbox->isFocused()) {
			textbox->setColor({133, 159, 61, 255});
			textbox->setFocused(false); // unfocus
			this->handleTextBoxUnfocus(textbox); 
		}
	}
}

void UIController::handleTextBoxUnfocus(UI::NumericTextBox* textbox) {
	std::string current_text = textbox->getText();
	int value = current_text.empty() ? 5 : std::stoi(current_text); // add default value if empty

	// Apply clamping based on the textbox type
	value = this->clampTextBoxValue(textbox, value); // clamp the value between min and max

	textbox->setText(std::to_string(value)); // change text
	this->adjustGridSizeTextboxValues(); // adjust values to match specified boundaries
}

int UIController::clampTextBoxValue(UI::NumericTextBox* textbox, int value) {
	if (textbox == this->textboxes[2]) {
		return std::clamp(value, 0, 100);
	} // clamp value in percentage textbox
	return std::clamp(value, 5, 100000); // clamp value in simulation_grid size textboxes
}

void UIController::handleTextInput(const SDL_Event& event, UI::NumericTextBox* textbox) {
	if (event.text.text[0] >= '0' && event.text.text[0] <= '9') {
		std::string current_text = textbox->getText();
		int new_value = current_text.empty() ? 0 : std::stoi(current_text);

		// handle percentage textbox
		if (textbox == this->textboxes[2]) {
			if (current_text.length() < 3) {
				std::string new_text = current_text + event.text.text[0]; // store new digit
				new_value = std::stoi(new_text);
				if (new_value <= 100) {
					textbox->setText(new_text); // add new digit if it doesn't exceed 100
				}
			}
		} else {
			// handle simulation_grid size textboxes
			int other_index = (textbox == this->textboxes[0]) ? 1 : 0;
			std::string other_text = this->textboxes[other_index]->getText(); // get other textbox value
			int max_digits = 7 - other_text.length(); // get maximum allowed number of digits in current textbox
			max_digits = std::max(max_digits, 1); // ensure at least 1 digit

			if (current_text.length() < max_digits) {
				std::string new_text = current_text + event.text.text[0];
				textbox->setText(new_text);
			} // add new digit if it doesn't exceed max_digits
		}
	}
}

void UIController::handleKeyDown(const SDL_Event& event, UI::NumericTextBox* textbox) {
	if (event.key.keysym.sym == SDLK_BACKSPACE) {
		textbox->pop_back(); // handle backspace
	} else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
		this->handleTextBoxUnfocus(textbox); // unfocus if enter is pressed
	}
}

void UIController::adjustGridSizeTextboxValues() {
	int width = this->textboxes[0]->getValue();
	int height = this->textboxes[1]->getValue();

	// product must not exceed 10000000
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

// ---- speed slider handling ----
void UIController::handleSliderInputs(const SDL_Event& event, int mouse_x, int mouse_y) {
	this->updateSliderKnobColor(mouse_x, mouse_y);
	this->handleSliderDrag(event, mouse_x, mouse_y);
}

void UIController::updateSliderKnobColor(int mouse_x, int mouse_y) {
	if (this->speed_slider->isKnobHovered(mouse_x, mouse_y)) {
		this->speed_slider->setKnobColor(SDL_Color{49, 81, 30, 255}); // change knob color to dark green on hover
	} else {
		this->speed_slider->setKnobColor(SDL_Color{133, 159, 61, 255}); // change knob color to light green if not hovered
	}
}

void UIController::handleSliderDrag(const SDL_Event& event, int mouse_x, int mouse_y) {
	if (event.type == SDL_MOUSEMOTION && (event.motion.state & SDL_BUTTON_LMASK)) {
		if (this->speed_slider->isBodyHovered(mouse_x, mouse_y)) {
			this->speed_slider->setKnobPosition(mouse_x); // change slider knob position to mouse position
			this->generations_per_second = this->speed_slider->getValue(); // update playback speed
		}
	}
}
#pragma endregion

#pragma region Rendering & UI
void UIController::render(SDL_Renderer* renderer) {
	// render panel
	SDL_Rect panel = {
		this->window_width - this->panel_width,
		0,
		this->panel_width,
		this->window_height
	};

	SDL_SetRenderDrawColor(renderer, 249, 252, 223, 255);
	SDL_RenderFillRect(renderer, &panel);

	// render buttons
	for (auto& button : this->buttons) {
		button->render(renderer);
	}

	// render textboxes
	for (auto& textbox : this->textboxes) {
		textbox->render(renderer);
	}

	// render slider
	this->speed_slider->render(renderer);
}
#pragma endregion

#pragma region help
bool UIController::isHelpWindowOpen() {
	return this->help_window_open;
}

void UIController::renderHelpWindow() {
	SDL_RenderClear(this->help_renderer);
	this->help_RenderContent();
	SDL_RenderPresent(this->help_renderer);
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
		return; 
	} // exit if text is empty

	// rendering
	auto font_path = UI::getExecutableDirectory() + "\\assets\\arial.ttf";
	TTF_Font* font = TTF_OpenFont(font_path.c_str(), 14);
	SDL_Color color = {220, 220, 220}; // text color
	SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(help_renderer, surface);
	SDL_Rect dest_rect = {x, y, surface->w, surface->h}; // text size and position
	SDL_RenderCopy(help_renderer, texture, nullptr, &dest_rect); // render

	// clean up
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	TTF_CloseFont(font);
}

void UIController::help_RenderContent() {
	SDL_SetRenderDrawColor(help_renderer, 30, 30, 30, 255);  // background color
	SDL_RenderClear(help_renderer); // clear screen

	const struct HelpEntry {
		const char* text;
		IconType icon;
	} help_text[] = {
		{"Make a cell alive by left clicking in its position, you can also drag the mouse to paint", IconType::Left},
		{"Make a cell die by right clicking in its position, you can also drag the mouse to erase", IconType::Right},
		{"Pan around the grid by moving the mouse while holding down the scroll button", IconType::Pan},
		{"Zoom in and out of the grid by moving your mouse's scrollwheel up and down", IconType::Scroll},
		{"Increase and decrease your brush size by pressing ']' and '[' or moving the scroll wheel up and down while holding ctrl", IconType::Scroll},
		{"Press the play button to run the simulation", IconType::Play},
		{"Control the playback speed using the slider at the bottom of the side panel", IconType::Speed},
		{"", IconType::None},  // Empty line
		{"If a cell is alive and has fewer than 2 alive neighbors it'll die", IconType::Cell},
		{"If a cell is alive and has 2 or 3 alive neighbors it'll live", IconType::Cell},
		{"If a cell is alive and has more than 3 alive neighbors it'll die", IconType::Cell},
		{"If a cell is dead and has exactly three alive neighbors it'll become alive", IconType::Cell}
	}; // help text and associated icons

	int y = 20; // first line position
	for (const auto& entry : help_text) {
		if (strlen(entry.text) > 0) {
			this->help_RenderIcon(entry.icon, 5, y);
			this->help_RenderText(entry.text, 30, y);
		}
		y += 30; // move to next line
	}
}

void UIController::help_handleInput(SDL_Event& event) {
	if ((event.type == SDL_QUIT) ||
		(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) ||
		(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)) {
		this->closeHelpWindow();
		this->help_window_open = false;
	} // handle window closing event
}

void UIController::help_RenderIcon(IconType type, int x, int y) {
	static const char* icon_paths[] = {
		"assets\\left-click.png",      // left click icon
		"assets\\right-click.png",     // right click icon
		"assets\\pan.png",			   // pan icon
		"assets\\scroll.png",          // scroll icon
		"assets\\play.png",			   // play icon
		"assets\\speed.png",		   // speed icon
		"assets\\cell.png"			   // cell icon
	};

	if (type == IconType::None || type < IconType::Left || type > IconType::Cell) {
		return;
	} // exit if invalid icon or no icon


	// icon loading
	SDL_Surface* icon_surface = IMG_Load(icon_paths[static_cast<int>(type) - 1]);
	if (!icon_surface) {
		std::cerr << "ERROR: Couldn't load icon " << icon_paths[static_cast<int>(type) - 1]
			<< " errormsg: " << IMG_GetError() << std::endl;
		return;
	} // handle error

	// texture creation
	SDL_Texture* icon_texture = SDL_CreateTextureFromSurface(help_renderer, icon_surface);
	if (!icon_texture) {
		std::cerr << "ERROR: Couldn't create texture, errormsg: " << SDL_GetError() << std::endl;
		SDL_FreeSurface(icon_surface);
		return;
	} // handle error

	// icon position and size
	SDL_Rect dest_rect = {x, y, 20, 20};  // Adjust size as needed

	// render
	SDL_RenderCopy(help_renderer, icon_texture, NULL, &dest_rect);

	// clean up
	SDL_DestroyTexture(icon_texture);
	SDL_FreeSurface(icon_surface);
}
#pragma endregion

#pragma region initialization
void UIController::init(Universe* universe, int window_width, int window_height, GridView* grid_view) {
	this->universe = universe;
	this->window_width = window_width;
	this->window_height = window_height;
	this->grid_view = grid_view;
	this->panel_width = window_width / 4;
	this->initializeUIComponents();
}

void UIController::initializeUIComponents() {
	int margin = std::min(this->panel_width / 20, window_height / 20);
	float height = this->window_height / 8 - 2 * margin;
	float button_width = this->panel_width - 2 * margin;
	float button_half_width = this->panel_width / 2 - 1.5 * margin;
	float x_first = this->window_width - this->panel_width + margin;
	float x_second = x_first + margin + button_half_width;

	this->initializeButtons(margin, height, button_width, button_half_width, x_first, x_second);
	this->initializeTextBoxes(margin, height, button_width, button_half_width, x_first, x_second);
	this->initializeSlider(margin, height, button_width, button_half_width, x_first, x_second);

}

void UIController::initializeButtons(int margin, float height, float button_width, float button_half_width, float x_first, float x_second) {
	this->buttons.emplace_back(new UI::Button(x_first, margin, button_half_width, height, "Play", UI::Button::ID::Play));
	this->buttons.emplace_back(new UI::Button(x_second, margin, button_half_width, height, "Next", UI::Button::ID::Next));
	this->buttons.emplace_back(new UI::Button(x_first, 2 * margin + height, button_half_width, height, "Recenter", UI::Button::ID::Recenter));
	this->buttons.emplace_back(new UI::Button(x_second, 2 * margin + height, button_half_width, height, "Clear", UI::Button::ID::Clear));
	this->buttons.emplace_back(new UI::Button(x_first, 3 * margin + 2 * height, button_half_width, height, "Load", UI::Button::ID::Load));
	this->buttons.emplace_back(new UI::Button(x_second, 3 * margin + 2 * height, button_half_width, height, "Export", UI::Button::ID::Export));
	this->buttons.emplace_back(new UI::Button(x_first, 4 * margin + 3 * height, button_half_width, height, "Randomize", UI::Button::ID::Randomize));
	this->buttons.emplace_back(new UI::Button(x_first, 5 * margin + 4 * height, button_width, height, "Help", UI::Button::ID::Help));
	this->buttons.emplace_back(new UI::Button(x_first, 8 * margin + 7 * height + 32.5, button_width, height, "Confirm", UI::Button::ID::Confirm));
}

void UIController::initializeTextBoxes(int margin, float height, float button_width, float button_half_width, float x_first, float x_second) {
	auto font_path = UI::getExecutableDirectory() + "\\assets\\arialbd.ttf";
	TTF_Font* font = TTF_OpenFont(font_path.c_str(), 14);
	this->textboxes.emplace_back(new UI::NumericTextBox(x_first, 6 * margin + 5 * height + 15, button_width, height, 5, 1e5, this->universe->getWidth(), font, "Grid Width", UI::NumericTextBox::ID::Width));
	this->textboxes.emplace_back(new UI::NumericTextBox( x_first, 7 * margin + 6 * height + 35, button_width, height, 5, 1e5, this->universe->getHeight(), font, "Grid Height", UI::NumericTextBox::ID::Height));
	this->textboxes.emplace_back(new UI::NumericTextBox( x_second, 4 * margin + 3 * height + 27, button_half_width, height / 2, 0, 100, 20, font, "% alive", UI::NumericTextBox::ID::Percent));
}

void UIController::initializeSlider(int margin, float height, float button_width, float button_half_width, float x_first, float x_second) {
	this->speed_slider = new UI::Slider(x_first, 9 * margin + 8.5 * height + 5, button_width, height / 2);
}
#pragma endregion

#pragma region file dialog
std::wstring UIController::openLoadFileDialog() {
	OPENFILENAME ofn;
	wchar_t file[260] = {0}; // filename

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr; // no specific window owns the dialog
	ofn.lpstrFile = file;  // store filename in file string
	ofn.nMaxFile = sizeof(file) / sizeof(wchar_t);
	ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0"; // file filter
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// display dialog box
	if (GetOpenFileName(&ofn) == TRUE) {
		return std::wstring(file); // convert filename to wstring
	} else {
		return L""; // handle dialog cancellation or faliure
	}
}

std::wstring UIController::openSaveFileDialog() {
	OPENFILENAME ofn;
	wchar_t file_name[260] = {0}; // filename

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetActiveWindow(); // owner is main window
	ofn.lpstrFile = file_name; // store filename in file string
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(file_name) / sizeof(wchar_t);
	ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0"; // file filter
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = L"txt";  
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;  

	// display dialog box
	if (GetSaveFileName(&ofn) == TRUE) {
		std::wstring file(file_name); // convert filename to wstring

		if (file.find(L".txt") == std::wstring::npos) {
			file += L".txt";
		} // add .txt extension

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
#pragma endregion

#pragma region play/stop
void UIController::lockDestructiveButtons(bool locked) {
	for (auto button : this->buttons) {
		if (button->getID() == UI::Button::ID::Load || button->getID() == UI::Button::ID::Export
			|| button->getID() == UI::Button::ID::Next || button->getID() == UI::Button::ID::Randomize
			|| button->getID() == UI::Button::ID::Confirm || button->getID() == UI::Button::ID::Clear) {
			button->setLocked(locked);
		} // lock load, save, next, randomize, clear, and confirm buttons
	}
}

void UIController::play() {
	while (this->is_playing.load()) {
		if (this->grid_resize_requested.load()) {
			this->universe->setGridSize(this->requested_width, this->requested_height);
			this->grid_resize_requested.store(false);
		} // handle resizing events before playing

		auto start_time = std::chrono::steady_clock::now(); // record cycle start time

		this->universe->nextGeneration(); // proceed to next generation every cycle

		std::this_thread::sleep_for(std::chrono::milliseconds(
			static_cast<int>(1000 / this->generations_per_second))
		); // delay time (depends on value of speed slider)

		auto end_time = std::chrono::steady_clock::now(); // record cycle end time

		auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count(); // measure time taken in cycle processing

		if (elapsed_time < (1000 / this->generations_per_second)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>((1000 / this->generations_per_second)) - elapsed_time));
		} // sleep for remaining time
	}
}

void UIController::handlePlayStopButton() {
	if (this->is_playing.load()) {
		this->is_playing.store(false);
	} else {
		this->is_playing.store(true); 
		std::thread play_thread([this]() {
			this->play();
		}); // create a separate thread to run the play method in parallel with the main game thread
		play_thread.detach(); // detach the thread to run independently
	}
}
#pragma endregion

