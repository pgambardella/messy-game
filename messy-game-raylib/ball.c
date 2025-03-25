/**
 * @file ball.c
 * @brief Implementation of ball entity
 */

#include <stdlib.h>
#include <math.h>
#include "ball.h"
#include "config.h"

 /**
  * @brief Create a new ball entity
  *
  * @param type Ball type
  * @param x Initial X position
  * @param y Initial Y position
  * @return Entity* Pointer to the created ball entity
  */
Entity* BallCreate(BallType type, float x, float y) {
    // For a ball, width and height are based on radius
    float diameter = BALL_RADIUS * 2;

    // Create base entity
    Entity* ball = EntityCreate(ENTITY_BALL, x, y, diameter, diameter);
    if (!ball) {
        TraceLog(LOG_ERROR, "Failed to create ball entity");
        return NULL;
    }

    // Create ball-specific data
    BallData* ballData = (BallData*)malloc(sizeof(BallData));
    if (!ballData) {
        TraceLog(LOG_ERROR, "Failed to allocate ball data");
        EntityDestroy(ball);
        return NULL;
    }

    // Initialize ball data
    ballData->type = type;
    ballData->radius = BALL_RADIUS;
    ballData->bounceFactor = BALL_BOUNCE_FACTOR;
    ballData->friction = BALL_FRICTION;
    ballData->damage = 10.0f; // Default damage value
    ballData->hasSpecialEffect = false;

    // Set type-specific properties
    switch (type) {
    case BALL_TYPE_NORMAL:
        ballData->innerColor = RED;
        ballData->outerColor = RED;
        break;

    case BALL_TYPE_FIRE:
        ballData->innerColor = (Color){ 255, 100, 0, 255 }; // Orange
        ballData->outerColor = (Color){ 255, 50, 0, 200 };  // Red-orange
        ballData->damage = 15.0f;
        ballData->hasSpecialEffect = true;
        break;

    case BALL_TYPE_ICE:
        ballData->innerColor = (Color){ 180, 230, 255, 255 }; // Light blue
        ballData->outerColor = (Color){ 100, 200, 255, 200 };  // Blue
        ballData->bounceFactor = 0.95f; // Ice balls bounce more
        ballData->friction = 0.99f;     // Ice balls have less friction
        ballData->damage = 8.0f;
        ballData->hasSpecialEffect = true;
        break;

    case BALL_TYPE_LIGHTNING:
        ballData->innerColor = (Color){ 255, 255, 100, 255 }; // Yellow
        ballData->outerColor = (Color){ 200, 200, 255, 200 };  // Light purple
        ballData->bounceFactor = 0.7f;  // Lightning balls bounce less
        ballData->damage = 20.0f;       // But do more damage
        ballData->hasSpecialEffect = true;
        break;

    default:
        ballData->innerColor = RED;
        ballData->outerColor = RED;
        break;
    }

    // Set initial ball state
    ball->speedX = 0;
    ball->speedY = 0;
    ball->active = true;
    ball->typeData = ballData;

    return ball;
}

/**
 * @brief Update ball state based on physics
 *
 * @param ball Pointer to ball entity
 * @param world Pointer to game world
 * @param player Pointer to player entity for player-ball interaction
 * @param deltaTime Time elapsed since last update
 */
void BallUpdate(Entity* ball, World* world, Entity* player, float deltaTime) {
    if (!ball || !world || !player || ball->type != ENTITY_BALL) return;

    // Skip if ball is not active
    if (!ball->active) return;

    BallData* ballData = (BallData*)ball->typeData;
    if (!ballData) return;

    // Store previous position for collision response
    float prevX = ball->x;
    float prevY = ball->y;

    // Apply ball physics - update position based on speed
    ball->x += ball->speedX;
    ball->y += ball->speedY;

    // Apply friction to gradually slow down the ball
    ball->speedX *= ballData->friction;
    ball->speedY *= ballData->friction;

    // Handle wall collisions
    BallHandleWallCollision(ball, world, prevX, prevY);

    // Handle collision with player
    BallHandlePlayerCollision(ball, player);

    // Special effects based on ball type
    if (ballData->hasSpecialEffect) {
        switch (ballData->type) {
        case BALL_TYPE_FIRE:
            // Fire ball effects (e.g., leaving a trail, damaging over time)
            break;

        case BALL_TYPE_ICE:
            // Ice ball effects (e.g., slowing enemies, freezing terrain)
            break;

        case BALL_TYPE_LIGHTNING:
            // Lightning ball effects (e.g., chain lightning, faster movement)
            break;

        default:
            break;
        }
    }

    // If ball is very slow, stop it completely to prevent tiny endless movement
    if (fabs(ball->speedX) < 0.1f) ball->speedX = 0;
    if (fabs(ball->speedY) < 0.1f) ball->speedY = 0;
}

/**
 * @brief Handle ball collision with walls
 *
 * @param ball Pointer to ball entity
 * @param world Pointer to game world
 * @param prevX Previous X position
 * @param prevY Previous Y position
 */
void BallHandleWallCollision(Entity* ball, World* world, float prevX, float prevY) {
    if (!ball || !world || ball->type != ENTITY_BALL) return;

    BallData* ballData = (BallData*)ball->typeData;
    if (!ballData) return;

    // Convert ball position to tile coordinates for checking nearby tiles
    int tileX = (int)(ball->x / TILE_WIDTH);
    int tileY = (int)(ball->y / TILE_HEIGHT);

    // Check tiles around the ball (3x3 grid centered on ball)
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int checkX = tileX + i;
            int checkY = tileY + j;

            // Check if this tile is a wall
            if (WorldIsWallAtPosition(world, checkX * TILE_WIDTH, checkY * TILE_HEIGHT)) {
                // Calculate the edges of the wall tile
                float tileLeft = checkX * TILE_WIDTH;
                float tileRight = tileLeft + TILE_WIDTH;
                float tileTop = checkY * TILE_HEIGHT;
                float tileBottom = tileTop + TILE_HEIGHT;

                // Handle horizontal collisions
                if (ball->y > tileTop && ball->y < tileBottom) {
                    // Ball hitting left side of wall
                    if (prevX + ballData->radius < tileLeft && ball->x + ballData->radius >= tileLeft) {
                        ball->x = tileLeft - ballData->radius;
                        ball->speedX = -ball->speedX * ballData->bounceFactor;
                    }
                    // Ball hitting right side of wall
                    else if (prevX - ballData->radius > tileRight && ball->x - ballData->radius <= tileRight) {
                        ball->x = tileRight + ballData->radius;
                        ball->speedX = -ball->speedX * ballData->bounceFactor;
                    }
                }

                // Handle vertical collisions
                if (ball->x > tileLeft && ball->x < tileRight) {
                    // Ball hitting top of wall
                    if (prevY + ballData->radius < tileTop && ball->y + ballData->radius >= tileTop) {
                        ball->y = tileTop - ballData->radius;
                        ball->speedY = -ball->speedY * ballData->bounceFactor;
                    }
                    // Ball hitting bottom of wall
                    else if (prevY - ballData->radius > tileBottom && ball->y - ballData->radius <= tileBottom) {
                        ball->y = tileBottom + ballData->radius;
                        ball->speedY = -ball->speedY * ballData->bounceFactor;
                    }
                }
            }
        }
    }

    // Keep ball within world boundaries
    int worldWidthPixels = world->width * TILE_WIDTH;
    int worldHeightPixels = world->height * TILE_HEIGHT;

    if (ball->x - ballData->radius < 0) {
        ball->x = ballData->radius;
        ball->speedX = -ball->speedX * ballData->bounceFactor;
    }
    else if (ball->x + ballData->radius > worldWidthPixels) {
        ball->x = worldWidthPixels - ballData->radius;
        ball->speedX = -ball->speedX * ballData->bounceFactor;
    }

    if (ball->y - ballData->radius < 0) {
        ball->y = ballData->radius;
        ball->speedY = -ball->speedY * ballData->bounceFactor;
    }
    else if (ball->y + ballData->radius > worldHeightPixels) {
        ball->y = worldHeightPixels - ballData->radius;
        ball->speedY = -ball->speedY * ballData->bounceFactor;
    }
}

/**
 * @brief Handle ball collision with player
 *
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 */
void BallHandlePlayerCollision(Entity* ball, Entity* player) {
    if (!ball || !player || ball->type != ENTITY_BALL || player->type != ENTITY_PLAYER) return;

    BallData* ballData = (BallData*)ball->typeData;
    if (!ballData) return;

    // Calculate distance between ball and player centers
    float dx = ball->x - player->x;
    float dy = ball->y - player->y;
    float distance = sqrtf(dx * dx + dy * dy);

    // Estimate player collision radius (use average of width and height / 2)
    float playerRadius = (player->width + player->height) / 4.0f;

    // Check for collision
    if (distance < ballData->radius + playerRadius) {
        // Normalize collision vector
        float nx = dx / distance;
        float ny = dy / distance;

        // Push ball away from player
        ball->x = player->x + nx * (ballData->radius + playerRadius);
        ball->y = player->y + ny * (ballData->radius + playerRadius);

        // Apply force to ball based on player's movement and collision direction
        float pushForce = PLAYER_PUSH_FORCE;
        ball->speedX = nx * pushForce + player->speedX * 0.5f;
        ball->speedY = ny * pushForce + player->speedY * 0.5f;

        // Cap ball speed
        float speedMagnitude = sqrtf(ball->speedX * ball->speedX + ball->speedY * ball->speedY);
        if (speedMagnitude > BALL_MAX_SPEED) {
            float scale = BALL_MAX_SPEED / speedMagnitude;
            ball->speedX *= scale;
            ball->speedY *= scale;
        }
    }
}

/**
 * @brief Render ball with appropriate effects
 *
 * @param ball Pointer to ball entity
 */
void BallRender(Entity* ball) {
    if (!ball || ball->type != ENTITY_BALL || !ball->active) return;

    BallData* ballData = (BallData*)ball->typeData;
    if (!ballData) return;

    // Draw ball
    DrawCircle((int)ball->x, (int)ball->y, ballData->radius, ballData->innerColor);

    // Draw special effects based on ball type
    if (ballData->hasSpecialEffect) {
        switch (ballData->type) {
        case BALL_TYPE_FIRE:
            // Fire effect (e.g., glow, particles)
            DrawCircleLines((int)ball->x, (int)ball->y, ballData->radius + 1, (Color) { 255, 200, 0, 150 });
            break;

        case BALL_TYPE_ICE:
            // Ice effect (e.g., frost particles)
            DrawCircleLines((int)ball->x, (int)ball->y, ballData->radius + 1, (Color) { 150, 200, 255, 150 });
            break;

        case BALL_TYPE_LIGHTNING:
            // Lightning effect (e.g., electric arcs)
            DrawCircleLines((int)ball->x, (int)ball->y, ballData->radius + 2, (Color) { 220, 220, 255, 150 });
            DrawCircleLines((int)ball->x, (int)ball->y, ballData->radius + 1, (Color) { 255, 255, 100, 200 });
            break;

        default:
            break;
        }
    }
}

/**
 * @brief Reset ball to position near player
 *
 * @param ball Pointer to ball entity
 * @param x New X position
 * @param y New Y position
 */
void BallReset(Entity* ball, float x, float y) {
    if (!ball || ball->type != ENTITY_BALL) return;

    // Reset position
    ball->x = x;
    ball->y = y;

    // Reset speed
    ball->speedX = 0;
    ball->speedY = 0;

    // Ensure ball is active
    ball->active = true;
}

/**
 * @brief Apply force to ball from a direction
 *
 * @param ball Pointer to ball entity
 * @param forceX X component of force
 * @param forceY Y component of force
 */
void BallApplyForce(Entity* ball, float forceX, float forceY) {
    if (!ball || ball->type != ENTITY_BALL || !ball->active) return;

    // Apply force to ball's speed
    ball->speedX += forceX;
    ball->speedY += forceY;

    // Cap ball speed
    float speedMagnitude = sqrtf(ball->speedX * ball->speedX + ball->speedY * ball->speedY);
    if (speedMagnitude > BALL_MAX_SPEED) {
        float scale = BALL_MAX_SPEED / speedMagnitude;
        ball->speedX *= scale;
        ball->speedY *= scale;
    }
}

/**
 * @brief Get ball-specific data from entity
 *
 * @param ball Pointer to ball entity
 * @return BallData* Pointer to ball data
 */
BallData* BallGetData(Entity* ball) {
    if (!ball || ball->type != ENTITY_BALL) return NULL;
    return (BallData*)ball->typeData;
}

/**
 * @brief Handle ball collision with enemies
 *
 * @param ball Pointer to ball entity
 * @param enemy Pointer to enemy entity
 * @return true If collision occurred
 * @return false If no collision
 */
bool BallHandleEnemyCollision(Entity* ball, Entity* enemy) {
    if (!ball || !enemy || ball->type != ENTITY_BALL || enemy->type != ENTITY_ENEMY) return false;

    BallData* ballData = (BallData*)ball->typeData;
    if (!ballData) return false;

    // Calculate distance between ball and enemy centers
    float dx = ball->x - enemy->x;
    float dy = ball->y - enemy->y;
    float distance = sqrtf(dx * dx + dy * dy);

    // Assume enemy has a collision radius similar to player
    float enemyRadius = (enemy->width + enemy->height) / 4.0f;

    // Check for collision
    if (distance < ballData->radius + enemyRadius) {
        // Normalize collision vector
        float nx = dx / distance;
        float ny = dy / distance;

        // Push ball away from enemy
        ball->x = enemy->x + nx * (ballData->radius + enemyRadius);
        ball->y = enemy->y + ny * (ballData->radius + enemyRadius);

        // Bounce ball off enemy
        float impactSpeed = ball->speedX * nx + ball->speedY * ny;

        // Only bounce if ball is moving toward enemy
        if (impactSpeed < 0) {
            // Calculate reflection vector
            ball->speedX -= 2 * impactSpeed * nx;
            ball->speedY -= 2 * impactSpeed * ny;

            // Apply bounce factor
            ball->speedX *= ballData->bounceFactor;
            ball->speedY *= ballData->bounceFactor;
        }

        // Apply special effects based on ball type
        switch (ballData->type) {
        case BALL_TYPE_FIRE:
            // Fire effect on enemy (e.g., burn damage over time)
            break;

        case BALL_TYPE_ICE:
            // Ice effect on enemy (e.g., slow movement)
            break;

        case BALL_TYPE_LIGHTNING:
            // Lightning effect on enemy (e.g., stun)
            break;

        default:
            break;
        }

        return true; // Collision occurred
    }

    return false; // No collision
}