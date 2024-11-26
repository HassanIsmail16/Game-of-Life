#include "GridView.h"
#include <algorithm>
#include <iostream>

GridView::GridView(Universe* universe) {
	this->universe = universe;
	this->horizontal_scrollbar = nullptr; // TODO: fix
	this->vertical_scrollbar = nullptr;

	this->offset_x = 0;
	this->offset_y = 0;
	this->is_dragging = false;
	this->drag_start_x = 0;
	this->drag_start_y = 0;
	this->cell_size = 20;
	this->zoom_factor = 1.0f;
	this->is_drawing = false;
	this->brush_size = 1;
	this->brush_x = -1;
	this->brush_y = -1;
	this->window_width = 800;
	this->window_height = 600;
	this->recenter();
}


void GridView::render(SDL_Renderer* renderer, const Universe& universe, int ui_panel_width) {
	SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // grid line color

	int rows = universe.getHeight();
	int cols = universe.getWidth();

	int render_width = this->window_width - ui_panel_width; // Exclude UI panel area
	int render_height = this->window_height;

	// Render vertical grid lines
	for (int x = 0; x <= cols; x++) {
		int screen_x = this->offset_x + x * this->cell_size;

		if (screen_x < 0 || screen_x >= render_width) continue; // Skip lines out of render bounds
		SDL_RenderDrawLine(renderer, screen_x, this->offset_y, screen_x, this->offset_y + rows * this->cell_size);
	}

	// Render horizontal grid lines
	for (int y = 0; y <= rows; y++) {
		int screen_y = this->offset_y + y * this->cell_size;

		if (screen_y < 0 || screen_y >= render_height) continue; // Skip lines out of render bounds
		SDL_RenderDrawLine(renderer, this->offset_x, screen_y, this->offset_x + cols * this->cell_size, screen_y);
	}

	// Render alive cells
	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < cols; col++) {
			int screen_x = this->offset_x + col * this->cell_size;
			int screen_y = this->offset_y + row * this->cell_size;

			// Skip cells completely out of bounds
			if (screen_x + this->cell_size <= 0 || screen_x >= render_width ||
				screen_y + this->cell_size <= 0 || screen_y >= render_height) {
				continue;
			}

			// If the cell is alive, render it
			if (universe.getCellState(col, row) == CellState::Alive) {
				SDL_Rect cell_rect = {
					screen_x,
					screen_y,
					this->cell_size,
					this->cell_size
				};
				SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // alive cell color (green)
				SDL_RenderFillRect(renderer, &cell_rect);
			}

		}
	}
}

void GridView::recenter() {
	this->offset_x = (this->window_width  - this->window_width / 4.0f - this->cell_size * this->universe->getWidth()) / 2;
	this->offset_y = (this->window_height - this->cell_size * this->universe->getHeight()) / 2;
}

void GridView::zoom(float zoom_delta, int mouse_x, int mouse_y, int window_width, int window_height) {
	// Calculate new zoom factor and check if it changed
	float new_zoom_factor = std::clamp(this->zoom_factor + zoom_delta, 0.5f, 3.0f);
	if (new_zoom_factor == this->zoom_factor) return;

	// Convert mouse position to grid coordinates before zoom
	float pre_zoom_grid_x = (mouse_x - this->offset_x) / static_cast<float>(this->cell_size);
	float pre_zoom_grid_y = (mouse_y - this->offset_y) / static_cast<float>(this->cell_size);

	// Store old cell size for ratio calculation
	float old_cell_size = this->cell_size;

	// Update zoom factor and cell size
	this->zoom_factor = new_zoom_factor;
	this->cell_size = static_cast<int>(this->zoom_factor * 20);

	// Calculate the mouse position in grid coordinates after zoom
	float post_zoom_screen_x = pre_zoom_grid_x * this->cell_size;
	float post_zoom_screen_y = pre_zoom_grid_y * this->cell_size;

	// Adjust offsets to keep the mouse point stable
	this->offset_x = mouse_x - post_zoom_screen_x;
	this->offset_y = mouse_y - post_zoom_screen_y;

	// Calculate grid dimensions
	int universe_width = this->universe->getWidth(), universe_height = this->universe->getHeight();
	int grid_width = universe_width * this->cell_size;
	int grid_height = universe_height * this->cell_size;

	// Center grid if smaller than window
	if (grid_width <= window_width) {
		this->offset_x = (window_width - grid_width) / 2;
	} else {
		this->offset_x = std::clamp(this->offset_x, -(grid_width - window_width), 0);
	}

	if (grid_height <= window_height) {
		this->offset_y = (window_height - grid_height) / 2;
	} else {
		this->offset_y = std::clamp(this->offset_y, -(grid_height - window_height), 0);
	}

	// Debug output
	std::cout << "Zoom Factor: " << this->zoom_factor
		<< ", Cell Size: " << this->cell_size
		<< ", Offset X: " << this->offset_x
		<< ", Offset Y: " << this->offset_y
		<< ", Grid Width: " << grid_width
		<< ", Grid Height: " << grid_height << std::endl;
}


void GridView::startDrag(int mouse_x, int mouse_y) {
	this->is_dragging = true;
	this->drag_start_x = mouse_x;
	this->drag_start_y = mouse_y;
}

void GridView::updateDrag(int mouse_x, int mouse_y, int window_width, int window_height) {
	if (!this->is_dragging) return;

	// Calculate grid dimensions
	int universe_width = this->universe->getWidth(), universe_height = this->universe->getHeight();
	int grid_width = universe_width * this->cell_size;
	int grid_height = universe_height * this->cell_size;

	// Calculate new offsets
	int new_offset_x = this->offset_x + (mouse_x - this->drag_start_x);
	int new_offset_y = this->offset_y + (mouse_y - this->drag_start_y);

	// If grid is smaller than window, center it
	if (grid_width <= window_width) {
		this->offset_x = (window_width - grid_width) / 2;
	} else {
		// Bound the offset so the grid stays within limits
		this->offset_x = std::clamp(new_offset_x, -(grid_width - window_width), 0);
	}

	if (grid_height <= window_height) {
		this->offset_y = (window_height - grid_height) / 2;
	} else {
		// Bound the offset so the grid stays within limits
		this->offset_y = std::clamp(new_offset_y, -(grid_height - window_height), 0);
	}

	// Update drag start position
	this->drag_start_x = mouse_x;
	this->drag_start_y = mouse_y;
}

void GridView::stopDrag() {
	this->is_dragging = false;
}

void GridView::setCellState(int mouse_x, int mouse_y, CellState state) {
	int cell_x = (mouse_x - this->offset_x) / this->cell_size;
	int cell_y = (mouse_y - this->offset_y) / this->cell_size;

	if (cell_x < 0 || cell_x >= this->universe->getWidth() || cell_y < 0 || cell_y >= this->universe->getHeight()) {
		return;
	}

	this->universe->setCellState(cell_y, cell_x, state);
}

void GridView::setStateAtBrush(CellState state) {
	if (this->brush_x == -1 || this->brush_y == -1) return;

	int brush_left = this->brush_x - this->cell_size * this->brush_size / 2;
	int brush_top = this->brush_y - this->cell_size * this->brush_size / 2;

	for (int x = 0; x < this->brush_size; ++x) {
		for (int y = 0; y < this->brush_size; ++y) {
			int cell_x = (brush_left + x * this->cell_size - this->offset_x) / this->cell_size;
			int cell_y = (brush_top + y * this->cell_size - this->offset_y) / this->cell_size;

			// Ensure cell indices are within valid bounds
			if (cell_x >= 0 && cell_x < this->universe->getWidth() &&
				cell_y >= 0 && cell_y < this->universe->getHeight()) {
				this->universe->setCellState(cell_x, cell_y, state);
			}
		}
	}
}

int GridView::getCellSize() {
	return this->cell_size;
}

void GridView::startDrawing() {
	this->is_drawing = true;
}

void GridView::stopDrawing() {
	this->is_drawing = false;
}

void GridView::setBrushPosition(int mouse_x, int mouse_y) {
	this->brush_x = mouse_x;
	this->brush_y = mouse_y;
}

void GridView::renderBrush(SDL_Renderer* renderer) {
	if (this->brush_x == -1 || this->brush_y == -1) return;

	if (this->brush_x >= this->window_width - this->window_width / 4) return;

	// Convert the brush position from screen space to grid space
	float grid_brush_x = (this->brush_x - this->offset_x) / static_cast<float>(this->cell_size);
	float grid_brush_y = (this->brush_y - this->offset_y) / static_cast<float>(this->cell_size);

	// Calculate the position and size of the brush in screen space
	SDL_Rect brush = {
		static_cast<int>(grid_brush_x * this->cell_size + this->offset_x) - (this->cell_size * this->brush_size) / 2,
		static_cast<int>(grid_brush_y * this->cell_size + this->offset_y) - (this->cell_size * this->brush_size) / 2,
		static_cast<int>(this->cell_size * this->brush_size),
		static_cast<int>(this->cell_size * this->brush_size)
	};

	// Draw the brush rectangle
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderDrawRect(renderer, &brush);
}

void GridView::increaseBrushSize() {
	this->brush_size = std::min(this->brush_size + 1, 5);
}

void GridView::decreaseBrushSize() {
	this->brush_size = std::max(this->brush_size - 1, 1);
}

void GridView::onWindowResize(int window_width, int window_height) {
	this->window_width = window_width;
	this->window_height = window_height;
}
