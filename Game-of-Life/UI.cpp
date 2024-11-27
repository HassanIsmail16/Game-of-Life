#include "UI.h"
#include <algorithm>
#include <windows.h>
#include <string>
#include <iostream>

std::string UI::getExecutableDirectory() {
	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);
	std::string exePath(path);
	size_t pos = exePath.find_last_of("\\/");
	return exePath.substr(0, pos); // get executable directory
}

#pragma region Button
UI::Button::Button(int x, int y, int width, int height, std::string text, ID id) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->text = text;
	this->id = id;
	this->color = SDL_Color{ 250, 0, 250, 255 };
	this->font_color = SDL_Color{246, 252, 223, 255};
}

void UI::Button::render(SDL_Renderer* renderer) {
	this->renderBody(renderer);
	this->renderText(renderer);
}

bool UI::Button::isHovered(int mouse_x, int mouse_y) {
	return mouse_x >= this->x && mouse_x < this->x + this->width && mouse_y >= this->y && mouse_y < this->y + this->height;
}

void UI::Button::setText(std::string text) {
	this->text = text;
}

void UI::Button::setColor(SDL_Color color) {
	this->color = color;
}

void UI::Button::setID(ID id) {
	this->id = id;
}

UI::Button::ID UI::Button::getID() {
	return this->id;
}

void UI::Button::setLocked(bool locked) {
	this->locked = locked;
	this->font_color = locked ? SDL_Color{ 205, 210, 190, 255 } : SDL_Color{ 246, 252, 223, 255 };
	this->color = locked ? SDL_Color{ 29, 61, 10, 255 } : SDL_Color{ 133, 159, 61, 255 };
}

bool UI::Button::isLocked() {
	return this->locked;
}

void UI::Button::renderBody(SDL_Renderer* renderer) {
	SDL_Rect body = {
		this->x,
		this->y,
		this->width,
		this->height
	};

	SDL_SetRenderDrawColor(renderer, this->color.r, this->color.g, this->color.b, this->color.a);
	SDL_RenderFillRect(renderer, &body);
}

void UI::Button::renderText(SDL_Renderer* renderer) {
	auto font_path = getExecutableDirectory() + "\\arialbd.ttf";
	TTF_Font* font = TTF_OpenFont(font_path.c_str(), 14);

	if (!font) std::cout << 124234 << std::endl;

	SDL_Surface* text_surface = TTF_RenderText_Solid(font, this->text.c_str(), {font_color.r, font_color.g, font_color.b});
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
	SDL_Rect text_rect = {this->x + (this->width - text_surface->w) / 2, this->y + (this->height - text_surface->h) / 2, text_surface->w, text_surface->h};

	SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
	SDL_FreeSurface(text_surface);
	SDL_DestroyTexture(text_texture);
	TTF_CloseFont(font);
}
#pragma endregion

#pragma region Slider
UI::Slider::Slider(int x, int y, int width, int height, int min, int max, int value) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->min = min;
	this->max = max;
	this->value = value;
}

void UI::Slider::render(SDL_Renderer* renderer) {
	this->renderBody(renderer);
	this->renderKnob(renderer);
}

bool UI::Slider::isKnobHovered(int mouse_x, int mouse_y) {
	SDL_Rect knob = this->getKnobRect();
	return mouse_x >= knob.x && mouse_x <= knob.x + knob.w && mouse_y >= knob.y && mouse_y <= knob.y + knob.h;
}

bool UI::Slider::isBodyHovered(int mouse_x, int mouse_y) {
	return mouse_x >= this->x && mouse_x <= this->x + this->width && mouse_y >= this->y && mouse_y <= this->y + this->height;
}

void UI::Slider::setValue(int value) {
	this->value = value;
}

int UI::Slider::getValue() {
	return this->value;
}

void UI::Slider::setKnobPosition(int mouse_x) {
	int value = std::clamp(double(mouse_x - this->x) / this->width * this->max, double(this->min), double(this->max));
	this->setValue(value);
}

SDL_Rect UI::Slider::getKnobRect() {
	int knob_x = ((double) this->value / this->max) * this->width + this->x - 2.5;
	int knob_y = this->y + 7;
	int knob_width = ((double) this->width / this->max) + 10;
	int knob_height = this->height / 2 + 5;

	return SDL_Rect{knob_x, knob_y, knob_width, knob_height};
}

void UI::Slider::setKnobColor(SDL_Color color) {
	this->knob_color = color;
}

void UI::Slider::renderBody(SDL_Renderer* renderer) {
	SDL_Rect body = {
		this->x,
		this->y + 2 * this->height / 4,
		this->width,
		this->height / 4
	};

	SDL_SetRenderDrawColor(renderer, 26, 26, 25, 255);
	SDL_RenderFillRect(renderer, &body);
}

void UI::Slider::renderKnob(SDL_Renderer* renderer) {
	SDL_Rect knob = this->getKnobRect();
	SDL_SetRenderDrawColor(renderer, this->knob_color.r, this->knob_color.g, this->knob_color.b, this->knob_color.a);
	SDL_RenderFillRect(renderer, &knob);
}
#pragma endregion

#pragma region TextBox
UI::NumericTextBox::NumericTextBox(int x, int y, int width, int height, int min, int max, int value, TTF_Font* font, std::string label, ID id) {
	this->font = font;
	this->label = label;
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->min = min;
	this->max = max;
	this->value = value;
	this->id = id;
	this->is_focused = false;
	this->text = std::to_string(this->value);
	this->color = SDL_Color{133, 159, 61, 255};
}

void UI::NumericTextBox::render(SDL_Renderer* renderer) {
	this->renderLabel(renderer);
	this->renderBody(renderer);
	this->renderText(renderer);
}

bool UI::NumericTextBox::isHovered(int mouse_x, int mouse_y) {
	return mouse_x >= this->x && mouse_x <= this->x + this->width && mouse_y >= this->y && mouse_y <= this->y + this->height;
}

bool UI::NumericTextBox::isFocused() {
	return this->is_focused;
}

void UI::NumericTextBox::setFocused(bool is_focused) {
	this->is_focused = is_focused;
}

int UI::NumericTextBox::getValue() {
	return this->value;
}

void UI::NumericTextBox::setText(std::string text) {
	this->text = text;
	this->value = std::clamp(std::stoi(text), this->min, this->max);
}

void UI::NumericTextBox::push_back(char ch) {
	this->text.push_back(ch);
}

void UI::NumericTextBox::pop_back() {
	if (this->text.empty()) return; // don't do anything if empty
	this->text.pop_back();
}

std::string UI::NumericTextBox::getText() {
	return this->text;
}

void UI::NumericTextBox::setColor(SDL_Color color) {
	this->color = color;
}

UI::NumericTextBox::ID UI::NumericTextBox::getID() {
	return this->id;
}

void UI::NumericTextBox::renderLabel(SDL_Renderer* renderer) {
	SDL_Surface* label_surface = TTF_RenderText_Solid(this->font, this->label.c_str(), {26, 26, 25, 255});
	SDL_Texture* label_texture = SDL_CreateTextureFromSurface(renderer, label_surface);
	SDL_Rect label_rect = {this->x, this->y - label_surface->h - 5, label_surface->w, label_surface->h};

	SDL_RenderCopy(renderer, label_texture, nullptr, &label_rect);
	SDL_FreeSurface(label_surface);
	SDL_DestroyTexture(label_texture);
}

void UI::NumericTextBox::renderBody(SDL_Renderer* renderer) {
	SDL_SetRenderDrawColor(renderer, this->color.r, this->color.g, this->color.b, this->color.a);
	SDL_Rect box_rect = {this->x, this->y, this->width, this->height};
	SDL_RenderDrawRect(renderer, &box_rect);
}

void UI::NumericTextBox::renderText(SDL_Renderer* renderer) {
	if (!this->text.empty()) {
		SDL_Surface* text_surface = TTF_RenderText_Blended(this->font, this->text.c_str(), {26, 26, 25});
		SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
		SDL_Rect text_rect = {this->x + 5, this->y + (this->height - text_surface->h) / 2, text_surface->w, text_surface->h};

		SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
		SDL_FreeSurface(text_surface);
		SDL_DestroyTexture(text_texture);
	}
}

#pragma endregion