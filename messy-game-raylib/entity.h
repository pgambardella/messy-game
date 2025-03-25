/**
 * @file entity.h
 * @brief Base definitions for game entities
 *
 * This file defines the base entity structure and common functionality
 * shared by all game entities like players, balls, and enemies.
 */

#ifndef MESSY_GAME_ENTITY_H
#define MESSY_GAME_ENTITY_H

#include <stdbool.h>
#include "raylib.h"
#include "config.h"

 /**
  * @brief Entity types enumeration
  *
  * Used to identify different types of entities in the game.
  * Extensible for future entity types.
  */
typedef enum {
    ENTITY_PLAYER,
    ENTITY_BALL,
    ENTITY_ENEMY,
    ENTITY_POWERUP,
    // Add more entity types as needed
    ENTITY_COUNT
} EntityType;

/**
 * @brief Direction enumeration
 *
 * Represents cardinal directions an entity can face.
 */
typedef enum {
    DIRECTION_DOWN = 0,
    DIRECTION_UP = 1,
    DIRECTION_LEFT = 2,
    DIRECTION_RIGHT = 3
} Direction;

/**
 * @brief Base entity structure
 *
 * Common properties shared by all entities.
 * Specific entity types will extend this with additional properties.
 */
typedef struct {
    EntityType type;       // Type of entity
    float x;               // X position in world
    float y;               // Y position in world
    float width;           // Width of entity
    float height;          // Height of entity
    float speedX;          // Horizontal speed
    float speedY;          // Vertical speed
    bool active;           // Whether entity is active
    Direction facing;      // Direction entity is facing
    Color tint;            // Color tint for rendering
    void* typeData;        // Pointer to type-specific data
} Entity;

/**
 * @brief Initialize a new entity
 *
 * @param type The type of entity to create
 * @param x Initial X position
 * @param y Initial Y position
 * @param width Entity width
 * @param height Entity height
 * @return Entity* Pointer to the created entity or NULL if failed
 */
Entity* EntityCreate(EntityType type, float x, float y, float width, float height);

/**
 * @brief Free entity resources
 *
 * @param entity Pointer to entity to destroy
 */
void EntityDestroy(Entity* entity);

/**
 * @brief Update entity state
 *
 * @param entity Pointer to entity to update
 * @param deltaTime Time elapsed since last update
 */
void EntityUpdate(Entity* entity, float deltaTime);

/**
 * @brief Render entity to screen
 *
 * @param entity Pointer to entity to render
 */
void EntityRender(Entity* entity);

/**
 * @brief Check if two entities are colliding
 *
 * @param a First entity
 * @param b Second entity
 * @return true Entities are colliding
 * @return false Entities are not colliding
 */
bool EntityCheckCollision(Entity* a, Entity* b);

#endif // MESSY_GAME_ENTITY_H