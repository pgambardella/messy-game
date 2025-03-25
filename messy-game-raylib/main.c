/**
 * @file main.c
 * @brief Entry point for the Messy Game
 *
 * This file contains the main function which initializes the game
 * and runs the main game loop.
 */

#include "raylib.h"
#include "game.h"
#include "config.h"

 /**
  * @brief Application entry point
  *
  * Initializes the game, runs the main loop, and cleans up resources.
  *
  * @return int Exit status
  */
int main(void) {
    // Initialize the game
    Game* game = GameCreate(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);

    // Initialize game systems
    if (!GameInitialize(game)) {
        // Handle initialization failure
        TraceLog(LOG_ERROR, "Failed to initialize game");
        GameDestroy(game);
        CloseWindow();
        return 1;
    }

    // Run the game loop until window should close or game ends
    while (!WindowShouldClose() && game->isRunning) {
        // Update and render game
        GameUpdate(game);
        GameRender(game);
    }

    // Clean up resources
    GameShutdown(game);
    GameDestroy(game);
    CloseWindow();

    return 0;
}