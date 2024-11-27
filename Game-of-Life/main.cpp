#include <SDL.h>
#include "Universe.h"
#include "InputHandler.h"
#include "UIController.h"
#include <iostream>


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
    SDL_Window* window = SDL_CreateWindow("Game of Life (Made by Hassan Ali)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    Universe universe(10, 20, 0);
    GridView view(&universe);
    UIController ui_ctrl(&universe, 800, 600, &view);
    InputHandler input_handler(&ui_ctrl, &view, &universe);
    SDL_StartTextInput();

    bool is_running = true;
    while (is_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            } else if (ui_ctrl.isHelpWindowOpen()) {
                // Prioritize help window events when help window is open
                ui_ctrl.help_handleInput(event);
            } else {
                // Normal input handling for main window
                input_handler.handleInput(event, 800, 600);
                ui_ctrl.handleInput(event);
            }
        }

        SDL_SetRenderDrawColor(renderer, 26, 26, 25, 255);
        SDL_RenderClear(renderer);
        view.render(renderer, universe, 200);
        view.renderBrush(renderer);
        ui_ctrl.render(renderer);

        if (ui_ctrl.isHelpWindowOpen()) {
            ui_ctrl.renderHelpWindow();
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}