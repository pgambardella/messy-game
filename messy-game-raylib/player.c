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
   playerData->currentXP = 0.0f;
    playerData->maxXP = PLAYER_BASE_MAX_XP;
    playerData->kickForce = PLAYER_BASE_KICK_FORCE;
    playerData->moveSpeed = PLAYER_BASE_MOVE_SPEED;
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

    // Get input manager
    InputManager* input = GetInputManager();
    if (input == NULL) {
        // Skip movement if no input manager available
        return;
    }

    // Get player data to access player's level-based stats
    PlayerData* playerData = PlayerGetData(player);
    if (!playerData) return;

    // Store previous position for collision resolution
    float prevX = player->x;
    float prevY = player->y;

    // Handle movement input using the input manager
    Vector2 moveDir = InputManagerGetMovementVector(input);

    // Apply acceleration based on input direction and player's move speed
    player->speedX += moveDir.x * PLAYER_ACCEL * deltaTime * playerData->moveSpeed;
    player->speedY += moveDir.y * PLAYER_ACCEL * deltaTime * playerData->moveSpeed;

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

    // Apply speed limits adjusted for player's level-based move speed
    float maxSpeed = PLAYER_MAX_SPEED * playerData->moveSpeed;
    if (player->speedX > maxSpeed) player->speedX = maxSpeed;
    if (player->speedX < -maxSpeed) player->speedX = -maxSpeed;
    if (player->speedY > maxSpeed) player->speedY = maxSpeed;
    if (player->speedY < -maxSpeed) player->speedY = -maxSpeed;

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

    // Enforce boundaries to prevent going off-screen
    float worldWidthPixels = world->width * TILE_WIDTH;
    float worldHeightPixels = world->height * TILE_HEIGHT;

    // Add a small buffer from the edges
    float buffer = 2.0f;
    if (player->x < buffer) {
        player->x = buffer;
        player->speedX = 0;
    }
    else if (player->x > worldWidthPixels - buffer) {
        player->x = worldWidthPixels - buffer;
        player->speedX = 0;
    }

    if (player->y < buffer) {
        player->y = buffer;
        player->speedY = 0;
    }
    else if (player->y > worldHeightPixels - buffer) {
        player->y = worldHeightPixels - buffer;
        player->speedY = 0;
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
        playerData->currentFrame = (playerData->currentFrame + 1) % 8; // 8 frames per animation
    }
}

/**
* @brief Award XP to player and handle level up
*
* @param player Pointer to player entity
* @param xpAmount Amount of XP to award
* @return true If player leveled up
* @return false If no level up occurred
*/
bool PlayerAwardXP(Entity * player, float xpAmount) {
    if (!player || player->type != ENTITY_PLAYER) return false;

    PlayerData* playerData = PlayerGetData(player);
    if (!playerData) return false;

    // Add XP
    playerData->currentXP += xpAmount;

    // Check for level up
    if (playerData->currentXP >= playerData->maxXP) {
        // Level up!
        playerData->level++;

        // Calculate remaining XP (overflow)
        float remainingXP = playerData->currentXP - playerData->maxXP;

        // Increase max XP for next level
        playerData->maxXP *= PLAYER_XP_SCALE_FACTOR;

        // Reset current XP with any overflow
        playerData->currentXP = remainingXP;

        // Increase player stats
        playerData->kickForce += PLAYER_BASE_KICK_FORCE * PLAYER_KICK_FORCE_PER_LEVEL;
        playerData->moveSpeed += PLAYER_BASE_MOVE_SPEED * PLAYER_MOVE_SPEED_PER_LEVEL;

        // Optional: Heal player on level up
        playerData->maxHealth += 10.0f;
        playerData->currentHealth = playerData->maxHealth;

        TraceLog(LOG_INFO, "Player leveled up to %d! New kick force: %.2f, New speed: %.2f",
            playerData->level, playerData->kickForce, playerData->moveSpeed);

        return true;
    }

    return false;
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

    // Get texture manager and renderer
    TextureManager* textures = GetTextureManager();
    Renderer* renderer = GetRenderer();
    if (!textures || !renderer) return;

    // Calculate source rectangle based on animation state
    // Account for 1-pixel gaps and top empty row

    // Real sprite dimensions (without gaps)
    const int actualSpriteWidth = 16;
    const int actualSpriteHeight = 16;

    // Starting pixel for different animations (accounting for 1px offset at top and gaps)
    int sourceY = 0;

    switch (playerData->currentAnim) {
    case ANIM_IDLE_DOWN:
    case ANIM_WALK_DOWN:
        sourceY = 1; // Starting after the 1px empty row
        break;
    case ANIM_IDLE_RIGHT:
    case ANIM_WALK_RIGHT:
        sourceY = 1 + actualSpriteHeight + 1; // Row 2 (after 1px gap)
        break;
    case ANIM_IDLE_UP:
    case ANIM_WALK_UP:
        sourceY = 1 + (actualSpriteHeight + 1) * 2; // Row 3
        break;
    case ANIM_IDLE_LEFT:
    case ANIM_WALK_LEFT:
        sourceY = 1 + (actualSpriteHeight + 1) * 3; // Row 4
        break;
    }

    // For idle animations, use frame 0; otherwise use the current animation frame
    int frameToUse = 0;
    if (playerData->currentAnim == ANIM_WALK_DOWN ||
        playerData->currentAnim == ANIM_WALK_UP ||
        playerData->currentAnim == ANIM_WALK_LEFT ||
        playerData->currentAnim == ANIM_WALK_RIGHT) {
        frameToUse = playerData->currentFrame;
    }

    // Calculate source X (with 1px gaps)
    int sourceX = frameToUse * (actualSpriteWidth /* + 1 */ );

    // Create source rectangle
    Rectangle source = {
        (float)sourceX,
        (float)sourceY,
        (float)actualSpriteWidth,
        (float)actualSpriteHeight
    };

    // Create destination rectangle
    Rectangle dest = {
        player->x - actualSpriteWidth / 2,
        player->y - actualSpriteHeight / 2,
        (float)actualSpriteWidth,
        (float)actualSpriteHeight
    };

    // Draw sprite
    DrawTexturePro(
        TextureManagerGet(textures, TEXTURE_PLAYER),
        source,
        dest,
        (Vector2) {
        0, 0
    },
        0.0f,
        player->tint
    );
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