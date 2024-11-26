#include "UI.h"
#include <algorithm>
#include <iostream>

UI::Button::Button(int x, int y, int width, int height, std::string text, ID id) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->text = text;
	this->id = id;
	this->color = SDL_Color{ 250, 0, 250, 255 };
}

void UI::Button::render(SDL_Renderer* renderer) {
	SDL_Rect body = {
		this->x,
		this->y,
		this->width,
		this->height
	};

	SDL_SetRenderDrawColor(renderer, this->color.r, this->color.g, this->color.b, this->color.a);
	SDL_RenderFillRect(renderer, &body);
}


bool UI::Button::isHovered(int mouse_x, int mouse_y) {
	return mouse_x >= this->x && mouse_x < this->x + this->width && mouse_y >= this->y && mouse_y < this->y + this->height;
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
	// slider body
	SDL_Rect body = {
		this->x,
		this->y,
		this->width,
		this->height
	};

	// slider knob
	SDL_Rect knob = this->getKnobRect();

	// rendering
	SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	SDL_RenderFillRect(renderer, &body);

	SDL_SetRenderDrawColor(renderer, this->knob_color.r, this->knob_color.g, this->knob_color.b, this->knob_color.a);
	SDL_RenderFillRect(renderer, &knob);
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
	int knob_x = ((double) this->value / this->max) * this->width + this->x;
	int knob_y = this->y;
	int knob_width = ((double) this->width / this->max);
	int knob_height = this->height;

	return SDL_Rect{knob_x, knob_y, knob_width, knob_height};
}

void UI::Slider::setKnobColor(SDL_Color color) {
	this->knob_color = color;
}

UI::NumericTextBox::NumericTextBox(int x, int y, int width, int height, int min, int max, int value, TTF_Font* font) {
	this->font = font;
	this->label = label;
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->min = min;
	this->max = max;
	this->value = value;
	this->is_focused = false;
	this->text = std::to_string(this->value);
	this->color = SDL_Color{122, 123, 1, 255};
}

void UI::NumericTextBox::render(SDL_Renderer* renderer) {
	std::cout << "t1" << std::endl;
	SDL_Surface* label_surface = TTF_RenderText_Solid(this->font, this->label.c_str(), this->color);
	SDL_Texture* label_texture = SDL_CreateTextureFromSurface(renderer, label_surface);
	SDL_Rect label_rect = {this->x, this->y - label_surface->h - 5, label_surface->w, label_surface->h};
	SDL_RenderCopy(renderer, label_texture, nullptr, &label_rect);
	SDL_FreeSurface(label_surface);
	SDL_DestroyTexture(label_texture);

	std::cout << "t2" << std::endl;

	SDL_SetRenderDrawColor(renderer, this->color.r, this->color.g, this->color.b, this->color.a);
	SDL_Rect box_rect = {this->x, this->y, this->width, this->height};
	SDL_RenderDrawRect(renderer, &box_rect);

	std::cout << "t3" << std::endl;


	SDL_Surface* text_surface = TTF_RenderText_Blended(this->font, this->text.c_str(), {255, 255, 255});
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
	SDL_Rect text_rect = {this->x + 5, this->y + (this->height - text_surface->h) / 2, text_surface->w, text_surface->h};
	SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
	SDL_FreeSurface(text_surface);
	SDL_DestroyTexture(text_texture);
	std::cout << "t4" << std::endl;

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

bool UI::NumericTextBox::isValidText(std::string text) {
	for (auto ch : text) {
		if (!isdigit(ch)) {
			return false;
		}
	}
	
	int value = std::stoi(text);
	return value >= this->min && value <= this->max;
}

void UI::NumericTextBox::setText(std::string text) {
	this->text = text;
	this->value = std::clamp(std::stoi(text), this->min, this->max);
}
