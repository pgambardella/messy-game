/**
 * @file win_condition.h
 * @brief Win condition definitions and functions
 *
 * This file defines structures and functions for the win condition system,
 * which includes a hole that the ball can fall into to trigger various effects.
 */

#ifndef MESSY_GAME_WIN_CONDITION_H
#define MESSY_GAME_WIN_CONDITION_H

#include "raylib.h"
#include "entity.h"
#include "ball.h"
#include "world.h"

 /**
  * @brief Win condition state enumeration
  *
  * Defines possible states for the win condition system.
  */
typedef enum {
    WIN_STATE_IDLE,           // No ball in hole
    WIN_STATE_PLAYER_SCORED,  // Player ball in hole
    WIN_STATE_ENEMY_SCORED,   // Enemy ball in hole
    WIN_STATE_NEUTRAL_HOLD    // Neutral ball in hold waiting to be ejected
} WinConditionState;

/**
 * @brief Thunder particle structure
 *
 * Represents a single particle in the thunder effect.
 */
typedef struct {
    Vector2 position;  // Particle position
    Vector2 velocity;  // Particle velocity
    float size;        // Particle size
    float alpha;       // Particle transparency
    bool active;       // Whether particle is active
} ThunderParticle;

/**
 * @brief Win condition structure
 *
 * Contains all data needed for the win condition system.
 */
typedef struct {
    Vector2 position;               // Position of hole
    float radius;                   // Radius of hole
    WinConditionState state;        // Current state
    float stateTimer;               // Timer for state transitions
    ThunderParticle* particles;     // Array of thunder particles
    int particleCount;              // Number of particles
    bool flashTextActive;           // Whether flash text is active
    float flashTextTimer;           // Timer for flash text
    float flashTextAlpha;           // Alpha for flash text
} WinCondition;

/**
 * @brief Create a new win condition
 *
 * @param x X position of hole
 * @param y Y position of hole
 * @param radius Radius of hole
 * @return WinCondition* Pointer to created win condition
 */
WinCondition* WinConditionCreate(float x, float y, float radius);

/**
 * @brief Destroy win condition and free resources
 *
 * @param winCondition Pointer to win condition
 */
void WinConditionDestroy(WinCondition* winCondition);

/**
 * @brief Update win condition state
 *
 * @param winCondition Pointer to win condition
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 * @param entities Array of all entities
 * @param entityCount Number of entities
 * @param deltaTime Time elapsed since last update
 */
void WinConditionUpdate(
    WinCondition* winCondition,
    Entity* ball,
    Entity* player,
    Entity** entities,
    int entityCount,
    float deltaTime
);

/**
 * @brief Render win condition
 *
 * @param winCondition Pointer to win condition
 */
void WinConditionRender(WinCondition* winCondition);

/**
 * @brief Check if ball has fallen into hole
 *
 * @param winCondition Pointer to win condition
 * @param ball Pointer to ball entity
 * @return true If ball has fallen into hole
 * @return false If ball has not fallen into hole
 */
bool WinConditionCheckBallInHole(WinCondition* winCondition, Entity* ball);

/**
 * @brief Handle player scoring
 *
 * @param winCondition Pointer to win condition
 * @param ball Pointer to ball entity
 * @param entities Array of all entities
 * @param entityCount Number of entities
 */
void WinConditionHandlePlayerScore(
    WinCondition* winCondition,
    Entity* ball,
    Entity** entities,
    int entityCount
);

/**
 * @brief Handle enemy scoring
 *
 * @param winCondition Pointer to win condition
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 */
void WinConditionHandleEnemyScore(
    WinCondition* winCondition,
    Entity* ball,
    Entity* player
);

/**
 * @brief Handle neutral ball in hole
 *
 * @param winCondition Pointer to win condition
 * @param ball Pointer to ball entity
 * @param deltaTime Time elapsed since last update
 * @return true If ball should be ejected
 * @return false If ball should continue to be held
 */
bool WinConditionHandleNeutralBall(
    WinCondition* winCondition,
    Entity* ball,
    float deltaTime
);

/**
 * @brief Eject ball from hole
 *
 * @param winCondition Pointer to win condition
 * @param ball Pointer to ball entity
 */
void WinConditionEjectBall(WinCondition* winCondition, Entity* ball);

/**
 * @brief Trigger thunder effect
 *
 * @param winCondition Pointer to win condition
 * @param originX X coordinate of thunder origin
 * @param originY Y coordinate of thunder origin
 * @param targetX X coordinate of thunder target
 * @param targetY Y coordinate of thunder target
 */
void WinConditionTriggerThunder(
    WinCondition* winCondition,
    float originX,
    float originY,
    float targetX,
    float targetY
);

/**
 * @brief Update thunder particles
 *
 * @param winCondition Pointer to win condition
 * @param deltaTime Time elapsed since last update
 */
void WinConditionUpdateThunder(WinCondition* winCondition, float deltaTime);

/**
 * @brief Render thunder particles
 *
 * @param winCondition Pointer to win condition
 */
void WinConditionRenderThunder(WinCondition* winCondition);

/**
 * @brief Trigger flash text
 *
 * @param winCondition Pointer to win condition
 */
void WinConditionTriggerFlashText(WinCondition* winCondition);

/**
 * @brief Update flash text
 *
 * @param winCondition Pointer to win condition
 * @param deltaTime Time elapsed since last update
 */
void WinConditionUpdateFlashText(WinCondition* winCondition, float deltaTime);

/**
 * @brief Render flash text
 *
 * @param winCondition Pointer to win condition
 */
void WinConditionRenderFlashText(WinCondition* winCondition);

#endif // MESSY_GAME_WIN_CONDITION_H