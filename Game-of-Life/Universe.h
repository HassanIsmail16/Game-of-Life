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
	void loadFromFile(std::string& filename);
	void exportToFile(std::string& filename);
	void display();
	void setGridSize(int width, int height);

	mutable std::mutex grid_mutex;
	Grid grid;
	
	void initialize(int width, int height, int percent);
private:
	Grid createEmptyGrid(int width, int height);
};

