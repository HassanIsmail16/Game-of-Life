#include "Universe.h"
#include <fstream>
#include <random>
#include <ctime>
#include <iostream>

Universe::Universe(int width, int height, int percent) {
	this->initialize(width, height, percent);
}

void Universe::reset() {
	// set all cells to dead
	for (int i = 0; i < this->getHeight(); i++) {
		for (int j = 0; j < this->getWidth(); j++) {
			this->simulation_grid[i][j] = CellState::Dead;
		}
	}
	this->rendering_grid = this->simulation_grid; // sync rendering grid
}

int Universe::countNeighbors(int cell_x, int cell_y) {
	int count = 0;

	for (int i = cell_y - 1; i <= cell_y + 1; i++) {
		for (int j = cell_x - 1; j <= cell_x + 1; j++) {
			if (i == cell_y && j == cell_x) {
				continue;
			} // skip if current cell

			if (i < 0 || i >= this->getHeight() || j < 0 || j >= this->getWidth()) {
				continue;
			} // skip if out of bounds

			if (this->simulation_grid[i][j] == CellState::Alive) {
				count++;
			} // count if in bounds
		}
	}

	return count;
}

void Universe::nextGeneration() {
	std::lock_guard<std::mutex> lock(this->grid_mutex);
	// create a temporary simulation_grid to store the next generation
	Grid next_grid(this->getHeight(), std::vector<CellState>(this->getWidth(), CellState::Dead));

	for (int i = 0; i < this->getHeight(); i++) {
		for (int j = 0; j < this->getWidth(); j++) {
			int neighbors = this->countNeighbors(j, i);
			if (this->simulation_grid[i][j] == CellState::Alive) {
				if (neighbors < 2 || neighbors > 3) {
					next_grid[i][j] = CellState::Dead; // if cell is alive and has less than 2 or more than 3 alive neighbors, it'll become dead
				} else {
					next_grid[i][j] = CellState::Alive; // if cell is alive and has exactly 2 or 3 alive neighbors, it'll remain alive
				}
			} else {
				if (neighbors == 3) {
					next_grid[i][j] = CellState::Alive; // if cell is dead and has exactly 3 alive neighbors, it'll become alive
				}
			}
		}
	}

	// replace old simulation_grid with new simulation_grid
	this->simulation_grid = std::move(next_grid);
}

void Universe::setCellState(int cell_x, int cell_y, CellState state) {
	{
		std::lock_guard<std::mutex> lock(this->grid_mutex);
		this->simulation_grid[cell_y][cell_x] = state;
		this->rendering_grid[cell_y][cell_x] = state; // sync rendering grid
	}
}

CellState Universe::getCellState(int cell_x, int cell_y) const {
	std::lock_guard<std::mutex> lock(grid_mutex); // lock simulation_grid mutex for thread safety

	if (cell_x >= 0 && cell_x < this->getWidth() && cell_y >= 0 && cell_y <= this->getHeight()) {
		return this->simulation_grid[cell_y][cell_x];
	} // if requested cell is in bounds return its state

	return CellState::Dead; // return dead if out of bounds
}

int Universe::getWidth() const {
	if (this->simulation_grid.empty()) return 0; // return 0 if simulation_grid is empty
	return this->simulation_grid.front().size();
}

int Universe::getHeight() const {
	return this->simulation_grid.size();
}

void Universe::loadFromFile(std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cout << "ERROR: Couldn't load from file" << std::endl;
		return;
	} // exit if couldn't open file

	int read_width = 0, read_height = 0;
	if (!(file >> read_width >> read_height)) {
		std::cout << "ERROR: Couldn't read width and height" << std::endl;
		return;
	} // exit if now width or height

	int width = std::clamp(read_width, 5, 100000); // clamp width to 5-100000
	int height = std::clamp(read_height, 5, 10000000 / width); // clamp height to 5-10000000/width
	
	// skip newline after reading width and height
	file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	// temporary simulation_grid to store file data
	Grid temp_grid(height, std::vector<CellState>(width, CellState::Dead));
	
	std::string current_line;
	int row = 0; // current row
	
	while (row < std::min(read_height, height) && std::getline(file, current_line)) {
		if (current_line.empty()) continue; // skip empty lines

		for (int col = 0; col < std::min({static_cast<int>(current_line.length()), width, read_width}); col++) {
			temp_grid[row][col] = (current_line[col] == '1') ? CellState::Alive : CellState::Dead;
		} // set cell state

		row++; // move to next row
	}

	{
		std::lock_guard<std::mutex> lock(this->grid_mutex);

		// set simulation_grid to new simulation_grid
		this->simulation_grid = std::move(temp_grid);
		this->rendering_grid = this->simulation_grid; // sync rendering grid
	}
}

void Universe::exportToFile(std::string& filename) {
	std::ofstream file(filename);

	if (!file.is_open()) {
		std::cout << "ERROR: Couldn't save file" << std::endl;
		return;
	} // exit if couldn't open file

	file << this->getWidth() << " " << this->getHeight() << std::endl; // write width and height

	// write cell states
	for (int i = 0; i < this->getHeight(); i++) {
		for (int j = 0; j < this->getWidth(); j++) {
			if (this->simulation_grid[i][j] == CellState::Alive) {
				file << 1;
			} else {
				file << 0;
			}
		}
		file << std::endl;
	}
}

void Universe::display() {
	for (auto row : this->simulation_grid) {
		for (auto cell : row) {
			std::cout << (cell == CellState::Alive ? "1" : "0") << ' ';
		}
		std::cout << std::endl;
	}
}

void Universe::setGridSize(int width, int height) {
	if (width <= 0 || height <= 0) {
		std::cerr << "ERROR: Invalid grid size: width = " << width << ", height = " << height << std::endl;
		return;
	} // exit if width or height is less than or equal to 0 

	try {
		auto copy = this->createEmptyGrid(width, height);

		int copy_height = std::min(this->getHeight(), height);
		int copy_width = std::min(this->getWidth(), width);

		{
			std::lock_guard<std::mutex> lock(this->grid_mutex);

			// copy old simulation_grid to the new grid
			for (int i = 0; i < copy_height; i++) {
				for (int j = 0; j < copy_width; j++) {
					copy[i][j] = this->simulation_grid[i][j];
				}
			}

			this->simulation_grid = std::move(copy); // update grid size
			this->rendering_grid = this->simulation_grid; // sync rendering grid
		}
	} catch (const std::exception& e) {
		std::cout << "ERROR: Exception during grid resize: " << e.what() << std::endl;
	} catch (...) {
		std::cout << "ERROR: Unknown exception during grid resize" << std::endl;
	}
}

void Universe::initialize(int width, int height, int percent) {
	Grid grid = this->createEmptyGrid(width, height); // create empty simulation_grid

	int total_cells = width * height; // total number of cells
	int num_alive = static_cast<int>(total_cells * (percent / 100.0f)); // number of alive cells

	std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr))); // get random number based on time
	std::uniform_int_distribution<int> dist_x(0, width - 1); // get random x coordinate between 0 and width
	std::uniform_int_distribution<int> dist_y(0, height - 1); // get random y coordinate between 0 and height

	auto placeRandomAlive = [&](int count) {
		for (int i = 0; i < count; i++) {
			int x = dist_x(rng); // get random x coordinate based on a random number
			int y = dist_y(rng); // get random y coordinate based on a random number

			while (grid[y][x] == CellState::Alive) {
				x = dist_x(rng);
				y = dist_y(rng);
			} // if cell is already alive, get new random coordinates
			
			grid[y][x] = CellState::Alive; // set cell to alive
		}
	}; // function to place a number of alive cells randomly on the simulation_grid

	placeRandomAlive(num_alive); // execute random placement
	
	{
		std::lock_guard<std::mutex> lock(this->grid_mutex);
		this->simulation_grid = std::move(grid); // set the simulation grid
		this->rendering_grid = this->simulation_grid; // sync rendering grid
	}
}


void Universe::updateRenderingGrid() {
	std::lock_guard<std::mutex> lock(this->grid_mutex);
	this->rendering_grid = this->simulation_grid;
}

const Grid& Universe::getRenderingGrid() {
	std::lock_guard<std::mutex> lock(this->grid_mutex);
	return this->rendering_grid;;
}

Grid Universe::createEmptyGrid(int width, int height) {
	Grid grid(height, std::vector<CellState>(width, CellState::Dead));
	return grid;
}
