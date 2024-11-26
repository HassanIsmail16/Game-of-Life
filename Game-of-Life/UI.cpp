#include "UI.h"
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
