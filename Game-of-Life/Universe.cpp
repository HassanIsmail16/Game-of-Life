#include "Universe.h"
#include <fstream>
#include <random>
#include <ctime>
#include <iostream>

Universe::Universe(int width, int height, int percent) {
	this->initialize(width, height, percent);
}

void Universe::reset() {
	for (int i = 0; i < this->getHeight(); i++) {
		for (int j = 0; j < this->getWidth(); j++) {
			this->grid[i][j] = CellState::Dead;
		}
	}
}

int Universe::countNeighbors(int cell_x, int cell_y) {
	int count = 0;

	for (int i = cell_y - 1; i <= cell_y + 1; i++) {
		for (int j = cell_x - 1; j <= cell_x + 1; j++) {
			if (i == cell_y && j == cell_x) {
				continue;
			}
			if (i < 0 || i >= this->getHeight() || j < 0 || j >= this->getWidth()) {
				continue;
			}
			if (this->grid[i][j] == CellState::Alive) {
				count++;
			}
		}
	}

	return count;
}

void Universe::nextGeneration() {
	auto new_grid = this->grid;

	for (int i = 0; i < this->getHeight(); i++) {
		for (int j = 0; j < this->getWidth(); j++) {
			int neighbors = this->countNeighbors(j, i);
			if (this->grid[i][j] == CellState::Alive) {
				if (neighbors < 2 || neighbors > 3) {
					new_grid[i][j] = CellState::Dead;
				}
			} else {
				if (neighbors == 3) {
					new_grid[i][j] = CellState::Alive;
				}
			}
		}
	}

	this->grid = std::move(new_grid);
}

void Universe::run(int steps) {
	for (int i = 0; i < steps; i++) {
		this->nextGeneration();
	}
}

void Universe::play() {
	
}

void Universe::pause() {

}

void Universe::setPlaybackSpeed(int speed) {

}

int Universe::getPlaybackSpeed() const {
	return 0;
}

void Universe::setCellState(int cell_x, int cell_y, CellState state) {
	this->grid[cell_y][cell_x] = state;
}

CellState Universe::getCellState(int cell_x, int cell_y) const {
	return this->grid[cell_y][cell_x];
}

int Universe::getWidth() const {
	return this->grid.front().size();
}

int Universe::getHeight() const {
	return this->grid.size();
}

void Universe::loadFromFile(std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cout << "couldn't load" << std::endl;
		return;
	}

	int width = 0, height = 0;
	if (!(file >> width >> height)) {
		std::cout << "couldn't read width and height" << std::endl;
		return;
	}

	// Consume the newline after dimensions
	file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	// Create a temporary 2D vector to store transposed data
	std::vector<std::vector<CellState>> temp_grid(height, std::vector<CellState>(width, CellState::Dead));

	std::string current_line;
	int row = 0;

	// Read lines and populate transposed grid
	while (row < height && std::getline(file, current_line)) {
		// Skip empty lines
		if (current_line.empty()) continue;

		// Ensure we don't exceed grid dimensions
		for (int col = 0; col < std::min(static_cast<int>(current_line.length()), width); col++) {
			temp_grid[row][col] = (current_line[col] == '1') ? CellState::Alive : CellState::Dead;
		}

		row++;
	}

	// Move the transposed grid
	this->grid = std::move(temp_grid);
}

void Universe::exportToFile(std::string& filename) {
	std::ofstream file(filename);

	if (!file.is_open()) {
		std::cout << "couldn't save" << std::endl;
		return;
	}

	file << this->getWidth() << " " << this->getHeight() << std::endl;

	for (int i = 0; i < this->getHeight(); i++) {
		for (int j = 0; j < this->getWidth(); j++) {
			if (this->grid[i][j] == CellState::Alive) {
				file << 1;
			} else {
				file << 0;
			}
		}
		file << std::endl;
	}
}

void Universe::display() {
	for (auto row : this->grid) {
		for (auto cell : row) {
			std::cout << (cell == CellState::Alive ? "1" : "0") << ' ';
		}
		std::cout << std::endl;
	}
}

void Universe::initialize(int width, int height, int percent) {
	Grid grid = this->createEmptyGrid(width, height);

	int total_cells = width * height;
	int num_alive = static_cast<int>(total_cells * (percent / 100.0f));

	std::mt19937 rng(static_cast<unsigned int>(std::time(0)));
	std::uniform_int_distribution<int> dist_x(0, width - 1);
	std::uniform_int_distribution<int> dist_y(0, height - 1);

	auto placeRandomAlive = [&](int count) {
		for (int i = 0; i < count; i++) {
			int x = dist_x(rng);
			int y = dist_y(rng);

			while (grid[y][x] == CellState::Alive) {
				x = dist_x(rng);
				y = dist_y(rng);
			}

			grid[y][x] = CellState::Alive;
		}
	};

	placeRandomAlive(num_alive);
	
	this->grid = std::move(grid);
}

Grid Universe::createEmptyGrid(int width, int height) {
	Grid grid(height, std::vector<CellState>(width, CellState::Dead));
	return grid;
}
