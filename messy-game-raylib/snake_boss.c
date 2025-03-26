/**
* @file snake_boss.c
* @brief Implementation of snake boss enemy
*/
#include <stdlib.h>
#include <math.h>
#include "snake_boss.h"
#include "config.h"
#include "player.h"

// Constants specific to the snake boss
#define SNAKE_INITIAL_MOVE_INTERVAL 1.0f   // Initial time between moves in seconds
#define SNAKE_MIN_MOVE_INTERVAL 0.3f       // Minimum time between moves (fastest speed)
#define SNAKE_INTERVAL_DECREASE 0.05f      // How much to decrease interval per segment
#define SNAKE_GROW_TIME 0.5f               // Time for growth animation
#define SNAKE_SHRINK_TIME 0.5f             // Time for shrink animation
#define SNAKE_HEAD_RADIUS 5.0f             // Radius of the snake head

// Forward declarations for internal helper functions
static bool CheckDirectionValidity(Entity* snakeBoss, Direction direction, World* world);
static Direction FindAnyValidDirection(Entity* snakeBoss, World* world, Direction oppositeDir);

/**
* @brief Check if moving in a given direction is valid
*
* @param snakeBoss Pointer to snake boss entity
* @param direction Direction to check
* @param world Pointer to game world
* @return true If direction is valid
* @return false If direction is invalid
*/
static bool CheckDirectionValidity(Entity* snakeBoss, Direction direction, World* world) {
    if (!snakeBoss || !world) return false;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData || bossData->segmentCount <= 0) return false;

    // Get head position
    int headGridX = bossData->segments[0].gridX;
    int headGridY = bossData->segments[0].gridY;

    // Calculate new position based on direction
    int newGridX = headGridX;
    int newGridY = headGridY;

    switch (direction) {
    case DIRECTION_UP:
        newGridY--;
        break;
    case DIRECTION_DOWN:
        newGridY++;
        break;
    case DIRECTION_LEFT:
        newGridX--;
        break;
    case DIRECTION_RIGHT:
        newGridX++;
        break;
    }

    // Check if position is valid
    return SnakeBossIsValidPosition(snakeBoss, newGridX, newGridY, world);
}

/**
* @brief Find any valid direction to move
*
* @param snakeBoss Pointer to snake boss entity
* @param world Pointer to game world
* @param oppositeDir Direction to avoid (opposite of current)
* @return Direction Valid direction to move or current direction if none found
*/
static Direction FindAnyValidDirection(Entity* snakeBoss, World* world, Direction oppositeDir) {
    if (!snakeBoss || !world) return DIRECTION_RIGHT;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData) return DIRECTION_RIGHT;

    // Try each direction in order: up, right, down, left
    Direction directions[4] = { DIRECTION_UP, DIRECTION_RIGHT, DIRECTION_DOWN, DIRECTION_LEFT };

    for (int i = 0; i < 4; i++) {
        // Skip opposite direction
        if (directions[i] == oppositeDir) continue;

        // Check if direction is valid
        if (CheckDirectionValidity(snakeBoss, directions[i], world)) {
            return directions[i];
        }
    }

    // If no valid direction found, return current direction (snake will likely hit a wall)
    return bossData->currentDir;
}

/**
* @brief Create a new snake boss entity
*
* @param gridX Initial X position in grid coordinates
* @param gridY Initial Y position in grid coordinates
* @param initialLength Initial number of segments
* @return Entity* Pointer to the created snake boss entity
*/
Entity* SnakeBossCreate(int gridX, int gridY, int initialLength) {
    // Calculate world coordinates from grid coordinates
    float worldX = gridX * TILE_WIDTH + TILE_WIDTH / 2.0f;
    float worldY = gridY * TILE_HEIGHT + TILE_HEIGHT / 2.0f;

    // Create base entity
    Entity* snakeBoss = EntityCreate(ENTITY_ENEMY, worldX, worldY, TILE_WIDTH, TILE_HEIGHT);
    if (!snakeBoss) {
        TraceLog(LOG_ERROR, "Failed to create snake boss entity");
        return NULL;
    }

    // Create snake boss-specific data
    SnakeBossData* bossData = (SnakeBossData*)malloc(sizeof(SnakeBossData));
    if (!bossData) {
        TraceLog(LOG_ERROR, "Failed to allocate snake boss data");
        EntityDestroy(snakeBoss);
        return NULL;
    }

    // Initialize snake boss data
    bossData->state = SNAKE_STATE_IDLE;
    bossData->currentDir = DIRECTION_RIGHT;
    bossData->nextDir = DIRECTION_RIGHT;
    bossData->moveTimer = 0.0f;
    bossData->moveInterval = SNAKE_INITIAL_MOVE_INTERVAL;
    bossData->growTimer = 0.0f;
    bossData->shrinkTimer = 0.0f;
    bossData->hasTarget = false;
    bossData->targetGridX = gridX;
    bossData->targetGridY = gridY;

    // Set colors
    bossData->headColor = ORANGE;
    bossData->bodyColor = (Color){ 255, 140, 0, 255 }; // Dark orange

    // Allocate memory for segments
    int initialCapacity = initialLength > 0 ? initialLength : 1;
    bossData->segmentCapacity = initialCapacity;
    bossData->segments = (SnakeSegment*)malloc(sizeof(SnakeSegment) * initialCapacity);

    if (!bossData->segments) {
        TraceLog(LOG_ERROR, "Failed to allocate snake segments");
        free(bossData);
        EntityDestroy(snakeBoss);
        return NULL;
    }

    // Initialize segments
    bossData->segmentCount = initialLength;
    for (int i = 0; i < initialLength; i++) {
        // Position segments in a row to the left of the head
        bossData->segments[i].gridX = gridX - i;
        bossData->segments[i].gridY = gridY;

        // Convert to world coordinates
        bossData->segments[i].worldX = bossData->segments[i].gridX * TILE_WIDTH + TILE_WIDTH / 2.0f;
        bossData->segments[i].worldY = bossData->segments[i].gridY * TILE_HEIGHT + TILE_HEIGHT / 2.0f;
    }

    // Attach snake data to entity
    snakeBoss->typeData = bossData;

    return snakeBoss;
}

/**
* @brief Update snake boss state
*
* @param snakeBoss Pointer to snake boss entity
* @param world Pointer to game world
* @param ball Pointer to ball entity
* @param player Pointer to player entity
* @param deltaTime Time elapsed since last update
*/
void SnakeBossUpdate(Entity* snakeBoss, World* world, Entity* ball, Entity* player, float deltaTime) {
    if (!snakeBoss || !world || !ball || snakeBoss->type != ENTITY_ENEMY) return;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData) return;

    // Only proceed if snake has segments
    if (bossData->segmentCount <= 0) return;

    // Debug info
    if (bossData->state != SNAKE_STATE_DEFEATED) {
        TraceLog(LOG_DEBUG, "Snake Boss - gridX: %d, gridY: %d, State: %d, Target: %d,%d, HasTarget: %d",
            bossData->segments[0].gridX, bossData->segments[0].gridY,
            bossData->state, bossData->targetGridX, bossData->targetGridY, bossData->hasTarget);
    }

    // Handle state-specific updates
    switch (bossData->state) {
    case SNAKE_STATE_IDLE:
        // Transition to tracking state
        bossData->state = SNAKE_STATE_TRACKING;
        bossData->hasTarget = false;  // Force target recalculation
        break;

    case SNAKE_STATE_TRACKING:
        // Always update the target position to follow the ball more aggressively
        // Convert ball position to grid coordinates
        int ballGridX = (int)(ball->x / TILE_WIDTH);
        int ballGridY = (int)(ball->y / TILE_HEIGHT);

        // Set new target
        bossData->targetGridX = ballGridX;
        bossData->targetGridY = ballGridY;
        bossData->hasTarget = true;

        // Find path to target
        SnakeBossFindPath(snakeBoss, ballGridX, ballGridY, world);

        // Transition to moving state
        bossData->state = SNAKE_STATE_MOVING;
        break;

    case SNAKE_STATE_MOVING:
        // Update move timer
        bossData->moveTimer += deltaTime;

        // Move when timer exceeds interval
        if (bossData->moveTimer >= bossData->moveInterval) {
            static int moveCount = 0;  // Moved declaration to beginning of block

            bossData->moveTimer = 0.0f;

            // Update direction for next move
            bossData->currentDir = bossData->nextDir;

            // Move snake
            bool moved = SnakeBossMove(snakeBoss, world);

            // Check if we've reached the target
            int headGridX = bossData->segments[0].gridX;
            int headGridY = bossData->segments[0].gridY;

            if (headGridX == bossData->targetGridX && headGridY == bossData->targetGridY) {
                // Target reached, go back to tracking for a new target
                bossData->hasTarget = false;
                bossData->state = SNAKE_STATE_TRACKING;
            }
            else if (!moved) {
                // Move failed, go back to tracking to find a new path
                bossData->state = SNAKE_STATE_TRACKING;
                bossData->hasTarget = false;
            }

            // Recalculate path every few moves to better track the ball
            moveCount++;
            if (moveCount >= 3) {  // Recalculate every 3 moves
                int newBallGridX = (int)(ball->x / TILE_WIDTH);
                int newBallGridY = (int)(ball->y / TILE_HEIGHT);

                moveCount = 0;

                // If ball has moved, update target
                if (newBallGridX != bossData->targetGridX || newBallGridY != bossData->targetGridY) {
                    bossData->targetGridX = newBallGridX;
                    bossData->targetGridY = newBallGridY;
                    SnakeBossFindPath(snakeBoss, newBallGridX, newBallGridY, world);
                }
            }
        }
        break;

    case SNAKE_STATE_GROWING:
        // Handle growth animation
        bossData->growTimer += deltaTime;
        if (bossData->growTimer >= SNAKE_GROW_TIME) {
            bossData->growTimer = 0.0f;
            bossData->state = SNAKE_STATE_TRACKING;
            bossData->hasTarget = false;  // Force target recalculation
        }
        break;

    case SNAKE_STATE_SHRINKING:
        // Handle shrinking animation
        bossData->shrinkTimer += deltaTime;
        if (bossData->shrinkTimer >= SNAKE_SHRINK_TIME) {
            bossData->shrinkTimer = 0.0f;
            bossData->state = SNAKE_STATE_TRACKING;
            bossData->hasTarget = false;  // Force target recalculation
        }
        break;

    case SNAKE_STATE_DEFEATED:
        // Nothing to do in defeated state
        break;
    }

    // Update entity position to match head segment
    if (bossData->segmentCount > 0) {
        snakeBoss->x = bossData->segments[0].worldX;
        snakeBoss->y = bossData->segments[0].worldY;
    }

    // Check for collisions
    SnakeBossHandleBallCollision(snakeBoss, ball);
    SnakeBossHandlePlayerCollision(snakeBoss, player);
}

/**
* @brief Find path to target for the snake boss
*
* @param snakeBoss Pointer to snake boss entity
* @param targetGridX Target X position in grid coordinates
* @param targetGridY Target Y position in grid coordinates
* @param world Pointer to game world
*/
void SnakeBossFindPath(Entity* snakeBoss, int targetGridX, int targetGridY, World* world) {
    if (!snakeBoss || !world) return;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData || bossData->segmentCount <= 0) return;

    // Get head position
    int headGridX = bossData->segments[0].gridX;
    int headGridY = bossData->segments[0].gridY;

    TraceLog(LOG_DEBUG, "Snake finding path from (%d,%d) to (%d,%d)",
        headGridX, headGridY, targetGridX, targetGridY);

    // Calculate direction to target (simple direct approach - not A*)
    int dx = targetGridX - headGridX;
    int dy = targetGridY - headGridY;

    // Determine the preferred direction
    Direction preferredDir = DIRECTION_RIGHT;  // Default

    // Choose direction based on which axis has the greater distance
    if (abs(dx) > abs(dy)) {
        // Prefer horizontal movement
        preferredDir = (dx > 0) ? DIRECTION_RIGHT : DIRECTION_LEFT;
    }
    else {
        // Prefer vertical movement
        preferredDir = (dy > 0) ? DIRECTION_DOWN : DIRECTION_UP;
    }

    // Get opposite of current direction (to avoid reversing)
    Direction oppositeDir;
    switch (bossData->currentDir) {
    case DIRECTION_UP: oppositeDir = DIRECTION_DOWN; break;
    case DIRECTION_DOWN: oppositeDir = DIRECTION_UP; break;
    case DIRECTION_LEFT: oppositeDir = DIRECTION_RIGHT; break;
    case DIRECTION_RIGHT: oppositeDir = DIRECTION_LEFT; break;
    default: oppositeDir = DIRECTION_DOWN; break;
    }

    // First, try the preferred direction
    if (preferredDir != oppositeDir && CheckDirectionValidity(snakeBoss, preferredDir, world)) {
        bossData->nextDir = preferredDir;
        TraceLog(LOG_DEBUG, "Snake chose preferred direction: %d", preferredDir);
        return;
    }

    // If preferred direction is invalid or would go backwards, try the other axis
    Direction secondaryDir;
    if (preferredDir == DIRECTION_RIGHT || preferredDir == DIRECTION_LEFT) {
        // If horizontal didn't work, try vertical
        secondaryDir = (dy > 0) ? DIRECTION_DOWN : DIRECTION_UP;
    }
    else {
        // If vertical didn't work, try horizontal
        secondaryDir = (dx > 0) ? DIRECTION_RIGHT : DIRECTION_LEFT;
    }

    if (secondaryDir != oppositeDir && CheckDirectionValidity(snakeBoss, secondaryDir, world)) {
        bossData->nextDir = secondaryDir;
        TraceLog(LOG_DEBUG, "Snake chose secondary direction: %d", secondaryDir);
        return;
    }

    // If both preferred directions failed, try any valid direction
    Direction anyDir = FindAnyValidDirection(snakeBoss, world, oppositeDir);
    bossData->nextDir = anyDir;
    TraceLog(LOG_DEBUG, "Snake chose any valid direction: %d", anyDir);
}

// These functions are now defined at the top of the file as static functions
// The code has been removed from here to prevent redefinition errors

/**
* @brief Move the snake boss one step in its current direction
*
* @param snakeBoss Pointer to snake boss entity
* @param world Pointer to game world
* @return true If move was successful
* @return false If move failed (e.g., wall collision)
*/
bool SnakeBossMove(Entity* snakeBoss, World* world) {
    if (!snakeBoss || !world) return false;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData || bossData->segmentCount <= 0) return false;

    // Get head position
    int headGridX = bossData->segments[0].gridX;
    int headGridY = bossData->segments[0].gridY;

    // Calculate new position based on direction
    int newGridX = headGridX;
    int newGridY = headGridY;

    switch (bossData->currentDir) {
    case DIRECTION_UP:
        newGridY--;
        break;
    case DIRECTION_DOWN:
        newGridY++;
        break;
    case DIRECTION_LEFT:
        newGridX--;
        break;
    case DIRECTION_RIGHT:
        newGridX++;
        break;
    }

    // Check if new position is valid
    if (!SnakeBossIsValidPosition(snakeBoss, newGridX, newGridY, world)) {
        return false;
    }

    // Store the tail's position for potential growth
    int tailGridX = bossData->segments[bossData->segmentCount - 1].gridX;
    int tailGridY = bossData->segments[bossData->segmentCount - 1].gridY;

    // Move all segments (except head) to position of segment in front of them
    for (int i = bossData->segmentCount - 1; i > 0; i--) {
        bossData->segments[i].gridX = bossData->segments[i - 1].gridX;
        bossData->segments[i].gridY = bossData->segments[i - 1].gridY;
    }

    // Move head to new position
    bossData->segments[0].gridX = newGridX;
    bossData->segments[0].gridY = newGridY;

    // Update world coordinates for all segments
    SnakeBossUpdateSegments(snakeBoss);

    return true;
}

/**
* @brief Update all segment positions
*
* @param snakeBoss Pointer to snake boss entity
*/
void SnakeBossUpdateSegments(Entity* snakeBoss) {
    if (!snakeBoss) return;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData) return;

    // Update world coordinates for all segments
    for (int i = 0; i < bossData->segmentCount; i++) {
        bossData->segments[i].worldX = bossData->segments[i].gridX * TILE_WIDTH + TILE_WIDTH / 2.0f;
        bossData->segments[i].worldY = bossData->segments[i].gridY * TILE_HEIGHT + TILE_HEIGHT / 2.0f;
    }

    // Update entity position to match head
    if (bossData->segmentCount > 0) {
        snakeBoss->x = bossData->segments[0].worldX;
        snakeBoss->y = bossData->segments[0].worldY;
    }
}

/**
* @brief Check if a position is valid for the snake to move to
*
* @param snakeBoss Pointer to snake boss entity
* @param gridX X position in grid coordinates
* @param gridY Y position in grid coordinates
* @param world Pointer to game world
* @return true If position is valid
* @return false If position is invalid (wall or self-collision)
*/
bool SnakeBossIsValidPosition(Entity* snakeBoss, int gridX, int gridY, World* world) {
    if (!snakeBoss || !world) return false;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData) return false;

    // Convert grid coordinates to world coordinates
    float worldX = gridX * TILE_WIDTH + TILE_WIDTH / 2.0f;
    float worldY = gridY * TILE_HEIGHT + TILE_HEIGHT / 2.0f;

    // Check if position has a wall
    if (WorldIsWallAtPosition(world, worldX, worldY)) {
        return false;
    }

    // Check for self-collision (skip head)
    for (int i = 1; i < bossData->segmentCount; i++) {
        if (bossData->segments[i].gridX == gridX && bossData->segments[i].gridY == gridY) {
            return false;
        }
    }

    return true;
}

/**
* @brief Handle snake boss collision with the ball
*
* @param snakeBoss Pointer to snake boss entity
* @param ball Pointer to ball entity
* @return true If collision occurred
* @return false If no collision
*/
bool SnakeBossHandleBallCollision(Entity* snakeBoss, Entity* ball) {
    if (!snakeBoss || !ball || snakeBoss->type != ENTITY_ENEMY || ball->type != ENTITY_BALL) return false;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData || bossData->segmentCount <= 0) return false;

    BallData* ballData = BallGetData(ball);
    if (!ballData) return false;

    // Check if ball is moving fast (indicating it was hit by player)
    bool ballIsProjectile = (fabs(ball->speedX) > BALL_INITIAL_SPEED * 1.5f ||
        fabs(ball->speedY) > BALL_INITIAL_SPEED * 1.5f);

    // Check collision with head
    float headX = bossData->segments[0].worldX;
    float headY = bossData->segments[0].worldY;
    float distance = sqrtf((ball->x - headX) * (ball->x - headX) +
        (ball->y - headY) * (ball->y - headY));

    if (distance < SNAKE_HEAD_RADIUS + ballData->radius) {
        // Collision with head
        if (ballIsProjectile) {
            // Ball was hit by player - shrink snake
            if (bossData->state != SNAKE_STATE_SHRINKING && bossData->state != SNAKE_STATE_DEFEATED) {
                bossData->state = SNAKE_STATE_SHRINKING;
                bossData->shrinkTimer = 0.0f;

                // Shrink snake
                if (!SnakeBossShrink(snakeBoss)) {
                    // Snake is defeated
                    bossData->state = SNAKE_STATE_DEFEATED;
                }

                // Bounce ball
                BallApplyForce(ball, (ball->x - headX) * 0.5f, (ball->y - headY) * 0.5f);
            }
        }
        else {
            // Normal collision - grow snake
            if (bossData->state != SNAKE_STATE_GROWING) {
                bossData->state = SNAKE_STATE_GROWING;
                bossData->growTimer = 0.0f;

                // Grow snake
                SnakeBossGrow(snakeBoss);

                // Kick ball away
                float kickForce = 6.0f;
                BallApplyForce(ball, (ball->x - headX) * kickForce, (ball->y - headY) * kickForce);
            }
        }

        return true;
    }

    // Check collision with body segments (only if ball is a projectile)
    if (ballIsProjectile) {
        for (int i = 1; i < bossData->segmentCount; i++) {
            float segX = bossData->segments[i].worldX;
            float segY = bossData->segments[i].worldY;

            // Create a collision rectangle for the segment
            Rectangle segRect = {
                segX - TILE_WIDTH / 2.0f,
                segY - TILE_HEIGHT / 2.0f,
                TILE_WIDTH,
                TILE_HEIGHT
            };

            // Check circle-rectangle collision
            if (CheckCollisionCircleRec((Vector2) { ball->x, ball->y }, ballData->radius, segRect)) {
                // Collision with body segment - shrink snake
                if (bossData->state != SNAKE_STATE_SHRINKING && bossData->state != SNAKE_STATE_DEFEATED) {
                    bossData->state = SNAKE_STATE_SHRINKING;
                    bossData->shrinkTimer = 0.0f;

                    // Shrink snake
                    if (!SnakeBossShrink(snakeBoss)) {
                        // Snake is defeated
                        bossData->state = SNAKE_STATE_DEFEATED;
                    }

                    // Bounce ball
                    BallApplyForce(ball, (ball->x - segX) * 0.5f, (ball->y - segY) * 0.5f);
                }

                return true;
            }
        }
    }

    return false;
}

/**
* @brief Handle snake boss collision with the player
*
* @param snakeBoss Pointer to snake boss entity
* @param player Pointer to player entity
* @return true If collision occurred
* @return false If no collision
*/
bool SnakeBossHandlePlayerCollision(Entity* snakeBoss, Entity* player) {
    if (!snakeBoss || !player || snakeBoss->type != ENTITY_ENEMY || player->type != ENTITY_PLAYER) return false;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData || bossData->segmentCount <= 0) return false;

    PlayerData* playerData = PlayerGetData(player);
    if (!playerData) return false;

    // Check collision with head
    float headX = bossData->segments[0].worldX;
    float headY = bossData->segments[0].worldY;
    float distance = sqrtf((player->x - headX) * (player->x - headX) +
        (player->y - headY) * (player->y - headY));

    float collisionRadius = (player->width + player->height) / 4.0f;

    if (distance < SNAKE_HEAD_RADIUS + collisionRadius) {
        // Damage player
        playerData->currentHealth -= 10.0f;
        if (playerData->currentHealth < 0) playerData->currentHealth = 0;

        // Push player away
        float pushForce = 5.0f;
        player->speedX += (player->x - headX) * pushForce;
        player->speedY += (player->y - headY) * pushForce;

        return true;
    }

    // Check collision with body segments
    for (int i = 1; i < bossData->segmentCount; i++) {
        float segX = bossData->segments[i].worldX;
        float segY = bossData->segments[i].worldY;

        // Create a collision rectangle for the segment
        Rectangle segRect = {
            segX - TILE_WIDTH / 2.0f,
            segY - TILE_HEIGHT / 2.0f,
            TILE_WIDTH,
            TILE_HEIGHT
        };

        // Create a collision circle for the player
        Rectangle playerRect = {
            player->x - player->width / 2.0f,
            player->y - player->height / 2.0f,
            player->width,
            player->height
        };

        // Check rectangle-rectangle collision
        if (CheckCollisionRecs(segRect, playerRect)) {
            // Damage player
            playerData->currentHealth -= 5.0f;
            if (playerData->currentHealth < 0) playerData->currentHealth = 0;

            // Push player away
            float pushForce = 3.0f;
            player->speedX += (player->x - segX) * pushForce;
            player->speedY += (player->y - segY) * pushForce;

            return true;
        }
    }

    return false;
}

/**
* @brief Make the snake boss grow by one segment
*
* @param snakeBoss Pointer to snake boss entity
*/
void SnakeBossGrow(Entity* snakeBoss) {
    if (!snakeBoss) return;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData) return;

    // Check if we need to expand the segments array
    if (bossData->segmentCount >= bossData->segmentCapacity) {
        // Create a completely new array with increased capacity
        int newCapacity = bossData->segmentCapacity * 2;
        SnakeSegment* newSegments = (SnakeSegment*)malloc(sizeof(SnakeSegment) * newCapacity);

        if (!newSegments) {
            TraceLog(LOG_ERROR, "Failed to allocate new snake segments array");
            return;
        }

        // Copy existing segments to new array
        if (bossData->segments) {
            for (int i = 0; i < bossData->segmentCount; i++) {
                newSegments[i] = bossData->segments[i];
            }

            // Free old array
            free(bossData->segments);
        }

        // Update pointers and capacity
        bossData->segments = newSegments;
        bossData->segmentCapacity = newCapacity;
    }

    // Add a new segment at the end, copying the position of the last segment
    if (bossData->segmentCount > 0) {
        int lastIndex = bossData->segmentCount - 1;
        int newIndex = bossData->segmentCount;

        // Explicitly initialize the new segment
        bossData->segments[newIndex].gridX = bossData->segments[lastIndex].gridX;
        bossData->segments[newIndex].gridY = bossData->segments[lastIndex].gridY;
        bossData->segments[newIndex].worldX = bossData->segments[lastIndex].worldX;
        bossData->segments[newIndex].worldY = bossData->segments[lastIndex].worldY;

        bossData->segmentCount++;

        // Make the snake faster as it grows
        bossData->moveInterval = fmaxf(SNAKE_MIN_MOVE_INTERVAL,
            bossData->moveInterval - SNAKE_INTERVAL_DECREASE);
    }
}

/**
* @brief Make the snake boss shrink by one segment
*
* @param snakeBoss Pointer to snake boss entity
* @return true If snake still has segments left
* @return false If snake is defeated (only head remains and it's hit)
*/
bool SnakeBossShrink(Entity* snakeBoss) {
    if (!snakeBoss) return false;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData) return false;

    // Check if only the head remains
    if (bossData->segmentCount <= 1) {
        return false; // Snake is defeated
    }

    // Remove the last segment
    bossData->segmentCount--;

    // Make the snake slower as it shrinks
    bossData->moveInterval = fminf(SNAKE_INITIAL_MOVE_INTERVAL,
        bossData->moveInterval + SNAKE_INTERVAL_DECREASE);

    return true;
}

/**
* @brief Render snake boss
*
* @param snakeBoss Pointer to snake boss entity
*/
void SnakeBossRender(Entity* snakeBoss) {
    if (!snakeBoss || snakeBoss->type != ENTITY_ENEMY) return;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData) return;

    // Only render if snake has segments
    if (bossData->segmentCount <= 0) return;

    // Render body segments first (in reverse order so head appears on top)
    for (int i = bossData->segmentCount - 1; i > 0; i--) {
        float segX = bossData->segments[i].worldX;
        float segY = bossData->segments[i].worldY;

        DrawRectangle(
            (int)(segX - TILE_WIDTH / 2.0f),
            (int)(segY - TILE_HEIGHT / 2.0f),
            TILE_WIDTH,
            TILE_HEIGHT,
            bossData->bodyColor
        );
    }

    // Render head (circle)
    float headX = bossData->segments[0].worldX;
    float headY = bossData->segments[0].worldY;

    DrawCircle(
        (int)headX,
        (int)headY,
        SNAKE_HEAD_RADIUS,
        bossData->headColor
    );

    // If in growing state, add a visual effect
    if (bossData->state == SNAKE_STATE_GROWING) {
        float growthProgress = bossData->growTimer / SNAKE_GROW_TIME;
        float effectSize = 10.0f * (1.0f - growthProgress);

        DrawCircleLines(
            (int)headX,
            (int)headY,
            SNAKE_HEAD_RADIUS + effectSize,
            Fade(GREEN, 0.7f * (1.0f - growthProgress))
        );
    }

    // If in shrinking state, add a visual effect
    if (bossData->state == SNAKE_STATE_SHRINKING) {
        float shrinkProgress = bossData->shrinkTimer / SNAKE_SHRINK_TIME;
        float effectSize = 10.0f * (1.0f - shrinkProgress);

        DrawCircleLines(
            (int)headX,
            (int)headY,
            SNAKE_HEAD_RADIUS + effectSize,
            Fade(RED, 0.7f * (1.0f - shrinkProgress))
        );
    }

    // If defeated, show special effect
    if (bossData->state == SNAKE_STATE_DEFEATED) {
        DrawCircleLines(
            (int)headX,
            (int)headY,
            SNAKE_HEAD_RADIUS * 2.0f,
            RED
        );

        DrawCircleLines(
            (int)headX,
            (int)headY,
            SNAKE_HEAD_RADIUS * 1.5f,
            YELLOW
        );
    }
}

/**
* @brief Get snake boss-specific data from entity
*
* @param snakeBoss Pointer to snake boss entity
* @return SnakeBossData* Pointer to snake boss data
*/
SnakeBossData* SnakeBossGetData(Entity* snakeBoss) {
    if (!snakeBoss || snakeBoss->type != ENTITY_ENEMY) return NULL;

    return (SnakeBossData*)snakeBoss->typeData;
}

/**
* @brief Check if entity is a snake boss
*
* Helper function to identify snake boss entities since they use ENTITY_ENEMY type
*
* @param entity Pointer to entity
* @return true If entity is a snake boss
* @return false If entity is not a snake boss
*/
bool IsSnakeBoss(Entity* entity) {
    if (!entity || entity->type != ENTITY_ENEMY) return false;

    // Try to get snake boss data - if it exists, this is a snake boss
    SnakeBossData* bossData = SnakeBossGetData(entity);
    return bossData != NULL;
}