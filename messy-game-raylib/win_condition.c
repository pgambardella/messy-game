/**
 * @file win_condition.c
 * @brief Implementation of win condition system
 */

#include <stdlib.h>
#include <math.h>
#include "win_condition.h"
#include "config.h"
#include "ball.h"
#include "player.h"
#include "snake_boss.h"

 /**
  * @brief Create a new win condition
  *
  * Allocates and initializes a win condition structure.
  *
  * @param x X position of hole
  * @param y Y position of hole
  * @param radius Radius of hole
  * @return WinCondition* Pointer to created win condition
  */
WinCondition* WinConditionCreate(float x, float y, float radius) {
    WinCondition* winCondition = (WinCondition*)malloc(sizeof(WinCondition));
    if (!winCondition) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for win condition");
        return NULL;
    }

    // Initialize win condition properties
    winCondition->position = (Vector2){ x, y };
    winCondition->radius = radius;
    winCondition->state = WIN_STATE_IDLE;
    winCondition->stateTimer = 0.0f;
    winCondition->flashTextActive = false;
    winCondition->flashTextTimer = 0.0f;
    winCondition->flashTextAlpha = 0.0f;

    // Allocate memory for particles
    winCondition->particleCount = WIN_THUNDER_PARTICLE_COUNT;
    winCondition->particles = (ThunderParticle*)malloc(
        sizeof(ThunderParticle) * winCondition->particleCount
    );

    if (!winCondition->particles) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for thunder particles");
        free(winCondition);
        return NULL;
    }

    // Initialize particles as inactive
    for (int i = 0; i < winCondition->particleCount; i++) {
        winCondition->particles[i].active = false;
    }

    TraceLog(LOG_INFO, "Created win condition at (%.1f, %.1f) with radius %.1f",
        x, y, radius);
    return winCondition;
}

/**
 * @brief Destroy win condition and free resources
 *
 * @param winCondition Pointer to win condition
 */
void WinConditionDestroy(WinCondition* winCondition) {
    if (!winCondition) return;

    // Free particles
    if (winCondition->particles) {
        free(winCondition->particles);
    }

    // Free win condition
    free(winCondition);
    TraceLog(LOG_INFO, "Destroyed win condition");
}

/**
 * @brief Update thunder particles
 *
 * Updates position and lifetime of thunder particles.
 *
 * @param winCondition Pointer to win condition
 * @param deltaTime Time elapsed since last update
 */
void WinConditionUpdateThunder(WinCondition* winCondition, float deltaTime) {
    if (!winCondition) return;

    // Update each active particle
    for (int i = 0; i < winCondition->particleCount; i++) {
        ThunderParticle* particle = &winCondition->particles[i];
        if (!particle->active) continue;

        // Update position
        particle->position.x += particle->velocity.x * deltaTime;
        particle->position.y += particle->velocity.y * deltaTime;

        // Fade out
        particle->alpha -= 2.0f * deltaTime;

        // Deactivate if fully transparent
        if (particle->alpha <= 0) {
            particle->active = false;
        }
    }
}

/**
 * @brief Render thunder particles
 *
 * Draws all active thunder particles.
 *
 * @param winCondition Pointer to win condition
 */
void WinConditionRenderThunder(WinCondition* winCondition) {
    if (!winCondition) return;

    // Render each active particle
    for (int i = 0; i < winCondition->particleCount; i++) {
        ThunderParticle* particle = &winCondition->particles[i];
        if (!particle->active) continue;

        // Draw particle
        Color particleColor = WIN_THUNDER_PARTICLE_COLOR;
        particleColor.a = (unsigned char)(255 * particle->alpha);

        DrawCircle(
            (int)particle->position.x,
            (int)particle->position.y,
            particle->size,
            particleColor
        );
    }
}

/**
 * @brief Update flash text
 *
 * Updates the flash text animation and timing.
 *
 * @param winCondition Pointer to win condition
 * @param deltaTime Time elapsed since last update
 */
void WinConditionUpdateFlashText(WinCondition* winCondition, float deltaTime) {
    if (!winCondition || !winCondition->flashTextActive) return;

    // Update timer
    winCondition->flashTextTimer += deltaTime;

    // Oscillate alpha for flashing effect (using sin wave)
    float flashSpeed = 10.0f; // Hz
    winCondition->flashTextAlpha = 0.5f + 0.5f * sinf(winCondition->flashTextTimer * flashSpeed);

    // Deactivate after duration
    if (winCondition->flashTextTimer >= WIN_FLASH_TEXT_DURATION) {
        winCondition->flashTextActive = false;
    }
}

/**
 * @brief Render flash text
 *
 * Draws the "FLASH!!" text overlay.
 *
 * @param winCondition Pointer to win condition
 */
void WinConditionRenderFlashText(WinCondition* winCondition) {
    if (!winCondition || !winCondition->flashTextActive) return;

    // Calculate text position (center of screen)
    const char* text = "KRA-KOWWW!";
    int textWidth = MeasureText(text, WIN_FLASH_TEXT_SIZE);
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int textX = (screenWidth - textWidth) / 2;
    int textY = screenHeight / 1.5;

    // Draw text with current alpha
    Color textColor = WIN_FLASH_TEXT_COLOR;
    textColor.a = (unsigned char)(255 * winCondition->flashTextAlpha);

    DrawText(text, textX, textY, WIN_FLASH_TEXT_SIZE, textColor);
}

/**
 * @brief Trigger thunder effect
 *
 * Creates a lightning-like particle effect between two points.
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
) {
    if (!winCondition) return;

    // Calculate direction vector
    float dx = targetX - originX;
    float dy = targetY - originY;
    float distance = sqrtf(dx * dx + dy * dy);

    // Normalize direction
    if (distance > 0) {
        dx /= distance;
        dy /= distance;
    }

    // Activate particles in a line from origin to target
    float stepSize = distance / winCondition->particleCount;
    for (int i = 0; i < winCondition->particleCount; i++) {
        if (i % 3 != 0) continue; // Only use every third particle for a more sparse effect

        ThunderParticle* particle = &winCondition->particles[i];

        // Calculate position along the line with some randomness
        float progress = (float)i / (float)winCondition->particleCount;
        float randomOffsetX = (float)GetRandomValue(-10, 10);
        float randomOffsetY = (float)GetRandomValue(-10, 10);

        particle->position.x = originX + dx * (distance * progress) + randomOffsetX;
        particle->position.y = originY + dy * (distance * progress) + randomOffsetY;

        // Set random velocity with bias toward target
        particle->velocity.x = dx * WIN_THUNDER_PARTICLE_SPEED + (float)GetRandomValue(-20, 20) / 10.0f;
        particle->velocity.y = dy * WIN_THUNDER_PARTICLE_SPEED + (float)GetRandomValue(-20, 20) / 10.0f;

        // Set size and alpha
        particle->size = WIN_THUNDER_PARTICLE_SIZE * (1.0f - (float)GetRandomValue(0, 5) / 10.0f);
        particle->alpha = 1.0f;
        particle->active = true;
    }

    TraceLog(LOG_INFO, "Triggered thunder effect from (%.1f, %.1f) to (%.1f, %.1f)",
        originX, originY, targetX, targetY);
}

/**
 * @brief Trigger flash text
 *
 * Activates the "FLASH!!" text overlay.
 *
 * @param winCondition Pointer to win condition
 */
void WinConditionTriggerFlashText(WinCondition* winCondition) {
    if (!winCondition) return;

    // Activate flash text
    winCondition->flashTextActive = true;
    winCondition->flashTextTimer = 0.0f;
    winCondition->flashTextAlpha = 1.0f;

    TraceLog(LOG_INFO, "Triggered FLASH!! text");
}

/**
 * @brief Check if ball has fallen into hole
 *
 * @param winCondition Pointer to win condition
 * @param ball Pointer to ball entity
 * @return true If ball has fallen into hole
 * @return false If ball has not fallen into hole
 */
bool WinConditionCheckBallInHole(WinCondition* winCondition, Entity* ball) {
    if (!winCondition || !ball || ball->type != ENTITY_BALL) return false;

    // Get ball data
    BallData* ballData = BallGetData(ball);
    if (!ballData) return false;

    // Calculate distance between ball and hole
    float dx = ball->x - winCondition->position.x;
    float dy = ball->y - winCondition->position.y;
    float distance = sqrtf(dx * dx + dy * dy);

    // Ball is in hole if its center is within the hole radius
    // We subtract a small buffer from the ball radius to make sure it's visibly inside
    return distance < winCondition->radius - (ballData->radius * 0.8f);
}

/**
 * @brief Handle player scoring
 *
 * Applies effects when player scores a goal.
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
) {
    if (!winCondition || !ball || !entities) return;

    // Trigger visual effects
    WinConditionTriggerFlashText(winCondition);

    // Count enemy entities for distributing thunder effect
    int enemyCount = 0;
    for (int i = 0; i < entityCount; i++) {
        if (!entities[i] || entities[i]->type != ENTITY_ENEMY) continue;
        enemyCount++;
    }

    // Apply damage and trigger thunder for each enemy
    for (int i = 0; i < entityCount; i++) {
        if (!entities[i] || entities[i]->type != ENTITY_ENEMY) continue;

        // Skip the ball
        if (entities[i] == ball) continue;

        // Special handling for snake boss
        if (IsSnakeBoss(entities[i])) {
            SnakeBossData* bossData = SnakeBossGetData(entities[i]);
            if (bossData && bossData->state != SNAKE_STATE_DEFEATED) {
                // Make the snake shrink multiple segments
                int shrinksToApply = WIN_PLAYER_SEGMENTS_SNAKEBOSS; // Number of segments to shrink
                for (int j = 0; j < shrinksToApply; j++) {
                    if (!SnakeBossShrink(entities[i])) {
                        // Snake is defeated
                        bossData->state = SNAKE_STATE_DEFEATED;
                        break;
                    }
                }

                // Set shrinking state
                if (bossData->state != SNAKE_STATE_DEFEATED) {
                    bossData->state = SNAKE_STATE_SHRINKING;
                    bossData->shrinkTimer = 0.0f;
                }

                // Trigger thunder effect from hole to snake head
                WinConditionTriggerThunder(
                    winCondition,
                    winCondition->position.x,
                    winCondition->position.y,
                    entities[i]->x,
                    entities[i]->y
                );
            }
        }
        else {
            // Handle other enemy types if needed
            // For now, just trigger thunder
            WinConditionTriggerThunder(
                winCondition,
                winCondition->position.x,
                winCondition->position.y,
                entities[i]->x,
                entities[i]->y
            );
        }
    }

    TraceLog(LOG_INFO, "Player scored! Applied effects to %d enemies", enemyCount);
}

/**
 * @brief Handle enemy scoring
 *
 * Applies effects when an enemy scores a goal.
 *
 * @param winCondition Pointer to win condition
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 */
void WinConditionHandleEnemyScore(
    WinCondition* winCondition,
    Entity* ball,
    Entity* player
) {
    if (!winCondition || !ball || !player) return;

    // Trigger visual effects
    WinConditionTriggerFlashText(winCondition);

    // Apply damage to player
    PlayerData* playerData = PlayerGetData(player);
    if (playerData) {
        // Apply damage to player
        playerData->currentHealth -= WIN_ENEMY_DAMAGE_TO_PLAYER;
        if (playerData->currentHealth < 0) {
            playerData->currentHealth = 0;
        }

        // Trigger thunder effect from hole to player
        WinConditionTriggerThunder(
            winCondition,
            winCondition->position.x,
            winCondition->position.y,
            player->x,
            player->y
        );

        TraceLog(LOG_INFO, "Enemy scored! Player health reduced to %.1f",
            playerData->currentHealth);
    }
}

/**
 * @brief Handle neutral ball in hole
 *
 * Manages the ball while it's being held in the hole.
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
) {
    if (!winCondition || !ball) return true;

    // Update timer
    winCondition->stateTimer += deltaTime;

    // Keep ball centered in hole
    ball->x = winCondition->position.x;
    ball->y = winCondition->position.y;
    ball->speedX = 0.0f;
    ball->speedY = 0.0f;

    // Make ball neutral white
    BallData* ballData = BallGetData(ball);
    if (ballData) {
        ballData->state = BALL_STATE_NEUTRAL;
        ballData->innerColor = WHITE;
        ballData->outerColor = WHITE;
    }

    // Check if hold time has elapsed
    return winCondition->stateTimer >= WIN_NEUTRAL_BALL_HOLD_TIME;
}

/**
 * @brief Eject ball from hole
 *
 * Shoots the ball out from the hole in a random direction.
 *
 * @param winCondition Pointer to win condition
 * @param ball Pointer to ball entity
 */
void WinConditionEjectBall(WinCondition* winCondition, Entity* ball) {
    if (!winCondition || !ball) return;

    // Reset ball state
    BallData* ballData = BallGetData(ball);
    if (!ballData) return;

    // Ensure ball is neutral
    ballData->state = BALL_STATE_NEUTRAL;
    ballData->innerColor = WHITE;
    ballData->outerColor = WHITE;

    // Calculate random ejection angle
    float angle = (float)GetRandomValue(0, 359) * DEG2RAD;
    float speed = BALL_INITIAL_SPEED * 1.5f; // Slightly faster than normal

    // Apply force in random direction
    BallApplyForce(ball, cosf(angle) * speed, sinf(angle) * speed);

    // Move ball slightly outside hole to prevent immediate recapture
    ball->x = winCondition->position.x + cosf(angle) * (winCondition->radius + ballData->radius);
    ball->y = winCondition->position.y + sinf(angle) * (winCondition->radius + ballData->radius);

    TraceLog(LOG_INFO, "Ball ejected from hole at angle %.1f degrees", angle * RAD2DEG);
}

/**
 * @brief Update win condition state
 *
 * Main update function for the win condition system.
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
) {
    if (!winCondition || !ball || !player) return;

    // Update thunder particles
    WinConditionUpdateThunder(winCondition, deltaTime);

    // Update flash text
    WinConditionUpdateFlashText(winCondition, deltaTime);

    // Handle state-specific updates
    switch (winCondition->state) {
    case WIN_STATE_IDLE:
        // Check if ball has fallen into hole
        if (WinConditionCheckBallInHole(winCondition, ball)) {
            BallData* ballData = BallGetData(ball);
            if (!ballData) break;

            // Determine action based on ball state
            if (ballData->state == BALL_STATE_PLAYER) {
                // Player scored
                winCondition->state = WIN_STATE_PLAYER_SCORED;
                WinConditionHandlePlayerScore(winCondition, ball, entities, entityCount);
            }
            else if (ballData->state == BALL_STATE_SNAKE) {
                // Enemy scored
                winCondition->state = WIN_STATE_ENEMY_SCORED;
                WinConditionHandleEnemyScore(winCondition, ball, player);
            }
            else {
                // Neutral ball
                winCondition->state = WIN_STATE_NEUTRAL_HOLD;
                winCondition->stateTimer = 0.0f;
            }

            // Stop ball movement
            ball->speedX = 0.0f;
            ball->speedY = 0.0f;

            // Center ball in hole
            ball->x = winCondition->position.x;
            ball->y = winCondition->position.y;
        }
        break;

    case WIN_STATE_PLAYER_SCORED:
    case WIN_STATE_ENEMY_SCORED:
        // These states are transitional and immediately revert to NEUTRAL_HOLD
        winCondition->state = WIN_STATE_NEUTRAL_HOLD;
        winCondition->stateTimer = 0.0f;
        break;

    case WIN_STATE_NEUTRAL_HOLD:
        // Check if it's time to eject the ball
        if (WinConditionHandleNeutralBall(winCondition, ball, deltaTime)) {
            WinConditionEjectBall(winCondition, ball);
            winCondition->state = WIN_STATE_IDLE;
        }
        break;
    }
}

/**
 * @brief Render win condition
 *
 * Renders the win condition hole and any active effects.
 *
 * @param winCondition Pointer to win condition
 */
void WinConditionRender(WinCondition* winCondition) {
    if (!winCondition) return;

    // Draw the hole
    DrawCircle(
        (int)winCondition->position.x,
        (int)winCondition->position.y,
        winCondition->radius,
        WIN_HOLE_COLOR
    );

    // Draw a border around the hole
    DrawCircleLines(
        (int)winCondition->position.x,
        (int)winCondition->position.y,
        winCondition->radius,
        DARKGRAY
    );

    // Render thunder particles
    WinConditionRenderThunder(winCondition);

    // Render flash text
    WinConditionRenderFlashText(winCondition);
}