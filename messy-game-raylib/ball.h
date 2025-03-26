/**
 * @file ball.h
 * @brief Ball entity definitions and functions
 *
 * This file defines ball-specific data structures and functions.
 * Handles ball physics, collision, and special effects.
 */

#ifndef MESSY_GAME_BALL_H
#define MESSY_GAME_BALL_H

#include "entity.h"
#include "world.h"

 /**
  * @brief Ball types enumeration
  *
  * Defines different types of balls with unique behaviors.
  */
typedef enum {
    BALL_TYPE_NORMAL,
    BALL_TYPE_FIRE,
    BALL_TYPE_ICE,
    BALL_TYPE_LIGHTNING,
    // Add more ball types as needed
    BALL_TYPE_COUNT
} BallType;

/**
* @brief Ball state enumeration
*
* Defines the current state of the ball based on its last interaction.
*/
typedef enum {
    BALL_STATE_NEUTRAL, // White, neutral state
    BALL_STATE_PLAYER,  // Blue, kicked by player, damages snake
    BALL_STATE_SNAKE    // Red, kicked by snake, damages player
} BallState;

/**
 * @brief Ball-specific data
 *
 * Extended data for ball entities.
 */
typedef struct {
    BallType type;         // Type of ball
    BallState state;       // Current state of the ball
    float radius;          // Ball radius
    float bounceFactor;    // How bouncy the ball is
    float friction;        // How quickly the ball slows down
    float damage;          // Damage dealt to enemies
    Color innerColor;      // Inner color for special effects
    Color outerColor;      // Outer color for special effects
    bool hasSpecialEffect; // Whether ball has special effects
    // Add more ball-specific attributes as needed
} BallData;

/**
 * @brief Create a new ball entity
 *
 * @param type Ball type
 * @param x Initial X position
 * @param y Initial Y position
 * @return Entity* Pointer to the created ball entity
 */
Entity* BallCreate(BallType type, float x, float y);

/**
 * @brief Update ball state based on physics
 *
 * @param ball Pointer to ball entity
 * @param world Pointer to game world
 * @param player Pointer to player entity for player-ball interaction
 * @param deltaTime Time elapsed since last update
 */
void BallUpdate(Entity* ball, World* world, Entity* player, float deltaTime);

/**
 * @brief Render ball with appropriate effects
 *
 * @param ball Pointer to ball entity
 */
void BallRender(Entity* ball);

/**
 * @brief Reset ball to position near player
 *
 * @param ball Pointer to ball entity
 * @param x New X position
 * @param y New Y position
 */
void BallReset(Entity* ball, float x, float y);

/**
 * @brief Handle ball collision with walls
 *
 * @param ball Pointer to ball entity
 * @param world Pointer to game world
 * @param prevX Previous X position
 * @param prevY Previous Y position
 */
void BallHandleWallCollision(Entity* ball, World* world, float prevX, float prevY);

/**
 * @brief Handle ball collision with player
 *
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 */
void BallHandlePlayerCollision(Entity* ball, Entity* player);

/**
 * @brief Handle ball collision with enemies
 *
 * @param ball Pointer to ball entity
 * @param enemy Pointer to enemy entity
 * @return true If collision occurred
 * @return false If no collision
 */
bool BallHandleEnemyCollision(Entity* ball, Entity* enemy);

/**
 * @brief Apply force to ball from a direction
 *
 * @param ball Pointer to ball entity
 * @param forceX X component of force
 * @param forceY Y component of force
 */
void BallApplyForce(Entity* ball, float forceX, float forceY);

/**
 * @brief Get ball-specific data from entity
 *
 * @param ball Pointer to ball entity
 * @return BallData* Pointer to ball data
 */
BallData* BallGetData(Entity* ball);

#endif // MESSY_GAME_BALL_H