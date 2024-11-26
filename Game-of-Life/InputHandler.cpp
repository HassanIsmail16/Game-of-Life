#include "InputHandler.h"

void InputHandler::handleInput(const SDL_Event& event, int width, int height) {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    if (this->ui_ctrl->isInsidePanel(mouse_x, mouse_y)) {
        SDL_ShowCursor(SDL_ENABLE);
        return;
    }

    SDL_ShowCursor(SDL_DISABLE);
    // Zooming with mouse wheel
    if (event.type == SDL_MOUSEWHEEL) {
        float zoom_delta = event.wheel.y > 0 ? 0.2f : -0.2f;
        this->grid_view->zoom(zoom_delta, mouse_x, mouse_y, width, height);
    }

    // Mouse button down handling
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        if (event.button.button == SDL_BUTTON_MIDDLE) {
            this->grid_view->startDrag(mouse_x, mouse_y);
        } else if (event.button.button == SDL_BUTTON_LEFT) {
            this->grid_view->setStateAtBrush(CellState::Alive);
            this->grid_view->startDrawing();  // Start continuous drawing
        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            this->grid_view->setStateAtBrush(CellState::Dead);
            this->grid_view->startDrawing();  // Start continuous drawing
        }
    }

    // Mouse button up handling
    if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button == SDL_BUTTON_MIDDLE) {
            this->grid_view->stopDrag();
        } else if (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT) {
            this->grid_view->stopDrawing();  // Stop continuous drawing
        }
    }

    // Mouse motion handling (for continuous drawing while holding down the button)
    if (event.type == SDL_MOUSEMOTION) {
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        if (event.motion.state & SDL_BUTTON_LMASK) {
            // Left button is held down (continue drawing as alive cells)
            this->grid_view->setStateAtBrush(CellState::Alive);
        } else if (event.motion.state & SDL_BUTTON_RMASK) {
            // Right button is held down (continue drawing as dead cells)
            this->grid_view->setStateAtBrush(CellState::Dead);
        }

        if (event.motion.state & SDL_BUTTON_MMASK) {
			// Middle button is held down (dragging)
			this->grid_view->updateDrag(mouse_x, mouse_y, width, height);
        }
        
        //this->grid_view->updateHoveredCell(mouse_x, mouse_y);
        this->grid_view->setBrushPosition(mouse_x, mouse_y);
    }

    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_RIGHTBRACKET) {
            this->grid_view->increaseBrushSize();
        } else if (event.key.keysym.sym == SDLK_LEFTBRACKET) {
            this->grid_view->decreaseBrushSize();
        }
    }
}
