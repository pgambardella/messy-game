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
    playerData->state = PLAYER_STATE_ALIVE;
    playerData->deathTimer = 0.0f;

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

    PlayerData* playerData = PlayerGetData(player);
    if (!playerData) return;

    // Handle player state
    switch (playerData->state) {
    case PLAYER_STATE_ALIVE:
        // Handle player movement
        PlayerHandleMovement(player, world, deltaTime);

        // Update player animation
        PlayerUpdateAnimation(player, deltaTime);

        // Check for death condition
        if (playerData->currentHealth <= 0) {
            playerData->state = PLAYER_STATE_DYING;
            playerData->currentAnim = ANIM_DEATH;
            playerData->currentFrame = 0;
            playerData->frameCounter = 0;
            playerData->deathTimer = 0.0f;
            TraceLog(LOG_INFO, "Player has died! Starting death animation.");
        }
        break;

    case PLAYER_STATE_DYING:
    case PLAYER_STATE_DEAD:
        // Handle death animation and screen
        PlayerHandleDeath(player, deltaTime);
        break;
    }
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
    float buffer = 5.0f;
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

    // Skip animation updates if we're in death animation
    if (playerData->currentAnim == ANIM_DEATH) {
        // Handle death animation separately
        return;
    }

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

        // Advance to next frame - limited to 4 frames for walk animations
        if (playerData->currentAnim >= ANIM_WALK_DOWN && playerData->currentAnim <= ANIM_WALK_RIGHT) {
            playerData->currentFrame = (playerData->currentFrame + 1) % 4; // 4 frames per walk animation
        }
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
bool PlayerAwardXP(Entity* player, float xpAmount) {
    if (!player || player->type != ENTITY_PLAYER) return false;

    PlayerData* playerData = PlayerGetData(player);
    if (!playerData) return false;

    // Don't award XP if player is dead
    if (playerData->state != PLAYER_STATE_ALIVE) return false;

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

    // Handle death screen separately
    if (playerData->state == PLAYER_STATE_DEAD) {
        PlayerRenderDeathScreen(player);
        return;
    }

    // Get texture manager and renderer
    TextureManager* textures = GetTextureManager();
    Renderer* renderer = GetRenderer();
    if (!textures || !renderer) return;

    // Calculate source rectangle based on animation state
    int sourceX = 0;
    int sourceY = 0;

    // Determine source coordinates based on animation
    switch (playerData->currentAnim) {
    case ANIM_IDLE_DOWN:
        sourceX = 0;
        sourceY = 0;
        break;
    case ANIM_IDLE_LEFT:
        sourceX = 1;
        sourceY = 0;
        break;
    case ANIM_IDLE_UP:
        sourceX = 2;
        sourceY = 0;
        break;
    case ANIM_IDLE_RIGHT:
        sourceX = 3;
        sourceY = 0;
        break;
    case ANIM_WALK_DOWN:
        sourceX = playerData->currentFrame;
        sourceY = 1;
        break;
    case ANIM_WALK_UP:
        sourceX = playerData->currentFrame;
        sourceY = 2;
        break;
    case ANIM_WALK_LEFT:
        sourceX = playerData->currentFrame;
        sourceY = 3;
        break;
    case ANIM_WALK_RIGHT:
        sourceX = playerData->currentFrame;
        sourceY = 4;
        break;
    case ANIM_DEATH:
        sourceX = playerData->currentFrame;
        sourceY = 5;
        break;
    default:
        sourceX = 0;
        sourceY = 0;
        break;
    }

    // Create source rectangle
    Rectangle source = {
        (float)(sourceX * SPRITE_WIDTH),
        (float)(sourceY * SPRITE_HEIGHT),
        (float)SPRITE_WIDTH,
        (float)SPRITE_HEIGHT
    };

    // Create destination rectangle
    Rectangle dest = {
        player->x - SPRITE_WIDTH / 2,
        player->y - SPRITE_HEIGHT / 2,
        (float)SPRITE_WIDTH,
        (float)SPRITE_HEIGHT
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

    // Reset player data
    PlayerData* playerData = (PlayerData*)player->typeData;
    if (playerData) {
        playerData->currentAnim = ANIM_IDLE_DOWN;
        playerData->frameCounter = 0;
        playerData->currentFrame = 0;
        playerData->state = PLAYER_STATE_ALIVE;
        playerData->deathTimer = 0.0f;
        playerData->currentHealth = playerData->maxHealth;
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

/**
* @brief Handle player death
*
* @param player Pointer to player entity
* @param deltaTime Time elapsed since last update
* @return true If death sequence is complete
* @return false If death sequence is still in progress
*/
bool PlayerHandleDeath(Entity* player, float deltaTime) {
    if (!player || player->type != ENTITY_PLAYER) return false;

    PlayerData* playerData = PlayerGetData(player);
    if (!playerData) return false;

    // Update death timer
    playerData->deathTimer += deltaTime;

    // Handle death animation first
    if (playerData->state == PLAYER_STATE_DYING) {
        // Calculate the frame based on death animation duration
        float frameDuration = PLAYER_DEATH_DURATION / PLAYER_DEATH_FRAMES;
        int frameIndex = (int)(playerData->deathTimer / frameDuration);

        // Update the current frame
        if (frameIndex < PLAYER_DEATH_FRAMES) {
            playerData->currentFrame = frameIndex;
        }
        else {
            // Animation finished, transition to death screen
            playerData->state = PLAYER_STATE_DEAD;
        }

        return false; // Death sequence not complete
    }

    // Handle death screen
    if (playerData->state == PLAYER_STATE_DEAD) {
        // Check if death screen duration has elapsed
        if (playerData->deathTimer >= PLAYER_DEATH_DURATION + DEATH_SCREEN_DURATION) {
            return true; // Death sequence is complete
        }

        // Check for key press to skip death screen
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
            IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return true; // Skip death screen on input
        }
    }

    return false; // Death sequence still in progress
}

/**
* @brief Render the death screen
*
* @param player Pointer to player entity
*/
void PlayerRenderDeathScreen(Entity* player) {
    if (!player || player->type != ENTITY_PLAYER) return;

    PlayerData* playerData = PlayerGetData(player);
    if (!playerData || playerData->state != PLAYER_STATE_DEAD) return;

    // Calculate screen fade alpha (fade in over time)
    float elapsedTime = playerData->deathTimer - PLAYER_DEATH_DURATION;
    float fadeInDuration = 1.0f; // Fade in over 1 second
    float alpha = fminf(1.0f, elapsedTime / fadeInDuration);

    // Draw full-screen black rectangle with fade
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ColorAlpha(BLACK, alpha));

    // Only show text after fade is complete
    if (alpha >= 0.9f) {
        // Draw "YOU DIED!" text
        const char* text = "YOU DIED!";
        int fontSize = DEATH_TEXT_SIZE;
        int textWidth = MeasureText(text, fontSize);
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        // Calculate position (center of screen)
        int textX = (screenWidth - textWidth) / 2;
        int textY = screenHeight / 2 - fontSize / 2;

        // Calculate text alpha (fade in)
        float textFadeInDuration = 0.5f;
        float textAlpha = fminf(1.0f, (elapsedTime - fadeInDuration) / textFadeInDuration);

        // Draw text with fade-in effect
        DrawText(text, textX, textY, fontSize, ColorAlpha(DEATH_TEXT_COLOR, textAlpha));

        // Draw instruction text
        const char* instructionText = "Press any key to restart";
        int instructionFontSize = fontSize / 2;
        int instructionWidth = MeasureText(instructionText, instructionFontSize);
        int instructionX = (screenWidth - instructionWidth) / 2;
        int instructionY = textY + fontSize + 20;

        DrawText(instructionText, instructionX, instructionY, instructionFontSize,
            ColorAlpha(WHITE, textAlpha));
    }
}