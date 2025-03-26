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
#define SNAKE_INITIAL_MOVE_INTERVAL 0.2f   // Initial time between moves in seconds
#define SNAKE_MIN_MOVE_INTERVAL 0.05f       // Minimum time between moves (fastest speed)
#define SNAKE_INTERVAL_DECREASE 0.05f      // How much to decrease interval per segment
#define SNAKE_GROW_TIME 2.0f               // Time for growth animation
#define SNAKE_SHRINK_TIME 2.0f             // Time for shrink animation
#define SNAKE_HEAD_RADIUS 6.0f             // Radius of the snake head

// Snake appearance configuration
#define SNAKE_SEGMENT_WIDTH_TILES 2       // Width of snake segment in tiles (default: 2)
#define SNAKE_SEGMENT_HEIGHT_TILES 2      // Height of snake segment in tiles (default: 2)
#define SNAKE_HEAD_RADIUS_FACTOR 1.5f     // Head radius as a factor of segment size (default: 1.5)

// Derived size calculations - do not modify these directly
#define SNAKE_SEGMENT_WIDTH (TILE_WIDTH * SNAKE_SEGMENT_WIDTH_TILES)
#define SNAKE_SEGMENT_HEIGHT (TILE_HEIGHT * SNAKE_SEGMENT_HEIGHT_TILES)
#define SNAKE_HEAD_RADIUS ((SNAKE_SEGMENT_WIDTH + SNAKE_SEGMENT_HEIGHT) / 4.0f * SNAKE_HEAD_RADIUS_FACTOR)

// Function prototypes for helper functions
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

    // Print debug info
    TraceLog(LOG_INFO, "Creating snake boss at grid (%d,%d), world (%.1f,%.1f)",
        gridX, gridY, worldX, worldY);

    // Create base entity
    Entity* snakeBoss = EntityCreate(ENTITY_ENEMY, worldX, worldY, SNAKE_SEGMENT_WIDTH, SNAKE_SEGMENT_HEIGHT);
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
    bossData->currentDir = DIRECTION_RIGHT; // Start moving right instead of down
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

    // Initialize segments HORIZONTALLY (not vertically)
    bossData->segmentCount = initialLength;
    for (int i = 0; i < initialLength; i++) {
        // Position segments in a row to the left of the head
        bossData->segments[i].gridX = gridX - i;
        bossData->segments[i].gridY = gridY;

        // Convert to world coordinates
        bossData->segments[i].worldX = bossData->segments[i].gridX * TILE_WIDTH + TILE_WIDTH / 2.0f;
        bossData->segments[i].worldY = bossData->segments[i].gridY * TILE_HEIGHT + TILE_HEIGHT / 2.0f;

        TraceLog(LOG_DEBUG, "Snake segment %d: grid (%d,%d), world (%.1f,%.1f)",
            i, bossData->segments[i].gridX, bossData->segments[i].gridY,
            bossData->segments[i].worldX, bossData->segments[i].worldY);
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
    {
        // Get initial ball position immediately
        int ballGridX = (int)(ball->x / TILE_WIDTH);
        int ballGridY = (int)(ball->y / TILE_HEIGHT);

        // Set target to ball position
        bossData->targetGridX = ballGridX;
        bossData->targetGridY = ballGridY;
        bossData->hasTarget = true;

        // Calculate initial path to ball
        SnakeBossFindPath(snakeBoss, ballGridX, ballGridY, world);

        // Transition to moving state directly (skip tracking)
        bossData->state = SNAKE_STATE_MOVING;

        // Add debug output
        TraceLog(LOG_DEBUG, "Snake IDLE: Initial target set to ball at (%d,%d)",
            ballGridX, ballGridY);
    }
    break;

    case SNAKE_STATE_TRACKING:
    {
        // Always update target position with current ball position
        int ballGridX = (int)(ball->x / TILE_WIDTH);
        int ballGridY = (int)(ball->y / TILE_HEIGHT);

        // Set new target
        bossData->targetGridX = ballGridX;
        bossData->targetGridY = ballGridY;
        bossData->hasTarget = true;

        // Log tracking info
        TraceLog(LOG_INFO, "Snake tracking: Current pos=(%d,%d), Ball pos=(%d,%d)",
            bossData->segments[0].gridX, bossData->segments[0].gridY, ballGridX, ballGridY);

        // Find path to target with more aggressive parameters
        SnakeBossFindPath(snakeBoss, ballGridX, ballGridY, world);

        // Transition to moving state immediately
        bossData->state = SNAKE_STATE_MOVING;
    }
    break;

    case SNAKE_STATE_MOVING:
    {
        // Update move timer
        bossData->moveTimer += deltaTime;

        // Move when timer exceeds interval
        if (bossData->moveTimer >= bossData->moveInterval) {
            static int moveCount = 0;
            int headGridX, headGridY;
            int newBallGridX, newBallGridY;
            bool moved;

            bossData->moveTimer = 0.0f;

            // Update direction for next move
            bossData->currentDir = bossData->nextDir;

            // Move snake
            moved = SnakeBossMove(snakeBoss, world);

            // Check if we've reached the target
            headGridX = bossData->segments[0].gridX;
            headGridY = bossData->segments[0].gridY;

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
                moveCount = 0;

                // Get fresh ball position
                newBallGridX = (int)(ball->x / TILE_WIDTH);
                newBallGridY = (int)(ball->y / TILE_HEIGHT);

                // If ball has moved, update target
                if (newBallGridX != bossData->targetGridX || newBallGridY != bossData->targetGridY) {
                    bossData->targetGridX = newBallGridX;
                    bossData->targetGridY = newBallGridY;
                    SnakeBossFindPath(snakeBoss, newBallGridX, newBallGridY, world);
                }
            }
        }
    }
    break;

    case SNAKE_STATE_GROWING:
    {
        // Handle growth animation
        bossData->growTimer += deltaTime;
        if (bossData->growTimer >= SNAKE_GROW_TIME) {
            bossData->growTimer = 0.0f;
            bossData->state = SNAKE_STATE_TRACKING;
            bossData->hasTarget = false;  // Force target recalculation
        }
    }
    break;

    case SNAKE_STATE_SHRINKING:
    {
        // Handle shrinking animation
        bossData->shrinkTimer += deltaTime;
        if (bossData->shrinkTimer >= SNAKE_SHRINK_TIME) {
            bossData->shrinkTimer = 0.0f;
            bossData->state = SNAKE_STATE_TRACKING;
            bossData->hasTarget = false;  // Force target recalculation
        }
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
    SnakeBossHandleBallCollision(snakeBoss, ball, player);
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

    // Calculate direction to target
    int dx = targetGridX - headGridX;
    int dy = targetGridY - headGridY;

    TraceLog(LOG_INFO, "Snake at (%d,%d) seeking ball at (%d,%d), dx=%d, dy=%d",
        headGridX, headGridY, targetGridX, targetGridY, dx, dy);

    // Get opposite of current direction (to avoid reversing)
    Direction oppositeDir;
    switch (bossData->currentDir) {
    case DIRECTION_UP: oppositeDir = DIRECTION_DOWN; break;
    case DIRECTION_DOWN: oppositeDir = DIRECTION_UP; break;
    case DIRECTION_LEFT: oppositeDir = DIRECTION_RIGHT; break;
    case DIRECTION_RIGHT: oppositeDir = DIRECTION_LEFT; break;
    default: oppositeDir = DIRECTION_DOWN; break;
    }

    // Calculate current Manhattan distance to target
    int currentDist = abs(dx) + abs(dy);

    // Create arrays for the four primary directions
    Direction directions[4] = { DIRECTION_UP, DIRECTION_RIGHT, DIRECTION_DOWN, DIRECTION_LEFT };
    int newDists[4] = { 0, 0, 0, 0 };
    bool validDirs[4] = { false, false, false, false };

    // Calculate new positions and distances for each direction
    int newX, newY;

    // Check UP
    newX = headGridX;
    newY = headGridY - 1;
    newDists[0] = abs(targetGridX - newX) + abs(targetGridY - newY);
    validDirs[0] = SnakeBossIsValidPosition(snakeBoss, newX, newY, world) && DIRECTION_UP != oppositeDir;

    // Check RIGHT
    newX = headGridX + 1;
    newY = headGridY;
    newDists[1] = abs(targetGridX - newX) + abs(targetGridY - newY);
    validDirs[1] = SnakeBossIsValidPosition(snakeBoss, newX, newY, world) && DIRECTION_RIGHT != oppositeDir;

    // Check DOWN
    newX = headGridX;
    newY = headGridY + 1;
    newDists[2] = abs(targetGridX - newX) + abs(targetGridY - newY);
    validDirs[2] = SnakeBossIsValidPosition(snakeBoss, newX, newY, world) && DIRECTION_DOWN != oppositeDir;

    // Check LEFT
    newX = headGridX - 1;
    newY = headGridY;
    newDists[3] = abs(targetGridX - newX) + abs(targetGridY - newY);
    validDirs[3] = SnakeBossIsValidPosition(snakeBoss, newX, newY, world) && DIRECTION_LEFT != oppositeDir;

    // Choose best valid direction (minimum distance)
    Direction bestDir = bossData->currentDir;
    int bestDist = 999999;
    bool foundValid = false;

    for (int i = 0; i < 4; i++) {
        if (validDirs[i]) {
            // Log each valid direction and its distance
            TraceLog(LOG_DEBUG, "Direction %d is valid, distance: %d",
                directions[i], newDists[i]);

            if (!foundValid || newDists[i] < bestDist) {
                bestDist = newDists[i];
                bestDir = directions[i];
                foundValid = true;
            }
        }
    }

    // If we found a valid direction that gets us closer to the target
    if (foundValid && bestDist <= currentDist) {
        bossData->nextDir = bestDir;
        TraceLog(LOG_INFO, "Snake chose OPTIMAL direction %d, distance: %d -> %d",
            bestDir, currentDist, bestDist);
    }
    // If we found any valid direction but it doesn't get us closer
    else if (foundValid) {
        bossData->nextDir = bestDir;
        TraceLog(LOG_INFO, "Snake chose direction %d, but distance increased: %d -> %d",
            bestDir, currentDist, bestDist);
    }
    // If no valid direction found (should be rare)
    else {
        // Keep current direction
        TraceLog(LOG_WARNING, "No valid direction found, keeping current: %d", bossData->currentDir);
    }
}


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

    // When using larger segments, we might want to move more than 1 tile at a time
    // to maintain visual spacing, but for grid logic we'll keep 1-tile movements
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
*@brief Check if a multi - tile area is valid for the snake to occupy
*
* @param snakeBoss Pointer to snake boss entity
* @param gridX Base X position in grid coordinates(top - left)
* @param gridY Base Y position in grid coordinates(top - left)
* @param world Pointer to game world
* @return true If all tiles in the area are valid
* @return false If any tile in the area is invalid
*/
bool SnakeBossIsValidMultiTilePosition(Entity * snakeBoss, int gridX, int gridY, World * world) {
    if (!snakeBoss || !world) return false;

    // For now, we're just checking the center point for collisions
    // This is a simpler approach that works well with the existing grid system

    // If needed, the code could be expanded to check all tiles in the segment's area
    // by iterating through the area and checking each tile:
    /*
    for (int dx = 0; dx < SNAKE_SEGMENT_WIDTH_TILES; dx++) {
        for (int dy = 0; dy < SNAKE_SEGMENT_HEIGHT_TILES; dy++) {
            float worldX = (gridX + dx) * TILE_WIDTH + TILE_WIDTH / 2.0f;
            float worldY = (gridY + dy) * TILE_HEIGHT + TILE_HEIGHT / 2.0f;

            if (WorldIsWallAtPosition(world, worldX, worldY)) {
                return false;
            }
        }
    }
    */

    return SnakeBossIsValidPosition(snakeBoss, gridX, gridY, world);
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
        // Calculate world position based on grid position and segment size
        bossData->segments[i].worldX = bossData->segments[i].gridX * TILE_WIDTH + (SNAKE_SEGMENT_WIDTH_TILES * TILE_WIDTH) / 2.0f;
        bossData->segments[i].worldY = bossData->segments[i].gridY * TILE_HEIGHT + (SNAKE_SEGMENT_HEIGHT_TILES * TILE_HEIGHT) / 2.0f;
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
* @param player Pointer to player entity (for XP awards)
* @return true If collision occurred
* @return false If no collision
*/
bool SnakeBossHandleBallCollision(Entity* snakeBoss, Entity* ball, Entity* player) {
    if (!snakeBoss || !ball || snakeBoss->type != ENTITY_ENEMY || ball->type != ENTITY_BALL) return false;

    SnakeBossData* bossData = SnakeBossGetData(snakeBoss);
    if (!bossData || bossData->segmentCount <= 0) return false;

    BallData* ballData = BallGetData(ball);
    if (!ballData) return false;

    // Check collision with head
    float headX = bossData->segments[0].worldX;
    float headY = bossData->segments[0].worldY;
    float distance = sqrtf((ball->x - headX) * (ball->x - headX) +
        (ball->y - headY) * (ball->y - headY));

    if (distance < SNAKE_HEAD_RADIUS + ballData->radius) {
        // Collision with head
        if (ballData->state == BALL_STATE_PLAYER) {
            // Blue ball (hit by player) - always damages snake
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

                // Award XP to player if player reference is valid
                if (player && player->type == ENTITY_PLAYER) {
                    PlayerAwardXP(player, PLAYER_XP_PER_HIT);
                    TraceLog(LOG_INFO, "Player awarded XP for hitting snake boss");
                }

                TraceLog(LOG_INFO, "BLUE ball damaged snake! Segments left: %d", bossData->segmentCount);
            }
        }
        else {
            // White or red ball - grow snake
            if (bossData->state != SNAKE_STATE_GROWING) {
                bossData->state = SNAKE_STATE_GROWING;
                bossData->growTimer = 0.0f;

                // Grow snake
                SnakeBossGrow(snakeBoss);

                // Kick ball away and change to red
                float kickForce = 6.0f;
                BallApplyForce(ball, (ball->x - headX) * kickForce, (ball->y - headY) * kickForce);

                // Change ball state to SNAKE (red)
                ballData->state = BALL_STATE_SNAKE;
                ballData->innerColor = RED;
                ballData->outerColor = MAROON;

                TraceLog(LOG_INFO, "Ball eaten by snake, changed to SNAKE state (red). Snake grew to %d segments",
                    bossData->segmentCount);
            }
        }

        return true;
    }

    // Check collision with body segments (only if ball is in PLAYER state)
    if (ballData->state == BALL_STATE_PLAYER) {
        for (int i = 1; i < bossData->segmentCount; i++) {
            float segX = bossData->segments[i].worldX;
            float segY = bossData->segments[i].worldY;

            // Create a collision rectangle for the segment
            Rectangle segRect = {
                segX - SNAKE_SEGMENT_WIDTH / 2.0f,
                segY - SNAKE_SEGMENT_HEIGHT / 2.0f,
                SNAKE_SEGMENT_WIDTH,
                SNAKE_SEGMENT_HEIGHT
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

                    // Award XP to player if player reference is valid
                    if (player && player->type == ENTITY_PLAYER) {
                        PlayerAwardXP(player, PLAYER_XP_PER_HIT);
                        TraceLog(LOG_INFO, "Player awarded XP for hitting snake body");
                    }

                    TraceLog(LOG_INFO, "BLUE ball hit snake body! Segments left: %d", bossData->segmentCount);
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
            segX - SNAKE_SEGMENT_WIDTH / 2.0f,
            segY - SNAKE_SEGMENT_HEIGHT / 2.0f,
            SNAKE_SEGMENT_WIDTH,
            SNAKE_SEGMENT_HEIGHT
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

        // Draw larger rectangle based on configured segment size
        DrawRectangle(
            (int)(segX - SNAKE_SEGMENT_WIDTH / 2.0f),
            (int)(segY - SNAKE_SEGMENT_HEIGHT / 2.0f),
            SNAKE_SEGMENT_WIDTH,
            SNAKE_SEGMENT_HEIGHT,
            bossData->bodyColor
        );
    }

    // Render head (circle) with configurable radius
    float headX = bossData->segments[0].worldX;
    float headY = bossData->segments[0].worldY;

    DrawCircle(
        (int)headX,
        (int)headY,
        SNAKE_HEAD_RADIUS,  // Use configurable head radius
        bossData->headColor
    );

    // Visual effects for growing, shrinking, defeated states
    if (bossData->state == SNAKE_STATE_GROWING) {
        float growthProgress = bossData->growTimer / SNAKE_GROW_TIME;
        float effectSize = SNAKE_HEAD_RADIUS * 0.5f * (1.0f - growthProgress);

        DrawCircleLines(
            (int)headX,
            (int)headY,
            SNAKE_HEAD_RADIUS + effectSize,
            Fade(GREEN, 0.7f * (1.0f - growthProgress))
        );
    }

    if (bossData->state == SNAKE_STATE_SHRINKING) {
        float shrinkProgress = bossData->shrinkTimer / SNAKE_SHRINK_TIME;
        float effectSize = SNAKE_HEAD_RADIUS * 0.5f * (1.0f - shrinkProgress);

        DrawCircleLines(
            (int)headX,
            (int)headY,
            SNAKE_HEAD_RADIUS + effectSize,
            Fade(RED, 0.7f * (1.0f - shrinkProgress))
        );
    }

    if (bossData->state == SNAKE_STATE_DEFEATED) {
        DrawCircleLines(
            (int)headX,
            (int)headY,
            SNAKE_HEAD_RADIUS * 1.5f,
            RED
        );

        DrawCircleLines(
            (int)headX,
            (int)headY,
            SNAKE_HEAD_RADIUS * 1.2f,
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