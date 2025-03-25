/**
 * @file player.c
 * @brief Implementation of player entity
 */

#include <stdlib.h>
#include <math.h>
#include "player.h"
#include "config.h"
#include "input.h"
#include "textures.h"
#include "renderer.h"

 /**
  * @brief Create a new player entity
  *
  * @param type Player character type
  * @param x Initial X position
  * @param y Initial Y position
  * @return Entity* Pointer to the created player entity
  */
Entity* PlayerCreate(PlayerType type, float x, float y) {
    // Create base entity
    Entity* player = EntityCreate(ENTITY_PLAYER, x, y, SPRITE_WIDTH, SPRITE_HEIGHT);
    if (!player) {
        TraceLog(LOG_ERROR, "Failed to create player entity");
        return NULL;
    }

    // Create player-specific data
    PlayerData* playerData = (PlayerData*)malloc(sizeof(PlayerData));
    if (!playerData) {
        TraceLog(LOG_ERROR, "Failed to allocate player data");
        EntityDestroy(player);
        return NULL;
    }

    // Initialize player data
    playerData->type = type;
    playerData->currentAnim = ANIM_IDLE_DOWN;
    playerData->frameCounter = 0;
    playerData->currentFrame = 0;
    playerData->level = 1;
    playerData->maxHealth = 100.0f;
    playerData->currentHealth = 100.0f;
    playerData->maxMana = 100.0f;
    playerData->currentMana = 100.0f;
    playerData->hasSpecialAbility = false;

    // Set type-specific data
    player->typeData = playerData;
    player->tint = WHITE;
    player->facing = DIRECTION_DOWN;

    return player;
}

/**
 * @brief Update player state based on input and game state
 *
 * @param player Pointer to player entity
 * @param world Pointer to game world
 * @param deltaTime Time elapsed since last update
 */
void PlayerUpdate(Entity* player, World* world, float deltaTime) {
    if (!player || !world || player->type != ENTITY_PLAYER) return;

    // Handle player movement
    PlayerHandleMovement(player, world, deltaTime);

    // Update player animation
    PlayerUpdateAnimation(player, deltaTime);

    // Add other player update logic here (abilities, status effects, etc.)
}

/**
 * @brief Handle player movement based on input
 *
 * @param player Pointer to player entity
 * @param world Pointer to game world
 * @param deltaTime Time elapsed since last update
 */
void PlayerHandleMovement(Entity* player, World* world, float deltaTime) {
    if (!player || !world || player->type != ENTITY_PLAYER) return;

    // Get input manager (normally this would be passed as a parameter)
    // For this example, we'll assume it's accessible globally or through a game state
    InputManager* input = GetInputManager(); // This function would need to be implemented
    if (input == NULL) {
        // Skip movement if no input manager available
        return;
    }

    // Store previous position for collision resolution
    float prevX = player->x;
    float prevY = player->y;

    // Handle movement input using the input manager
    Vector2 moveDir = InputManagerGetMovementVector(input);

    // Apply acceleration based on input direction
    player->speedX += moveDir.x * PLAYER_ACCEL * deltaTime;
    player->speedY += moveDir.y * PLAYER_ACCEL * deltaTime;

    // Apply deceleration if no input in that direction
    if (moveDir.x == 0) {
        if (player->speedX > 0) {
            player->speedX -= PLAYER_DECEL * deltaTime;
            if (player->speedX < 0) player->speedX = 0;
        }
        else if (player->speedX < 0) {
            player->speedX += PLAYER_DECEL * deltaTime;
            if (player->speedX > 0) player->speedX = 0;
        }
    }

    if (moveDir.y == 0) {
        if (player->speedY > 0) {
            player->speedY -= PLAYER_DECEL * deltaTime;
            if (player->speedY < 0) player->speedY = 0;
        }
        else if (player->speedY < 0) {
            player->speedY += PLAYER_DECEL * deltaTime;
            if (player->speedY > 0) player->speedY = 0;
        }
    }

    // Apply speed limits
    if (player->speedX > PLAYER_MAX_SPEED) player->speedX = PLAYER_MAX_SPEED;
    if (player->speedX < -PLAYER_MAX_SPEED) player->speedX = -PLAYER_MAX_SPEED;
    if (player->speedY > PLAYER_MAX_SPEED) player->speedY = PLAYER_MAX_SPEED;
    if (player->speedY < -PLAYER_MAX_SPEED) player->speedY = -PLAYER_MAX_SPEED;

    // Update position based on speed
    // Try horizontal movement first
    player->x += player->speedX;

    // Check for collision with walls in horizontal direction
    if (WorldIsWallAtPosition(world, player->x, player->y)) {
        // Revert to previous position in x direction
        player->x = prevX;
        player->speedX = 0;
    }

    // Then try vertical movement
    player->y += player->speedY;

    // Check for collision with walls in vertical direction
    if (WorldIsWallAtPosition(world, player->x, player->y)) {
        // Revert to previous position in y direction
        player->y = prevY;
        player->speedY = 0;
    }

    // Update facing direction based on movement
    if (fabs(player->speedX) > fabs(player->speedY)) {
        // Horizontal movement dominates
        if (player->speedX > 0) player->facing = DIRECTION_RIGHT;
        else if (player->speedX < 0) player->facing = DIRECTION_LEFT;
    }
    else if (player->speedY != 0) {
        // Vertical movement dominates
        if (player->speedY > 0) player->facing = DIRECTION_DOWN;
        else if (player->speedY < 0) player->facing = DIRECTION_UP;
    }
}

/**
 * @brief Update player animation state
 *
 * @param player Pointer to player entity
 * @param deltaTime Time elapsed since last update
 */
void PlayerUpdateAnimation(Entity* player, float deltaTime) {
    if (!player || player->type != ENTITY_PLAYER) return;

    PlayerData* playerData = (PlayerData*)player->typeData;
    if (!playerData) return;

    // Determine if the player is moving
    bool isMoving = (fabs(player->speedX) > 0.1f || fabs(player->speedY) > 0.1f);

    // Update animation based on facing direction and movement state
    if (isMoving) {
        switch (player->facing) {
        case DIRECTION_DOWN:
            playerData->currentAnim = ANIM_WALK_DOWN;
            break;
        case DIRECTION_UP:
            playerData->currentAnim = ANIM_WALK_UP;
            break;
        case DIRECTION_LEFT:
            playerData->currentAnim = ANIM_WALK_LEFT;
            break;
        case DIRECTION_RIGHT:
            playerData->currentAnim = ANIM_WALK_RIGHT;
            break;
        }
    }
    else {
        // Idle animations for each direction
        switch (player->facing) {
        case DIRECTION_DOWN:
            playerData->currentAnim = ANIM_IDLE_DOWN;
            break;
        case DIRECTION_UP:
            playerData->currentAnim = ANIM_IDLE_UP;
            break;
        case DIRECTION_LEFT:
            playerData->currentAnim = ANIM_IDLE_LEFT;
            break;
        case DIRECTION_RIGHT:
            playerData->currentAnim = ANIM_IDLE_RIGHT;
            break;
        }
    }

    // Update animation frame timing
    playerData->frameCounter++;
    if (playerData->frameCounter >= ANIM_FRAME_SPEED) {
        playerData->frameCounter = 0;

        // Advance to next frame
        playerData->currentFrame++;

        // Loop animation (assume 3 frames per animation)
        if (playerData->currentFrame > 2) {
            playerData->currentFrame = 0;
        }
    }
}

/**
 * @brief Render player with appropriate animation
 *
 * @param player Pointer to player entity
 */
void PlayerRender(Entity* player) {
    if (!player || player->type != ENTITY_PLAYER) return;

    PlayerData* playerData = (PlayerData*)player->typeData;
    if (!playerData) return;

    // Get texture manager (normally this would be passed as a parameter)
    // For this example, we'll assume it's accessible globally or through a game state
    TextureManager* textures = GetTextureManager(); // This function would need to be implemented

    // Calculate source rectangle based on animation state
    int sourceX = 0;
    int sourceY = 0;

    switch (playerData->currentAnim) {
    case ANIM_IDLE_DOWN:
        sourceX = 0;
        sourceY = 0;
        break;
    case ANIM_WALK_DOWN:
        sourceX = playerData->currentFrame;
        sourceY = 0;
        break;
    case ANIM_IDLE_UP:
        sourceX = 0;
        sourceY = 1;
        break;
    case ANIM_WALK_UP:
        sourceX = playerData->currentFrame;
        sourceY = 1;
        break;
    case ANIM_IDLE_LEFT:
        sourceX = 0;
        sourceY = 2;
        break;
    case ANIM_WALK_LEFT:
        sourceX = playerData->currentFrame;
        sourceY = 2;
        break;
    case ANIM_IDLE_RIGHT:
        sourceX = 0;
        sourceY = 3;
        break;
    case ANIM_WALK_RIGHT:
        sourceX = playerData->currentFrame;
        sourceY = 3;
        break;
    }

    // Get the renderer (normally this would be passed as a parameter)
    Renderer* renderer = GetRenderer(); // This function would need to be implemented

    // Draw the player sprite
    RendererDrawPlayerSprite(
        renderer,
        TEXTURE_PLAYER,
        sourceX,
        sourceY,
        player->x - player->width / 2,
        player->y - player->height / 2,
        player->tint
    );

    // Draw additional player effects or status indicators if needed
}

/**
 * @brief Reset player to starting position
 *
 * @param player Pointer to player entity
 * @param x New X position
 * @param y New Y position
 */
void PlayerReset(Entity* player, float x, float y) {
    if (!player || player->type != ENTITY_PLAYER) return;

    // Reset position
    player->x = x;
    player->y = y;

    // Reset speed
    player->speedX = 0.0f;
    player->speedY = 0.0f;

    // Reset animation
    PlayerData* playerData = (PlayerData*)player->typeData;
    if (playerData) {
        playerData->currentAnim = ANIM_IDLE_DOWN;
        playerData->frameCounter = 0;
        playerData->currentFrame = 0;
        player->facing = DIRECTION_DOWN;
    }
}

/**
 * @brief Get player-specific data from entity
 *
 * @param player Pointer to player entity
 * @return PlayerData* Pointer to player data
 */
PlayerData* PlayerGetData(Entity* player) {
    if (!player || player->type != ENTITY_PLAYER) return NULL;
    return (PlayerData*)player->typeData;
}