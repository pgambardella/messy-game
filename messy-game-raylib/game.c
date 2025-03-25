/**
 * @file game.c
 * @brief Implementation of core game system
 */

#include <stdlib.h>
#include <stdio.h>
#include "game.h"
#include "config.h"

 /**
  * @brief Create a new game instance
  *
  * Allocates and initializes the game structure.
  *
  * @param screenWidth Width of the game screen
  * @param screenHeight Height of the game screen
  * @return Game* Pointer to the created game, or NULL if failed
  */
Game* GameCreate(int screenWidth, int screenHeight) {
    Game* game = (Game*)malloc(sizeof(Game));
    if (!game) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for game");
        return NULL;
    }

    // Initialize game structure
    game->state = GAME_STATE_NONE;
    game->prevState = GAME_STATE_NONE;
    game->isRunning = false;
    game->gameTime = 0.0f;
    game->deltaTime = 0.0f;
    game->fps = 0;

    // Initialize subsystems
    game->textures = TextureManagerCreate(MAX_TEXTURES);
    if (!game->textures) {
        TraceLog(LOG_ERROR, "Failed to create texture manager");
        free(game);
        return NULL;
    }

    game->renderer = RendererCreate(screenWidth, screenHeight, game->textures);
    if (!game->renderer) {
        TraceLog(LOG_ERROR, "Failed to create renderer");
        TextureManagerDestroy(game->textures);
        free(game);
        return NULL;
    }

    game->camera = CameraCreate(screenWidth, screenHeight, 5.0f);
    if (!game->camera) {
        TraceLog(LOG_ERROR, "Failed to create camera");
        RendererDestroy(game->renderer);
        TextureManagerDestroy(game->textures);
        free(game);
        return NULL;
    }

    game->input = InputManagerCreate(20); // Initial capacity for 20 bindings
    if (!game->input) {
        TraceLog(LOG_ERROR, "Failed to create input manager");
        CameraDestroy(game->camera);
        RendererDestroy(game->renderer);
        TextureManagerDestroy(game->textures);
        free(game);
        return NULL;
    }

    // Initialize empty entity list
    game->entityCapacity = 100; // Start with capacity for 100 entities
    game->entities = (Entity**)malloc(sizeof(Entity*) * game->entityCapacity);
    if (!game->entities) {
        TraceLog(LOG_ERROR, "Failed to allocate entity array");
        InputManagerDestroy(game->input);
        CameraDestroy(game->camera);
        RendererDestroy(game->renderer);
        TextureManagerDestroy(game->textures);
        free(game);
        return NULL;
    }

    game->entityCount = 0;
    game->player = NULL;
    game->ball = NULL;
    game->world = NULL;

    return game;
}

/**
 * @brief Destroy game and free resources
 *
 * @param game Pointer to game
 */
void GameDestroy(Game* game) {
    if (!game) return;

    // Free all entities
    for (int i = 0; i < game->entityCount; i++) {
        EntityDestroy(game->entities[i]);
    }
    free(game->entities);

    // Free world if it exists
    if (game->world) {
        WorldDestroy(game->world);
    }

    // Free subsystems
    InputManagerDestroy(game->input);
    CameraDestroy(game->camera);
    RendererDestroy(game->renderer);
    TextureManagerDestroy(game->textures);

    // Free game structure
    free(game);
}

/**
 * @brief Initialize game systems
 *
 * This function initializes all game subsystems and loads initial assets.
 *
 * @param game Pointer to game
 * @return bool Whether initialization was successful
 */
bool GameInitialize(Game* game) {
    if (!game) return false;

    // Initialize audio device
    InitAudioDevice();

    game->input = InputManagerCreate(20);
    if (!game->input) {
        TraceLog(LOG_ERROR, "Failed to create input manager");
        return false;
    }

    // Load default input bindings
    InputManagerLoadDefaultBindings(game->input);

    // Load textures
    if (!TextureManagerLoadGameAssets(game->textures)) {
        TraceLog(LOG_ERROR, "Failed to load game assets");
        return false;
    }

    // Create world
    game->world = WorldCreate(WORLD_WIDTH, WORLD_HEIGHT);
    if (!game->world) {
        TraceLog(LOG_ERROR, "Failed to create world");
        return false;
    }

    // Initialize world with basic layout
    InitializeWorldLayout(game->world, game->camera);

    // Create player
    game->player = GameSetPlayer(game, PLAYER_TYPE_KNIGHT);
    if (!game->player) {
        TraceLog(LOG_ERROR, "Failed to create player");
        return false;
    }

    // Create ball
    game->ball = GameSetBall(game, BALL_TYPE_NORMAL);
    if (!game->ball) {
        TraceLog(LOG_ERROR, "Failed to create ball");
        return false;
    }

    // Set camera to follow player
    CameraFollowTarget(game->camera, game->player);

    // Set initial game state
    GameChangeState(game, GAME_STATE_PLAYING);

    // Game is now running
    game->isRunning = true;

    return true;
}

/**
 * @brief Initialize world layout
 *
 * Helper function to set up the initial world layout with walls and obstacles.
 *
 * @param world Pointer to world
 */
void InitializeWorldLayout(World* world, GameCamera* camera) {
    if (!world) return;

    // Initialize all tiles as empty
    for (int x = 0; x < world->width; x++) {
        for (int y = 0; y < world->height; y++) {
            WorldSetTileType(world, x, y, TILE_TYPE_EMPTY);
        }
    }

    // Create boundary walls
    int visibleWidthInTiles = (int)(SCREEN_WIDTH / (TILE_WIDTH * CAMERA_ZOOM)); 
    int visibleHeightInTiles = (int)(SCREEN_HEIGHT / (TILE_HEIGHT * CAMERA_ZOOM));

    // Calculate tile positions for screen edges
    int leftEdge = (world->width / 2) - (visibleWidthInTiles / 2);
    int rightEdge = (world->width / 2) + (visibleWidthInTiles / 2);
    int topEdge = (world->height / 2) - (visibleHeightInTiles / 2);
    int bottomEdge = (world->height / 2) + (visibleHeightInTiles / 2);

    // Create top and bottom walls
    for (int i = leftEdge; i <= rightEdge; i++) {
        WorldSetTileType(world, i, topEdge, TILE_TYPE_WALL);
        WorldSetTileType(world, i, bottomEdge, TILE_TYPE_WALL);
    }

    // Create left and right walls
    for (int j = topEdge; j <= bottomEdge; j++) {
        WorldSetTileType(world, leftEdge, j, TILE_TYPE_WALL);
        WorldSetTileType(world, rightEdge, j, TILE_TYPE_WALL);
    }

    // Add some walls in the middle for obstacles
    int centerX = world->width / 2;
    int centerY = world->height / 2;

    // Small horizontal wall
    for (int i = centerX - 5; i <= centerX + 5; i++) {
        WorldSetTileType(world, i, centerY, TILE_TYPE_WALL);
    }

    // Small vertical walls
    for (int j = centerY - 3; j <= centerY + 3; j++) {
        WorldSetTileType(world, centerX + 10, j, TILE_TYPE_WALL);
        WorldSetTileType(world, centerX - 10, j, TILE_TYPE_WALL);
    }

    // Update your game->camera setup
    camera->camera.target = (Vector2){ centerX * TILE_WIDTH, centerY * TILE_HEIGHT };
    camera->camera.zoom = CAMERA_ZOOM; 

    if (camera) {
        int centerX = world->width / 2;
        int centerY = world->height / 2;
        camera->camera.target = (Vector2){ (float)(centerX * TILE_WIDTH), (float)(centerY * TILE_HEIGHT) };
        camera->camera.zoom = CAMERA_ZOOM;
    }
}

/**
 * @brief Update game state
 *
 * Main game update function that updates all game systems and entities.
 *
 * @param game Pointer to game
 */
void GameUpdate(Game* game) {
    if (!game) return;

    // Update delta time and game time
    game->deltaTime = GetFrameTime();
    game->gameTime += game->deltaTime;
    game->fps = GetFPS();

    // Update input system
    InputManagerUpdate(game->input);

    // Handle game state transitions based on input
    GameHandleEvents(game);

    // Update entities based on current game state
    if (game->state == GAME_STATE_PLAYING) {
        // Update player
        PlayerUpdate(game->player, game->world, game->deltaTime);

        // Update ball
        BallUpdate(game->ball, game->world, game->player, game->deltaTime);

        // Update all other entities
        for (int i = 0; i < game->entityCount; i++) {
            Entity* entity = game->entities[i];

            // Skip player and ball as they've already been updated
            if (entity == game->player || entity == game->ball) continue;

            EntityUpdate(entity, game->deltaTime);
        }

        // Update world
        WorldUpdate(game->world, game->deltaTime);
    }

    // Update camera last so it can follow updated entities
    CameraUpdate(game->camera, game->deltaTime);
}

/**
 * @brief Render game
 *
 * Main game render function that renders all game elements.
 *
 * @param game Pointer to game
 */
void GameRender(Game* game) {
    if (!game) return;

    // Begin drawing
    RendererBeginFrame(game->renderer);

    // Begin 2D camera mode
    CameraBeginMode(game->camera);

    // Render world
    WorldRender(game->world);

    // Render entities in proper order
    for (int i = 0; i < game->entityCount; i++) {
        // Skip player and ball for now (they'll be rendered separately)
        if (game->entities[i] == game->player || game->entities[i] == game->ball) continue;

        EntityRender(game->entities[i]);
    }

    // Render ball
    if (game->ball) {
        BallRender(game->ball);
    }

    // Render player on top
    if (game->player) {
        PlayerRender(game->player);
    }

    // End 2D camera mode
    CameraEndMode();

    // Render HUD and UI elements (not affected by camera)
    if (game->player) {
        RendererDrawHUD(game->renderer, game->player);
    }

    // Draw debug info if in debug mode
    if (game->renderer->debugMode) {
        DrawText(TextFormat("FPS: %d", game->fps), 10, 10, 20, WHITE);
        DrawText(TextFormat("Entities: %d", game->entityCount), 10, 40, 20, WHITE);

        if (game->player) {
            Vector2 playerPos = { game->player->x, game->player->y };
            DrawText(TextFormat("Player Pos: %.1f, %.1f", playerPos.x, playerPos.y), 10, 70, 20, WHITE);
        }
    }

    // End drawing
    RendererEndFrame(game->renderer);
}

/**
 * @brief Handle game events
 *
 * Processes game inputs and triggers appropriate events.
 *
 * @param game Pointer to game
 */
void GameHandleEvents(Game* game) {
    if (!game) return;

    // Check for game pause/unpause
    if (InputManagerIsActionJustPressed(game->input, ACTION_PAUSE)) {
        if (game->state == GAME_STATE_PLAYING) {
            GameChangeState(game, GAME_STATE_PAUSED);
        }
        else if (game->state == GAME_STATE_PAUSED) {
            GameChangeState(game, GAME_STATE_PLAYING);
        }
    }

    // Check for game reset
    if (InputManagerIsActionJustPressed(game->input, ACTION_RESET)) {
        GameReset(game);
    }

    // Check for menu toggle
    if (InputManagerIsActionJustPressed(game->input, ACTION_MENU)) {
        if (game->state == GAME_STATE_PLAYING || game->state == GAME_STATE_PAUSED) {
            GameChangeState(game, GAME_STATE_MENU);
        }
        else if (game->state == GAME_STATE_MENU) {
            GameChangeState(game, game->prevState);
        }
    }
}

/**
 * @brief Change game state
 *
 * Updates the game state and performs any necessary state transitions.
 *
 * @param game Pointer to game
 * @param newState New game state
 */
void GameChangeState(Game* game, GameState newState) {
    if (!game) return;

    // Skip if already in this state
    if (game->state == newState) return;

    // Store the previous state
    game->prevState = game->state;

    // Exit current state
    switch (game->state) {
    case GAME_STATE_PLAYING:
        // Pause music or other continuous effects
        break;
    case GAME_STATE_MENU:
        // Close menu
        break;
    default:
        break;
    }

    // Set new state
    game->state = newState;

    // Enter new state
    switch (newState) {
    case GAME_STATE_PLAYING:
        // Resume music
        break;
    case GAME_STATE_PAUSED:
        // Show pause menu
        break;
    case GAME_STATE_MENU:
        // Show main menu
        break;
    case GAME_STATE_GAME_OVER:
        // Show game over screen
        break;
    default:
        break;
    }
}

/**
 * @brief Reset game to initial state
 *
 * Resets player, ball, and other game elements to their initial state.
 *
 * @param game Pointer to game
 */
void GameReset(Game* game) {
    if (!game) return;

    // Reset player position to center of world
    if (game->player) {
        float centerX = (game->world->width * TILE_WIDTH) / 2.0f;
        float centerY = (game->world->height * TILE_HEIGHT) / 2.0f;
        PlayerReset(game->player, centerX, centerY);
    }

    // Reset ball position near player
    if (game->ball && game->player) {
        BallReset(game->ball, game->player->x + 20, game->player->y + 20);
    }

    // Reset other game elements as needed

    // Set game state to playing
    GameChangeState(game, GAME_STATE_PLAYING);
}

/**
 * @brief Add entity to game
 *
 * @param game Pointer to game
 * @param entity Pointer to entity
 * @return bool Whether entity was added successfully
 */
bool GameAddEntity(Game* game, Entity* entity) {
    if (!game || !entity) return false;

    // Check if we need to expand the entity array
    if (game->entityCount >= game->entityCapacity) {
        // Double the capacity
        int newCapacity = game->entityCapacity * 2;
        Entity** newEntities = (Entity**)realloc(game->entities, sizeof(Entity*) * newCapacity);

        if (!newEntities) {
            TraceLog(LOG_ERROR, "Failed to expand entity array");
            return false;
        }

        game->entities = newEntities;
        game->entityCapacity = newCapacity;
    }

    // Add the entity to the array
    game->entities[game->entityCount] = entity;
    game->entityCount++;

    return true;
}

/**
 * @brief Remove entity from game
 *
 * @param game Pointer to game
 * @param entity Pointer to entity
 * @return bool Whether entity was removed successfully
 */
bool GameRemoveEntity(Game* game, Entity* entity) {
    if (!game || !entity) return false;

    // Find the entity in the array
    int index = -1;
    for (int i = 0; i < game->entityCount; i++) {
        if (game->entities[i] == entity) {
            index = i;
            break;
        }
    }

    // If entity not found, return false
    if (index == -1) return false;

    // Remove the entity by shifting the rest of the array
    for (int i = index; i < game->entityCount - 1; i++) {
        game->entities[i] = game->entities[i + 1];
    }

    // Decrease entity count
    game->entityCount--;

    // Check if this was the player or ball
    if (entity == game->player) game->player = NULL;
    if (entity == game->ball) game->ball = NULL;

    return true;
}

/**
 * @brief Set player for game
 *
 * Creates a new player entity or replaces the existing one.
 *
 * @param game Pointer to game
 * @param playerType Player type
 * @return Entity* Pointer to player entity
 */
Entity* GameSetPlayer(Game* game, PlayerType playerType) {
    if (!game) return NULL;

    // Create new player at center of world
    float centerX = (game->world->width * TILE_WIDTH) / 2.0f;
    float centerY = (game->world->height * TILE_HEIGHT) / 2.0f;

    Entity* player = PlayerCreate(playerType, centerX, centerY);
    if (!player) {
        TraceLog(LOG_ERROR, "Failed to create player");
        return NULL;
    }

    // Remove old player if exists
    if (game->player) {
        GameRemoveEntity(game, game->player);
        EntityDestroy(game->player);
    }

    // Add new player to entities
    if (!GameAddEntity(game, player)) {
        TraceLog(LOG_ERROR, "Failed to add player to entities");
        EntityDestroy(player);
        return NULL;
    }

    // Set as current player
    game->player = player;

    return player;
}

/**
 * @brief Set ball for game
 *
 * Creates a new ball entity or replaces the existing one.
 *
 * @param game Pointer to game
 * @param ballType Ball type
 * @return Entity* Pointer to ball entity
 */
Entity* GameSetBall(Game* game, BallType ballType) {
    if (!game || !game->player) return NULL;

    // Create new ball near player
    float ballX = game->player->x + 20;
    float ballY = game->player->y + 20;

    Entity* ball = BallCreate(ballType, ballX, ballY);
    if (!ball) {
        TraceLog(LOG_ERROR, "Failed to create ball");
        return NULL;
    }

    // Remove old ball if exists
    if (game->ball) {
        GameRemoveEntity(game, game->ball);
        EntityDestroy(game->ball);
    }

    // Add new ball to entities
    if (!GameAddEntity(game, ball)) {
        TraceLog(LOG_ERROR, "Failed to add ball to entities");
        EntityDestroy(ball);
        return NULL;
    }

    // Set as current ball
    game->ball = ball;

    return ball;
}

/**
 * @brief Shutdown game systems
 *
 * @param game Pointer to game
 */
void GameShutdown(Game* game) {
    if (!game) return;

    // Close audio device
    CloseAudioDevice();

    // Other cleanup as needed
}

/**
 * @brief Load game level
 *
 * @param game Pointer to game
 * @param levelId Level ID
 * @return bool Whether level was loaded successfully
 */
bool GameLoadLevel(Game* game, int levelId) {
    if (!game) return false;

    // Free current world if exists
    if (game->world) {
        WorldDestroy(game->world);
    }

    // Load new world from file
    char filename[64];
    sprintf_s(filename, sizeof(filename), "Assets/Levels/level_%d.dat", levelId);

    game->world = WorldLoad(filename);
    if (!game->world) {
        TraceLog(LOG_ERROR, "Failed to load level %d", levelId);
        return false;
    }

    // Reset player and ball positions
    GameReset(game);

    return true;
}