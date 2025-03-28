/**
 * @file match.h
 * @brief Soccer match game mode and goal mechanics
 *
 * This file defines the structures and functions for implementing
 * a soccer match game mode with goal scoring and time tracking.
 */
#ifndef MESSY_GAME_MATCH_H
#define MESSY_GAME_MATCH_H

#include "raylib.h"
#include "entity.h"
#include "ball.h"
#include "world.h"
#include "camera.h" // Required for GameCamera type

 /**
  * @brief Match states enumeration
  *
  * Defines possible states for the soccer match
  */
typedef enum {
    MATCH_STATE_PLAYING,     // Normal match play
    MATCH_STATE_GOAL,        // Goal celebration
    MATCH_STATE_FINISHED     // Match ended
} MatchState;

/**
 * @brief Goal scorer enumeration
 *
 * Identifies who scored a goal
 */
typedef enum {
    GOAL_SCORER_NONE,        // No goal scored
    GOAL_SCORER_PLAYER,      // Player scored
    GOAL_SCORER_ENEMY        // Enemy scored
} GoalScorer;

/**
 * @brief Goal structure
 *
 * Defines the goal's position and dimensions
 */
typedef struct {
    Rectangle area;           // Full goal area
    Rectangle netEntrance;    // The part where the ball enters
    Vector2 position;         // Goal position (top-left)
    int width;                // Width in tiles
    int height;               // Height in tiles
} Goal;

/**
 * @brief Match structure
 *
 * Contains all match state information
 */
typedef struct {
    MatchState state;              // Current match state
    GoalScorer lastScorer;         // Who scored the last goal
    Goal goal;                     // The goal
    int playerScore;               // Player score
    int enemyScore;                // Enemy score
    float matchTime;               // Total match time (in seconds)
    float currentTime;             // Current match time (in seconds)
    float goalCelebrationTime;     // Current goal celebration time
    float goalCelebrationDuration; // Duration of goal celebration
    bool isActive;                 // Whether match is active
} Match;

/**
 * @brief Create a new match
 *
 * @param world Pointer to game world
 * @return Match* Pointer to the created match
 */
Match* MatchCreate(World* world);

/**
 * @brief Destroy match and free resources
 *
 * @param match Pointer to match
 */
void MatchDestroy(Match* match);

/**
 * @brief Update match state
 *
 * @param match Pointer to match
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 * @param entities Pointer to array of entities
 * @param entityCount Number of entities
 * @param deltaTime Time elapsed since last update
 */
void MatchUpdate(Match* match, Entity* ball, Entity* player, Entity** entities,
    int entityCount, float deltaTime);

/**
 * @brief Render match UI elements
 *
 * @param match Pointer to match
 * @param ball Pointer to ball entity for state display
 */
void MatchRenderUI(Match* match, Entity* ball);

/**
 * @brief Check if a goal was scored
 *
 * @param match Pointer to match
 * @param ball Pointer to ball entity
 * @return GoalScorer Who scored the goal (or GOAL_SCORER_NONE)
 */
GoalScorer MatchCheckGoal(Match* match, Entity* ball);

/**
 * @brief Handle a goal being scored
 *
 * @param match Pointer to match
 * @param scorer Who scored the goal
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 * @param entities Pointer to array of entities
 * @param entityCount Number of entities
 */
void MatchHandleGoal(Match* match, GoalScorer scorer, Entity* ball,
    Entity* player, Entity** entities, int entityCount);

/**
 * @brief Reset positions after a goal
 *
 * @param match Pointer to match
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 * @param entities Pointer to array of entities
 * @param entityCount Number of entities
 */
void MatchResetPositions(Match* match, Entity* ball, Entity* player,
    Entity** entities, int entityCount);

/**
 * @brief Initialize the goal in the world
 *
 * @param match Pointer to match
 * @param world Pointer to world
 */
void MatchInitializeGoal(Match* match, World* world);

/**
 * @brief Render goal celebration effects
 *
 * @param match Pointer to match
 */
void MatchRenderGoalCelebration(Match* match);

/**
 * @brief Initialize world layout with goal
 *
 * @param world Pointer to world
 * @param camera Pointer to camera
 * @param match Pointer to match
 */
void InitializeWorldLayoutWithGoal(World* world, GameCamera* camera, Match* match);

#endif // MESSY_GAME_MATCH_H