#include "GridView.h"
#include <algorithm>
#include <iostream>

GridView::GridView(Universe* universe) {
	this->universe = universe;
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

#pragma region Rendering
void GridView::render(SDL_Renderer* renderer, Universe& universe, int ui_panel_width) {
	// capture a snapshot of the current simulation_grid state
	int rows, cols;
	std::vector<std::vector<CellState>> grid_snapshot;

	{
		grid_snapshot = universe.getRenderingGrid();

		// lock simulation_grid while taking a snapshot to prevent any other thread from modifying it
		std::lock_guard<std::mutex> lock(universe.grid_mutex);
		if (grid_snapshot.empty()) {
			return;
		}
		rows = grid_snapshot.size();
		cols = grid_snapshot.front().size();
	}

	SDL_SetRenderDrawColor(renderer, 200, 211, 180, 255); // simulation_grid line color
	int render_width = this->window_width - ui_panel_width; // don't render a simulation_grid in ui panel area
	int render_height = this->window_height;


	// render vertical simulation_grid lines
	for (int x = 0; x <= cols; x++) {
		int screen_x = this->offset_x + x * this->cell_size;
		if (screen_x < 0 || screen_x >= render_width) continue; // Skip lines out of render bounds
		SDL_RenderDrawLine(renderer, screen_x, this->offset_y, screen_x, this->offset_y + rows * this->cell_size);
	}

	// render horizontal simulation_grid lines
	for (int y = 0; y <= rows; y++) {
		int screen_y = this->offset_y + y * this->cell_size;
		if (screen_y < 0 || screen_y >= render_height) continue; // Skip lines out of render bounds
		SDL_RenderDrawLine(renderer, this->offset_x, screen_y, this->offset_x + cols * this->cell_size, screen_y);
	}

	// render alive cells
	try {
		for (int row = 0; row < rows; row++) {
			for (int col = 0; col < cols; col++) {
				// calculate screen position
				int screen_x = this->offset_x + col * this->cell_size;
				int screen_y = this->offset_y + row * this->cell_size;

				// skip cells out of bounds
				if (screen_x + this->cell_size <= 0 || screen_x >= render_width ||
					screen_y + this->cell_size <= 0 || screen_y >= render_height) {
					continue;
				}

				// render if cell is in bounds
				if (row >= 0 && row < grid_snapshot.size() &&
					col >= 0 && col < grid_snapshot[row].size()) {
					if (grid_snapshot[row][col] == CellState::Alive) {
						SDL_Rect cell_rect = {
							screen_x,
							screen_y,
							this->cell_size,
							this->cell_size
						};

						SDL_SetRenderDrawColor(renderer, 252, 197, 45, 255); // alive cell color (yellow)
						SDL_RenderFillRect(renderer, &cell_rect);
					}
				}
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "ERROR: Grid rendering error: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "ERROR: Unknown grid rendering error" << std::endl;
	}
}

int GridView::getCellSize() {
	return this->cell_size;
}
#pragma endregion

#pragma region Camera
void GridView::recenter() {
	this->offset_x = (this->window_width  - this->window_width / 4.0f - this->cell_size * this->universe->getWidth()) / 2;
	this->offset_y = (this->window_height - this->cell_size * this->universe->getHeight()) / 2;
}

void GridView::zoom(float zoom_delta, int mouse_x, int mouse_y, int window_width, int window_height) {
	// get new zoom factor
	float new_zoom_factor = std::clamp(this->zoom_factor + zoom_delta, 0.5f, 3.0f);
	if (new_zoom_factor == this->zoom_factor) return; // exit if no change

	// get mouse position int simulation_grid coordinates before zoom
	float pre_zoom_grid_x = (mouse_x - this->offset_x) / static_cast<float>(this->cell_size);
	float pre_zoom_grid_y = (mouse_y - this->offset_y) / static_cast<float>(this->cell_size);

	// update zoom factor and cell size
	this->zoom_factor = new_zoom_factor;
	this->cell_size = static_cast<int>(this->zoom_factor * 20);

	// get mouse position int simulation_grid coordinates after zoom
	float post_zoom_screen_x = pre_zoom_grid_x * this->cell_size;
	float post_zoom_screen_y = pre_zoom_grid_y * this->cell_size;

	// update offsets
	this->offset_x = mouse_x - post_zoom_screen_x;
	this->offset_y = mouse_y - post_zoom_screen_y;

	// get new simulation_grid dimensions
	int universe_width = this->universe->getWidth(), universe_height = this->universe->getHeight();
	int grid_width = universe_width * this->cell_size;
	int grid_height = universe_height * this->cell_size;

	// clamp offsets to prevent simulation_grid from completely leaving the screen
	this->offset_x = std::clamp(this->offset_x,
		std::min(window_width - grid_width, 0),
		std::max(0, window_width - grid_width));

	this->offset_y = std::clamp(this->offset_y,
		std::min(window_height - grid_height, 0),
		std::max(0, window_height - grid_height));
}

void GridView::startDrag(int mouse_x, int mouse_y) {
	this->is_dragging = true;
	this->drag_start_x = mouse_x;
	this->drag_start_y = mouse_y;
}

void GridView::updateDrag(int mouse_x, int mouse_y, int window_width, int window_height) {
	if (!this->is_dragging) return;

	// get simulation_grid dimensions
	int universe_width = this->universe->getWidth(), universe_height = this->universe->getHeight();
	int grid_width = universe_width * this->cell_size;
	int grid_height = universe_height * this->cell_size;

	// get new offsets
	int new_offset_x = this->offset_x + (mouse_x - this->drag_start_x);
	int new_offset_y = this->offset_y + (mouse_y - this->drag_start_y);

	// handle horizontal panning
	if (grid_width > window_width) {
		// full horizontal panning if in bounds
		this->offset_x = std::clamp(new_offset_x, -(grid_width - window_width), 0);
	} else if (grid_width < window_width) {
		// partial horizontal panning if out of bounds
		int max_pan_x = std::max(0, window_width - grid_width);
		this->offset_x = std::clamp(new_offset_x, -max_pan_x, max_pan_x);
	}

	// handle vertical panning
	if (grid_height > window_height) {
		// full vertical panning if in bounds
		this->offset_y = std::clamp(new_offset_y, -(grid_height - window_height), 0);
	} else if (grid_height < window_height) {
		// partial vertical panning if out of bounds
		int max_pan_y = std::max(0, window_height - grid_height);
		this->offset_y = std::clamp(new_offset_y, -max_pan_y, max_pan_y);
	}

	// update drag start position
	this->drag_start_x = mouse_x;
	this->drag_start_y = mouse_y;
}

void GridView::stopDrag() {
	this->is_dragging = false;
}
#pragma endregion

#pragma region Drawing
void GridView::setCellState(int mouse_x, int mouse_y, CellState state) {
	// get cell at mouse position
	int cell_x = (mouse_x - this->offset_x) / this->cell_size;
	int cell_y = (mouse_y - this->offset_y) / this->cell_size;

	if (cell_x < 0 || cell_x >= this->universe->getWidth() || cell_y < 0 || cell_y >= this->universe->getHeight()) {
		return;
	} // exit if cell is out of bounds
	
	this->universe->setCellState(cell_y, cell_x, state);
}

void GridView::setStateAtBrush(CellState state) {
	if (this->brush_x == -1 || this->brush_y == -1) return; // exit if brush is not set

	// get brush simulation_grid coordinates
	int brush_left = this->brush_x - this->cell_size * this->brush_size / 2;
	int brush_top = this->brush_y - this->cell_size * this->brush_size / 2;

	// set cell states covered by brush
	for (int x = 0; x < this->brush_size; ++x) {
		for (int y = 0; y < this->brush_size; ++y) {
			// get cell coordinates
			int cell_x = (brush_left + x * this->cell_size - this->offset_x) / this->cell_size;
			int cell_y = (brush_top + y * this->cell_size - this->offset_y) / this->cell_size;

			// set cell states if in bounds
			if (cell_x >= 0 && cell_x < this->universe->getWidth() &&
				cell_y >= 0 && cell_y < this->universe->getHeight()) {
				this->universe->setCellState(cell_x, cell_y, state);
			}
		}
	}
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
	if (this->brush_x == -1 || this->brush_y == -1) return; // exit if brush is not set

	if (this->brush_x >= this->window_width - this->window_width / 4) return; // exit if cursor is inside ui side panel

	// get brush simulation_grid coordinates
	float grid_brush_x = (this->brush_x - this->offset_x) / static_cast<float>(this->cell_size);
	float grid_brush_y = (this->brush_y - this->offset_y) / static_cast<float>(this->cell_size);

	// get brush rectangle
	SDL_Rect brush = {
		static_cast<int>(grid_brush_x * this->cell_size + this->offset_x) - (this->cell_size * this->brush_size) / 2,
		static_cast<int>(grid_brush_y * this->cell_size + this->offset_y) - (this->cell_size * this->brush_size) / 2,
		static_cast<int>(this->cell_size * this->brush_size),
		static_cast<int>(this->cell_size * this->brush_size)
	};

	// render brush rectangle
	SDL_SetRenderDrawColor(renderer, 0, 150, 150, 255);
	SDL_RenderDrawRect(renderer, &brush);
}

void GridView::increaseBrushSize() {
	this->brush_size = std::min(this->brush_size + 1, 5);
}

void GridView::decreaseBrushSize() {
	this->brush_size = std::max(this->brush_size - 1, 1);
}

bool GridView::isDrawing() {
	return this->is_drawing;
}

#pragma endregion