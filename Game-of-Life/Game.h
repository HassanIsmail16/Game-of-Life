	#pragma once
	#include <SDL.h>
	#include "Universe.h"
	#include "InputHandler.h"
	#include "UIController.h"
	#include <iostream>


	class Game {
	public:
		Game();
		~Game();
		void run();

	private:
		void init();
		void handleEvents();
		void render();
		void cleanup();

		SDL_Window* window;
		SDL_Renderer* renderer;
		Universe* universe;
		GridView* grid_view;
		UIController* ui_ctrl;
		InputHandler* input_handler;

		bool is_running;
	};

