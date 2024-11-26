#pragma once
#include "Universe.h"
#include "UI.h"

class GridView {
public:
	GridView(Universe* universe);
	void render(SDL_Renderer* renderer, const Universe& universe, int ui_panel_width);
	void resize(int width, int height, const Universe& universe);
	void recenter();
	void zoom(float zoom_delta, int mouse_x, int mouse_y, int width, int height);
	void startDrag(int mouse_x, int mouse_y);
	void updateDrag(int mouse_x, int mouse_y, int window_width, int window_height);
	void stopDrag();
	void setCellState(int mouse_x, int mouse_y, CellState state);
	void setStateAtBrush(CellState state);
	int getCellSize();
	void startDrawing();
	void stopDrawing();
	void setBrushPosition(int mouse_x, int mouse_y);
	void renderBrush(SDL_Renderer* renderer);
	void increaseBrushSize();
	void decreaseBrushSize();
	void onWindowResize(int window_width, int window_height);

private:
	Universe* universe;
	UI::ScrollBar* horizontal_scrollbar;
	UI::ScrollBar* vertical_scrollbar;

	// camera attributes
	int offset_x;
	int offset_y;
	bool is_dragging;
	int drag_start_x;
	int drag_start_y;

	// size attributes
	int cell_size;

	// zoom attributes
	float zoom_factor;

	// drawing attributes
	bool is_drawing;
	int brush_size;
	int brush_x;
	int brush_y;

	// window data
	int window_width;
	int window_height;
};

