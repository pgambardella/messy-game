/**
* @file player.h
* @brief Player entity definitions and functions
*
* This file defines player-specific data structures and functions.
* Handles player movement, animation, abilities, and states.
*/
#ifndef MESSY_GAME_PLAYER_H
#define MESSY_GAME_PLAYER_H
#include "entity.h"
#include "world.h"
#define PLAYER_BASE_KICK_FORCE 5.0f // Base kick force
#define PLAYER_BASE_MOVE_SPEED 1.0f // Base movement multiplier
#define PLAYER_KICK_FORCE_PER_LEVEL 0.5f // 50% kick force increase per level
#define PLAYER_MOVE_SPEED_PER_LEVEL 0.5f // 50% speed increase per level
#define PLAYER_XP_PER_HIT 110.0f // XP gained per successful enemy hit
#define PLAYER_BASE_MAX_XP 100.0f // XP needed for first level up
#define PLAYER_XP_SCALE_FACTOR 1.1f // Each level requires 10% more XP
/**
* @brief Player state enumeration
*
* Defines all possible states for a player entity.
*/
typedef enum {
    PLAYER_STATE_ALIVE, // Player is alive and active
    PLAYER_STATE_DYING, // Player is dying (death animation playing)
    PLAYER_STATE_DEAD   // Player is dead (death screen showing)
} PlayerState;

/**
* @brief Player animation states
*
* Defines all possible animation states for a player character.
*/
typedef enum {
    // Idle animations (row 0)
    ANIM_IDLE_DOWN,   // Column 0
    ANIM_IDLE_LEFT,   // Column 1
    ANIM_IDLE_UP,     // Column 2
    ANIM_IDLE_RIGHT,  // Column 3

    // Walk animations
    ANIM_WALK_DOWN,   // Row 1, Columns 0-3
    ANIM_WALK_UP,     // Row 2, Columns 0-3
    ANIM_WALK_LEFT,   // Row 3, Columns 0-3
    ANIM_WALK_RIGHT,  // Row 4, Columns 0-3

    // Death animation (Row 5, Columns 0-4)
    ANIM_DEATH,

    // Count of all animations
    ANIM_COUNT
} PlayerAnimation;

/**
* @brief Player character types/classes
*
* Defines different player types for future character selection.
*/
typedef enum {
    PLAYER_TYPE_KNIGHT,
    PLAYER_TYPE_MAGE,
    PLAYER_TYPE_ARCHER,
    // Add more player types as needed
    PLAYER_TYPE_COUNT
} PlayerType;

/**
* @brief Player-specific data
*
* Extended data for player entities.
*/
typedef struct {
    PlayerType type;          // Player character type
    PlayerAnimation currentAnim; // Current animation state
    int frameCounter;         // Frame counter for animation timing
    int currentFrame;         // Current animation frame
    int level;                // Player level
    float maxHealth;          // Maximum health
    float currentHealth;      // Current health
    float currentXP;          // Current XP
    float maxXP;              // XP needed for next level
    float kickForce;          // Force applied when kicking the ball
    float moveSpeed;          // Movement speed multiplier
    bool hasSpecialAbility;   // Whether player has special ability
    PlayerState state;        // Current player state
    float deathTimer;         // Timer for death animation and screen
    // Add more player-specific attributes as needed
} PlayerData;

/**
* @brief Create a new player entity
*
* @param type Player character type
* @param x Initial X position
* @param y Initial Y position
* @return Entity* Pointer to the created player entity
*/
Entity* PlayerCreate(PlayerType type, float x, float y);

/**
* @brief Update player state based on input and game state
*
* @param player Pointer to player entity
* @param world Pointer to game world
* @param deltaTime Time elapsed since last update
*/
void PlayerUpdate(Entity* player, World* world, float deltaTime);

/**
* @brief Award XP to player and handle level up
*
* @param player Pointer to player entity
* @param xpAmount Amount of XP to award
* @return true If player leveled up
* @return false If no level up occurred
*/
bool PlayerAwardXP(Entity* player, float xpAmount);

/**
* @brief Render player with appropriate animation
*
* @param player Pointer to player entity
*/
void PlayerRender(Entity* player);

/**
* @brief Handle player movement based on input
*
* @param player Pointer to player entity
* @param world Pointer to game world
* @param deltaTime Time elapsed since last update
*/
void PlayerHandleMovement(Entity* player, World* world, float deltaTime);

/**
* @brief Update player animation state
*
* @param player Pointer to player entity
* @param deltaTime Time elapsed since last update
*/
void PlayerUpdateAnimation(Entity* player, float deltaTime);

/**
* @brief Reset player to starting position
*
* @param player Pointer to player entity
* @param x New X position
* @param y New Y position
*/
void PlayerReset(Entity* player, float x, float y);

/**
* @brief Get player-specific data from entity
*
* @param player Pointer to player entity
* @return PlayerData* Pointer to player data
*/
PlayerData* PlayerGetData(Entity* player);

/**
* @brief Handle player death
*
* @param player Pointer to player entity
* @param deltaTime Time elapsed since last update
* @return true If death sequence is complete
* @return false If death sequence is still in progress
*/
bool PlayerHandleDeath(Entity* player, float deltaTime);

/**
* @brief Render the death screen
*
* @param player Pointer to player entity
*/
void PlayerRenderDeathScreen(Entity* player);

#endif // MESSY_GAME_PLAYER_H