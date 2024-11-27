#pragma once
#include "Universe.h"
#include "UI.h"

class GridView {
public:
	GridView(Universe* universe);

#pragma region Rendering
public:
	// ---- methods ----
	void render(SDL_Renderer* renderer, Universe& universe, int ui_panel_width);

private:
	// ---- attributes ----
	int window_width;
	int window_height;
#pragma endregion

#pragma region Camera
public:
	// ---- methods ----
	void recenter();
	void zoom(float zoom_delta, int mouse_x, int mouse_y, int width, int height);
	void startDrag(int mouse_x, int mouse_y);
	void updateDrag(int mouse_x, int mouse_y, int window_width, int window_height);
	void stopDrag();

private:
	// ---- attributes ----
	int offset_x;
	int offset_y;
	bool is_dragging;
	int drag_start_x;
	int drag_start_y;
	
	int cell_size;
	
	float zoom_factor;
#pragma endregion

#pragma region Drawing
public:
	// ---- methods ----
	void setCellState(int mouse_x, int mouse_y, CellState state);
	void setStateAtBrush(CellState state);
	void startDrawing();
	void stopDrawing();
	void setBrushPosition(int mouse_x, int mouse_y);
	void renderBrush(SDL_Renderer* renderer);
	void increaseBrushSize();
	void decreaseBrushSize();
	bool isDrawing();
	int getCellSize();

private:
	// ---- attributes ----
	bool is_drawing;
	int brush_size;
	int brush_x;
	int brush_y;
#pragma endregion

private:
	Universe* universe;
};

