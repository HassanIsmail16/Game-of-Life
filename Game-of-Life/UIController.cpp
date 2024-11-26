#include "UIController.h"
#include <iostream>

void UIController::onWindowResize(int window_width, int window_height) {
	this->window_width = window_width;
	this->window_height = window_height;
	this->panel_width = window_width / 8;
}

bool UIController::isInsidePanel(int mouse_x, int mouse_y) {
	std::cout << "inside" << std::endl;
	return mouse_x >= (this->window_width - this->panel_width) && mouse_x < this->window_width;
}

void UIController::render(SDL_Renderer* renderer) {
	SDL_Rect panel = {
		this->window_width - this->panel_width,
		0,
		this->panel_width,
		this->window_height
	};

	SDL_SetRenderDrawColor(renderer, 0, 250, 0, 255);
	SDL_RenderFillRect(renderer, &panel);
}

int UIController::getPanelWidth() {
	return this->panel_width;
}
