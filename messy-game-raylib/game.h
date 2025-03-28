/**
 * @file game.h
 * @brief Core game system
 *
 * This file defines the core game system that coordinates all other systems.
 */
#ifndef MESSY_GAME_GAME_H
#define MESSY_GAME_GAME_H
#include <stdbool.h>
#include "renderer.h"
#include "camera.h"
#include "textures.h"

#include "entity.h"
#include "player.h"
#include "ball.h"
#include "world.h"
#include "input.h"
#include "snake_boss.h"
#include "win_condition.h" // Added win condition header

 /**
  * @brief Game states enumeration
  *
  * Defines all possible game states.
  */
typedef enum {
    GAME_STATE_NONE,
    GAME_STATE_SPLASH,
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER,
    GAME_STATE_VICTORY,
    // Add more game states as needed
    GAME_STATE_COUNT
} GameState;

/**
 * @brief Game structure
 *
 * Central structure that coordinates all game systems.
 */
typedef struct {
    GameState state; // Current game state
    GameState prevState; // Previous game state
    bool isRunning; // Whether game is running

    float gameTime; // Total game time
    float deltaTime; // Time since last update
    int fps; // Current FPS
    Renderer* renderer; // Rendering system
    GameCamera* camera; // Camera system
    TextureManager* textures; // Texture manager
    InputManager* input; // Input system
    World* world; // Game world
    Entity* player; // Player entity
    Entity* ball; // Ball entity
    Entity** entities; // Array of all entities
    int entityCount; // Number of entities
    int entityCapacity; // Capacity of entities array
    WinCondition* winCondition; // Win condition system
    // Add more game attributes as needed
} Game;

/**
 * @brief Create a new game
 *
 * @param screenWidth Width of screen
 * @param screenHeight Height of screen
 * @return Game* Pointer to created game
 */
Game* GameCreate(int screenWidth, int screenHeight);

/**
 * @brief Destroy game and free resources
 *
 * @param game Pointer to game
 */
void GameDestroy(Game* game);

/**
 * @brief Initialize game systems
 *
 * @param game Pointer to game
 * @return bool Whether initialization was successful
 */
bool GameInitialize(Game* game);

/**
 * @brief Run the game loop
 *
 * @param game Pointer to game
 */
void GameRun(Game* game);

/**
 * @brief Update game state
 *
 * @param game Pointer to game
 */
void GameUpdate(Game* game);

/**
 * @brief Render game
 *
 * @param game Pointer to game
 */
void GameRender(Game* game);

/**
 * @brief Initialize world layout
 *
 * Helper function to set up the initial world layout with walls and obstacles.
 *
 * @param world Pointer to world
 * @param camera Pointer to camera
 */
void InitializeWorldLayout(World* world, GameCamera* camera);

/**
 * @brief Initialize win condition in world
 *
 * Helper function to set up the win condition in the world.
 *
 * @param world Pointer to world
 * @param camera Pointer to camera
 * @return WinCondition* Created win condition
 */
WinCondition* InitializeWinCondition(World* world, GameCamera* camera);

/**
 * @brief Shutdown game systems
 *
 * @param game Pointer to game
 */
void GameShutdown(Game* game);

/**
 * @brief Change game state
 *
 * @param game Pointer to game
 * @param newState New game state
 */
void GameChangeState(Game* game, GameState newState);

/**
 * @brief Add entity to game
 *
 * @param game Pointer to game
 * @param entity Pointer to entity
 * @return bool Whether entity was added successfully
 */
bool GameAddEntity(Game* game, Entity* entity);

/**
 * @brief Remove entity from game
 *
 * @param game Pointer to game
 * @param entity Pointer to entity
 * @return bool Whether entity was removed successfully
 */
bool GameRemoveEntity(Game* game, Entity* entity);

/**
 * @brief Handle game events
 *
 * @param game Pointer to game
 */
void GameHandleEvents(Game* game);

/**
 * @brief Reset game to initial state
 *
 * @param game Pointer to game
 */
void GameReset(Game* game);

/**
 * @brief Set player for game
 *
 * @param game Pointer to game
 * @param playerType Player type
 * @return Entity* Pointer to player entity
 */
Entity* GameSetPlayer(Game* game, PlayerType playerType);

/**
 * @brief Set ball for game
 *
 * @param game Pointer to game
 * @param ballType Ball type
 * @return Entity* Pointer to ball entity
 */
Entity* GameSetBall(Game* game, BallType ballType);

/**
 * @brief Load game level
 *
 * @param game Pointer to game
 * @param levelId Level ID
 * @return bool Whether level was loaded successfully
 */
bool GameLoadLevel(Game* game, int levelId);

/**
 * @brief Set snake boss for game
 *
 * @param game Pointer to game
 * @param gridX Initial X position in grid coordinates
 * @param gridY Initial Y position in grid coordinates
 * @param initialLength Initial number of segments
 * @return Entity* Pointer to snake boss entity
 */
Entity* GameSetSnakeBoss(Game* game, int gridX, int gridY, int initialLength);

/**
 * @brief Initialize world layout with snake boss
 *
 * Helper function to set up a world layout with a snake boss enemy.
 *
 * @param world Pointer to world
 * @param camera Pointer to camera
 */
void InitializeWorldLayoutWithSnakeBoss(World* world, GameCamera* camera);

#endif // MESSY_GAME_GAME_H