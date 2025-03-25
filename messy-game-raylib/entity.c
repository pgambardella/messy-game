/**
 * @file entity.c
 * @brief Implementation of base entity functionality
 */

#include <stdlib.h>
#include <math.h>
#include "entity.h"

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
Entity* EntityCreate(EntityType type, float x, float y, float width, float height) {
    Entity* entity = (Entity*)malloc(sizeof(Entity));
    if (!entity) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for entity");
        return NULL;
    }

    entity->type = type;
    entity->x = x;
    entity->y = y;
    entity->width = width;
    entity->height = height;
    entity->speedX = 0.0f;
    entity->speedY = 0.0f;
    entity->active = true;
    entity->facing = DIRECTION_DOWN;
    entity->tint = WHITE;
    entity->typeData = NULL;

    return entity;
}

/**
 * @brief Free entity resources
 *
 * @param entity Pointer to entity to destroy
 */
void EntityDestroy(Entity* entity) {
    if (!entity) return;

    // Free type-specific data if it exists
    if (entity->typeData) {
        free(entity->typeData);
        entity->typeData = NULL;
    }

    // Free entity
    free(entity);
}

/**
 * @brief Update entity state
 *
 * Base update function for generic entities.
 * Specific entity types should implement their own update functions.
 *
 * @param entity Pointer to entity to update
 * @param deltaTime Time elapsed since last update
 */
void EntityUpdate(Entity* entity, float deltaTime) {
    if (!entity || !entity->active) return;

    // Apply current speed to position
    entity->x += entity->speedX;
    entity->y += entity->speedY;

    // Update facing direction based on movement
    if (fabs(entity->speedX) > fabs(entity->speedY)) {
        if (entity->speedX > 0) entity->facing = DIRECTION_RIGHT;
        else if (entity->speedX < 0) entity->facing = DIRECTION_LEFT;
    }
    else if (entity->speedY != 0) {
        if (entity->speedY > 0) entity->facing = DIRECTION_DOWN;
        else if (entity->speedY < 0) entity->facing = DIRECTION_UP;
    }
}

/**
 * @brief Render entity to screen
 *
 * Base render function that draws a simple rectangle.
 * Specific entity types should implement their own render functions.
 *
 * @param entity Pointer to entity to render
 */
void EntityRender(Entity* entity) {
    if (!entity || !entity->active) return;

    // Default rendering is just a colored rectangle
    // This should be overridden by specific entity types
    DrawRectangle(
        (int)(entity->x - entity->width / 2),
        (int)(entity->y - entity->height / 2),
        (int)entity->width,
        (int)entity->height,
        entity->tint
    );
}

/**
 * @brief Check if two entities are colliding
 *
 * Uses a simple rectangle-based collision detection algorithm.
 *
 * @param a First entity
 * @param b Second entity
 * @return true Entities are colliding
 * @return false Entities are not colliding
 */
bool EntityCheckCollision(Entity* a, Entity* b) {
    if (!a || !b || !a->active || !b->active) return false;

    // Calculate entity boundaries
    float a_left = a->x - a->width / 2;
    float a_right = a->x + a->width / 2;
    float a_top = a->y - a->height / 2;
    float a_bottom = a->y + a->height / 2;

    float b_left = b->x - b->width / 2;
    float b_right = b->x + b->width / 2;
    float b_top = b->y - b->height / 2;
    float b_bottom = b->y + b->height / 2;

    // Check for overlap in both dimensions
    return (a_right >= b_left && a_left <= b_right &&
        a_bottom >= b_top && a_top <= b_bottom);
}

/**
 * @brief Check if entity is inside a rectangle
 *
 * Utility function to check if an entity is contained within a rectangle.
 *
 * @param entity Pointer to entity
 * @param rect Rectangle to check
 * @return true Entity is inside rectangle
 * @return false Entity is not inside rectangle
 */
bool EntityIsInsideRectangle(Entity* entity, Rectangle rect) {
    if (!entity) return false;

    // Calculate entity boundaries
    float left = entity->x - entity->width / 2;
    float right = entity->x + entity->width / 2;
    float top = entity->y - entity->height / 2;
    float bottom = entity->y + entity->height / 2;

    // Check if entity is completely inside rectangle
    return (left >= rect.x && right <= rect.x + rect.width &&
        top >= rect.y && bottom <= rect.y + rect.height);
}

/**
 * @brief Move entity and check for collisions
 *
 * Utility function to move an entity while checking for wall collisions.
 *
 * @param entity Pointer to entity
 * @param dx X movement delta
 * @param dy Y movement delta
 * @param isWallAtPosition Function pointer to wall collision check
 * @return true Movement was successful
 * @return false Movement was blocked
 */
bool EntityMoveWithCollision(
    Entity* entity,
    float dx,
    float dy,
    bool (*isWallAtPosition)(float, float)
) {
    if (!entity || !isWallAtPosition) return false;

    bool moved = false;

    // Store original position
    float originalX = entity->x;
    float originalY = entity->y;

    // Try horizontal movement
    entity->x += dx;
    if (isWallAtPosition(entity->x, entity->y)) {
        // Horizontal movement caused collision, revert
        entity->x = originalX;
    }
    else {
        moved = true;
    }

    // Try vertical movement
    entity->y += dy;
    if (isWallAtPosition(entity->x, entity->y)) {
        // Vertical movement caused collision, revert
        entity->y = originalY;
    }
    else {
        moved = true;
    }

    return moved;
}

/**
 * @brief Calculate direction between two entities
 *
 * Returns normalized direction vector from entity a to entity b.
 *
 * @param a Source entity
 * @param b Target entity
 * @return Vector2 Normalized direction vector
 */
Vector2 EntityDirectionTo(Entity* a, Entity* b) {
    if (!a || !b) return (Vector2) { 0, 0 };

    Vector2 direction = {
        b->x - a->x,
        b->y - a->y
    };

    // Calculate magnitude
    float magnitude = sqrtf(direction.x * direction.x + direction.y * direction.y);

    // Normalize if magnitude is not zero
    if (magnitude > 0) {
        direction.x /= magnitude;
        direction.y /= magnitude;
    }

    return direction;
}

/**
 * @brief Calculate distance between two entities
 *
 * @param a First entity
 * @param b Second entity
 * @return float Distance between entities
 */
float EntityDistanceTo(Entity* a, Entity* b) {
    if (!a || !b) return 0.0f;

    float dx = b->x - a->x;
    float dy = b->y - a->y;

    return sqrtf(dx * dx + dy * dy);
}