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

 /**
  * @brief Player animation states
  *
  * Defines all possible animation states for a player character.
  */
typedef enum {
    ANIM_IDLE_DOWN,
    ANIM_WALK_DOWN,
    ANIM_IDLE_UP,
    ANIM_WALK_UP,
    ANIM_IDLE_LEFT,
    ANIM_WALK_LEFT,
    ANIM_IDLE_RIGHT,
    ANIM_WALK_RIGHT,
    // Future animations (attack, special moves, etc.)
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
    PlayerType type;           // Player character type
    PlayerAnimation currentAnim; // Current animation state
    int frameCounter;          // Frame counter for animation timing
    int currentFrame;          // Current animation frame
    int level;                 // Player level
    float maxHealth;           // Maximum health
    float currentHealth;       // Current health
    float maxMana;             // Maximum mana/energy
    float currentMana;         // Current mana/energy
    bool hasSpecialAbility;    // Whether player has special ability
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

#endif // MESSY_GAME_PLAYER_H