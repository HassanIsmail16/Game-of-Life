#include "Game.h"

Game::Game() : window(nullptr), renderer(nullptr), universe(nullptr), grid_view(nullptr), ui_ctrl(nullptr), input_handler(nullptr), is_running(false) {}

Game::~Game() {
    this->cleanup();
}

void Game::run() {
    this->init();
    while (this->is_running) {
        this->handleEvents();
        this->render();
        SDL_Delay(16);
    } // game loop
    this->cleanup();
}

void Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0 || IMG_Init(IMG_INIT_PNG) < 0) {
        std::cerr << "ERROR: Couldn't initialize SDL" << SDL_GetError() << std::endl;
        this->is_running = false;
        return;
    } // initialize SDL, SDLTTF, and SDLIMG

    window = SDL_CreateWindow("Game of Life (Made by Hassan Ali)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    if (!window) {
        std::cerr << "ERROR: Couldn't initialize window" << SDL_GetError() << std::endl;
        this->is_running = false;
        return;
    } // create window

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        std::cerr << "ERROR: Couldn't initialize renderer" << SDL_GetError() << std::endl;
        this->is_running = false;
        return;
    } // create renderer

    this->universe = new Universe(20, 20, 20);
    this->grid_view = new GridView(universe);
    this->ui_ctrl = new UIController(universe, 800, 600, grid_view);
    this->input_handler = new GridController(ui_ctrl, grid_view, universe);

    SDL_StartTextInput(); // initialize text input
    this->is_running = true;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { // handle quitting 
            this->is_running = false;
        } else if (this->ui_ctrl->isHelpWindowOpen()) { // handle help window
            this->ui_ctrl->help_handleInput(event);
        } else { // handle normal events
            this->input_handler->handleInput(event, 800, 600);
            this->ui_ctrl->handleInput(event);
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(this->renderer, 26, 26, 25, 255); // background color
    SDL_RenderClear(this->renderer); // clear renderer

    this->grid_view->render(this->renderer, *(this->universe), 200); // render grid
    this->grid_view->renderBrush(this->renderer); // render brush
    this->ui_ctrl->render(this->renderer); // render menu

    if (this->ui_ctrl->isHelpWindowOpen()) {
        this->ui_ctrl->renderHelpWindow();
    } // help window rendering

    SDL_RenderPresent(renderer);
}

void Game::cleanup() {
    SDL_StopTextInput();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    delete universe;
	delete grid_view;
	delete ui_ctrl;
	delete input_handler;
}
