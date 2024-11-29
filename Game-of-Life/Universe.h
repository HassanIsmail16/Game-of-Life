#pragma once
#include <memory>
#include <string>
#include <mutex>
#include <vector>

enum class CellState {
	Dead,
	Alive
};

typedef std::vector<std::vector<CellState>> Grid;

class Universe {
public:
	Universe(int width = 100, int height = 100, int percent = 0);
	void reset();
	int countNeighbors(int cell_x, int cell_y);
	void nextGeneration();
	void setCellState(int cell_x, int cell_y, CellState state);
	CellState getCellState(int cell_x, int cell_y) const;
	int getWidth() const;
	int getHeight() const;
	void setGridSize(int width, int height);
	void loadFromFile(std::string& filename);
	void exportToFile(std::string& filename);
	void display(); // for debug reasons
	void initialize(int width, int height, int percent); // initialize a random simulation_grid
	const Grid& getRenderingGrid();

	mutable std::mutex grid_mutex; // for thread safety
	
private:
	Grid createEmptyGrid(int width, int height);
	
	Grid simulation_grid;
	Grid rendering_grid;
};

