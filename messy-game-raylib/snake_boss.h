/**
* @file snake_boss.h
* @brief Snake boss enemy definitions and functions
*
* This file defines snake boss-specific data structures and functions.
* Handles the snake boss's movement, collision, growth, and states.
*/
#ifndef MESSY_GAME_SNAKE_BOSS_H
#define MESSY_GAME_SNAKE_BOSS_H

#include "entity.h"
#include "world.h"
#include "ball.h"

/**
* @brief Snake boss state enumeration
*
* Defines all possible states for the snake boss.
*/
typedef enum {
    SNAKE_STATE_IDLE,        // Snake is not moving
    SNAKE_STATE_TRACKING,    // Snake is deciding where to move next
    SNAKE_STATE_MOVING,      // Snake is currently moving
    SNAKE_STATE_GROWING,     // Snake is growing after catching the ball
    SNAKE_STATE_SHRINKING,   // Snake is shrinking after being hit by the ball
    SNAKE_STATE_DEFEATED     // Snake is defeated (only head remains and hit)
} SnakeBossState;

/**
* @brief Snake boss segment structure
*
* Represents a single segment of the snake boss.
*/
typedef struct {
    int gridX;               // X position in grid coordinates
    int gridY;               // Y position in grid coordinates
    float worldX;            // X position in world coordinates
    float worldY;            // Y position in world coordinates
} SnakeSegment;

/**
* @brief Snake boss-specific data
*
* Extended data for snake boss entities.
*/
typedef struct {
    SnakeBossState state;    // Current state of the snake boss
    SnakeSegment* segments;  // Array of all segments
    int segmentCount;        // Current number of segments
    int segmentCapacity;     // Capacity of segments array
    Direction currentDir;    // Current movement direction
    Direction nextDir;       // Next movement direction

    int targetGridX;         // Target X position in grid coordinates
    int targetGridY;         // Target Y position in grid coordinates
    bool hasTarget;          // Whether snake has a current target

    float moveTimer;         // Timer for movement control
    float moveInterval;      // Time between moves (decreases as snake grows)
    float growTimer;         // Timer for growth animation
    float shrinkTimer;       // Timer for shrink animation

    Color headColor;         // Color of the snake head
    Color bodyColor;         // Color of the body segments
} SnakeBossData;

/**
* @brief Create a new snake boss entity
*
* @param gridX Initial X position in grid coordinates
* @param gridY Initial Y position in grid coordinates
* @param initialLength Initial number of segments
* @return Entity* Pointer to the created snake boss entity
*/
Entity* SnakeBossCreate(int gridX, int gridY, int initialLength);

/**
* @brief Update snake boss state
*
* @param snakeBoss Pointer to snake boss entity
* @param world Pointer to game world
* @param ball Pointer to ball entity
* @param player Pointer to player entity
* @param deltaTime Time elapsed since last update
*/
void SnakeBossUpdate(Entity* snakeBoss, World* world, Entity* ball, Entity* player, float deltaTime);

/**
* @brief Render snake boss
*
* @param snakeBoss Pointer to snake boss entity
*/
void SnakeBossRender(Entity* snakeBoss);

/**
* @brief Handle snake boss collision with the ball
*
* @param snakeBoss Pointer to snake boss entity
* @param ball Pointer to ball entity
* @param player Pointer to player entity (for XP awards)
* @return true If collision occurred
* @return false If no collision
*/
bool SnakeBossHandleBallCollision(Entity* snakeBoss, Entity* ball, Entity* player);

/**
* @brief Handle snake boss collision with the player
*
* @param snakeBoss Pointer to snake boss entity
* @param player Pointer to player entity
* @return true If collision occurred
* @return false If no collision
*/
bool SnakeBossHandlePlayerCollision(Entity* snakeBoss, Entity* player);

/**
* @brief Make the snake boss grow by one segment
*
* @param snakeBoss Pointer to snake boss entity
*/
void SnakeBossGrow(Entity* snakeBoss);

/**
* @brief Make the snake boss shrink by one segment
*
* @param snakeBoss Pointer to snake boss entity
* @return true If snake still has segments left
* @return false If snake is defeated (no segments left)
*/
bool SnakeBossShrink(Entity* snakeBoss);

/**
* @brief Find path to target for the snake boss
*
* @param snakeBoss Pointer to snake boss entity
* @param targetGridX Target X position in grid coordinates
* @param targetGridY Target Y position in grid coordinates
* @param world Pointer to game world
*/
void SnakeBossFindPath(Entity* snakeBoss, int targetGridX, int targetGridY, World* world);

/**
* @brief Move the snake boss one step in its current direction
*
* @param snakeBoss Pointer to snake boss entity
* @param world Pointer to game world
* @return true If move was successful
* @return false If move failed (e.g., wall collision)
*/
bool SnakeBossMove(Entity* snakeBoss, World* world);

/**
* @brief Update all segment positions
*
* @param snakeBoss Pointer to snake boss entity
*/
void SnakeBossUpdateSegments(Entity* snakeBoss);

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
bool SnakeBossIsValidPosition(Entity* snakeBoss, int gridX, int gridY, World* world);

/**
* @brief Get snake boss-specific data from entity
*
* @param snakeBoss Pointer to snake boss entity
* @return SnakeBossData* Pointer to snake boss data
*/
SnakeBossData* SnakeBossGetData(Entity* snakeBoss);

/**
* @brief Check if entity is a snake boss
*
* @param entity Pointer to entity
* @return true If entity is a snake boss
* @return false If entity is not a snake boss
*/
bool IsSnakeBoss(Entity* entity);

#endif // MESSY_GAME_SNAKE_BOSS_H