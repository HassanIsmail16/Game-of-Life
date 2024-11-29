#include "GridController.h"

void GridController::handleInput(const SDL_Event& event, int width, int height) {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    if (this->ui_ctrl->isInsidePanel(mouse_x, mouse_y)) {
        this->handleMouseInsideUI(mouse_x, mouse_y);
        return;
    } // mouse is inside ui panel

    SDL_ShowCursor(SDL_DISABLE); // hide cursor if inside simulation_grid

    // handle zooming and changing brush size
    if (event.type == SDL_MOUSEWHEEL) {
        this->handleMouseWheel(event, mouse_x, mouse_y, width, height);
    }

    // handle mouse button up/down
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        this->handleMouseButton(event, mouse_x, mouse_y);
    }

    // Handle mouse motion 
    if (event.type == SDL_MOUSEMOTION) {
        this->handleMouseMotion(event, mouse_x, mouse_y, width, height);
    }

    // handle keyboard inputs
    if (event.type == SDL_KEYDOWN) {
        this->handleKeyPress(event);
    }
}

void GridController::handleMouseInsideUI(int mouse_x, int mouse_y) {
    SDL_ShowCursor(SDL_ENABLE);
    this->grid_view->setBrushPosition(-1, -1); // unset brush
}

void GridController::handleMouseWheel(const SDL_Event& event, int mouse_x, int mouse_y, int width, int height) {
    auto keystate = SDL_GetKeyboardState(nullptr);

    if (keystate[SDL_SCANCODE_LCTRL]) {
        // change brush size with ctrl + scroll
        if (event.wheel.y > 0) {
            this->grid_view->increaseBrushSize();
        } else {
            this->grid_view->decreaseBrushSize();
        }
    } else {
        // zooming with scrolling
        float zoom_delta = event.wheel.y > 0 ? 0.2f : -0.2f;
        this->grid_view->zoom(zoom_delta, mouse_x, mouse_y, width, height);
    }
}

void GridController::handleMouseButton(const SDL_Event& event, int mouse_x, int mouse_y) {
    if (event.button.button == SDL_BUTTON_MIDDLE) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            this->grid_view->startDrag(mouse_x, mouse_y); // start panning if scroll button down
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            this->grid_view->stopDrag(); // end panning if scroll button up
        }
    } else if (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT) {
        CellState new_state = (event.button.button == SDL_BUTTON_LEFT) ? CellState::Alive : CellState::Dead;
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            this->grid_view->setStateAtBrush(new_state);
            this->grid_view->startDrawing(); // start drawing if left/right button down
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            this->grid_view->stopDrawing(); // stop drawing if left/right button up
        }
    }
}

void GridController::handleMouseMotion(const SDL_Event& event, int mouse_x, int mouse_y, int width, int height) {
    if (event.motion.state & SDL_BUTTON_LMASK) {
        this->grid_view->setStateAtBrush(CellState::Alive); 
    } else if (event.motion.state & SDL_BUTTON_RMASK) {
        this->grid_view->setStateAtBrush(CellState::Dead);
    } // if left/right button down and mouse moves

    if (event.motion.state & SDL_BUTTON_MMASK) {
        this->grid_view->updateDrag(mouse_x, mouse_y, width, height);
    } // if scroll button down and mouse moves, pan around the simulation_grid

    this->grid_view->setBrushPosition(mouse_x, mouse_y); // change brush position to mouse position
}

void GridController::handleKeyPress(const SDL_Event& event) {
    if (event.key.keysym.sym == SDLK_RIGHTBRACKET) {
        this->grid_view->increaseBrushSize(); // increase brush size if user presses ]
    } else if (event.key.keysym.sym == SDLK_LEFTBRACKET) {
        this->grid_view->decreaseBrushSize(); // decrease brush size if user presses [
    }
}
