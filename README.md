# Game-of-Life
## Description
This is an implementation of John Conway's Game of Life.

### Rules of The Game
- If a cell is alive and has fewer than 2 alive neighbors it'll die.
- If a cell is alive and has 2 or 3 alive neighbors it'll live.
- If a cell is alive and has more than 3 alive neighbors it'll die.
- If a cell is dead and has exactly three alive neighbors it'll become alive.

### How to Play
- Make a cell alive by left clicking in its position, you can also drag the mouse to paint.
- Make a cell die by right clicking in its position, you can also drag the mouse to erase.
- Pan around the grid by moving the mouse while holding down the scroll button.
- Zoom in and out of the grid by moving your mouse's scrollwheel up and down.
- Increase and decrease your brush size by pressing ']' and '[' or moving the scroll wheel up and down while holding ctrl.
- Press the play button to run the simulation.
- Control the playback speed using the slider at the bottom of the side panel.

<div align="center">
    <img src="https://github.com/user-attachments/assets/81b03e65-3e78-4210-840b-58fdbdb3fd85" alt="An image of the game">
</div>

## How to Run
- You can build the executable directly using Visual Studio 2022. The project solution file is in the repository.
- You can run the executable found in the latest release in the repository if you have the VC++ Redistributable Component.

