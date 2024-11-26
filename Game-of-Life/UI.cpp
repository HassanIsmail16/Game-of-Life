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
	std::cout << value << std::endl;
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

