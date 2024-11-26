#pragma once
#include <memory>
#include <string>
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
	void run(int steps);
	void play();
	void pause();
	void setPlaybackSpeed(int speed);
	int getPlaybackSpeed() const;
	void setCellState(int cell_x, int cell_y, CellState state);
	CellState getCellState(int cell_x, int cell_y) const;
	int getWidth() const;
	int getHeight() const;
	void loadFromFile(std::string& filename);
	void exportToFile(std::string& filename);
	void display();

private:
	void initialize(int width, int height, int percent);
	Grid createEmptyGrid(int width, int height);
	Grid grid;
};

