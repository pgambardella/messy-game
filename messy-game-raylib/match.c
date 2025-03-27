/**
 * @file match.c
 * @brief Implementation of soccer match game mode
 */
#include <stdlib.h>
#include "match.h"
#include "config.h"
#include "player.h"
#include "snake_boss.h"
#include "renderer.h"

 /**
  * @brief Create a new match
  *
  * Initializes match structure with default values
  *
  * @param world Pointer to game world
  * @return Match* Pointer to the created match
  */
Match* MatchCreate(World* world) {
    Match* match = (Match*)malloc(sizeof(Match));
    if (!match) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for match");
        return NULL;
    }

    // Initialize match structure
    match->state = MATCH_STATE_PLAYING;
    match->lastScorer = GOAL_SCORER_NONE;
    match->playerScore = 0;
    match->enemyScore = 0;
    match->matchTime = MATCH_DURATION_MINUTES * 60.0f; // Convert to seconds
    match->currentTime = match->matchTime;
    match->goalCelebrationTime = 0.0f;
    match->goalCelebrationDuration = GOAL_CELEBRATION_DURATION;
    match->isActive = true;

    // Initialize goal
    MatchInitializeGoal(match, world);

    TraceLog(LOG_INFO, "Match created with %d minute duration", (int)MATCH_DURATION_MINUTES);
    return match;
}

/**
 * @brief Destroy match and free resources
 *
 * @param match Pointer to match
 */
void MatchDestroy(Match* match) {
    if (!match) return;

    // Free match structure
    free(match);
    TraceLog(LOG_INFO, "Match destroyed");
}

/**
 * @brief Format time as MM:SS
 *
 * Converts time in seconds to a formatted string (MM:SS)
 *
 * @param seconds Time in seconds
 * @param buffer String buffer to store formatted time
 * @param bufferSize Size of the buffer
 */
static void FormatMatchTime(float seconds, char* buffer, int bufferSize) {
    int minutes = (int)seconds / 60;
    int secs = (int)seconds % 60;
    snprintf(buffer, bufferSize, "%02d:%02d", minutes, secs);
}

/**
 * @brief Update playing state
 *
 * Updates match during normal gameplay
 *
 * @param match Pointer to match
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 * @param entities Pointer to array of entities
 * @param entityCount Number of entities
 * @param deltaTime Time elapsed since last update
 */
static void UpdatePlayingState(Match* match, Entity* ball, Entity* player,
    Entity** entities, int entityCount, float deltaTime) {
    // Update match time
    match->currentTime -= deltaTime;
    if (match->currentTime <= 0) {
        match->currentTime = 0;
        match->state = MATCH_STATE_FINISHED;
        TraceLog(LOG_INFO, "Match finished! Final score: Player %d - Enemy %d",
            match->playerScore, match->enemyScore);
        return;
    }

    // Check for goals
    GoalScorer scorer = MatchCheckGoal(match, ball);
    if (scorer != GOAL_SCORER_NONE) {
        MatchHandleGoal(match, scorer, ball, player, entities, entityCount);
    }
}

/**
 * @brief Update goal celebration state
 *
 * Updates match during goal celebration
 *
 * @param match Pointer to match
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 * @param entities Pointer to array of entities
 * @param entityCount Number of entities
 * @param deltaTime Time elapsed since last update
 */
static void UpdateGoalState(Match* match, Entity* ball, Entity* player,
    Entity** entities, int entityCount, float deltaTime) {
    // Update celebration timer
    match->goalCelebrationTime += deltaTime;
    if (match->goalCelebrationTime >= match->goalCelebrationDuration) {
        match->goalCelebrationTime = 0;
        match->state = MATCH_STATE_PLAYING;
        MatchResetPositions(match, ball, player, entities, entityCount);
    }
}

/**
 * @brief Update match state
 *
 * Main match update function that handles different match states
 *
 * @param match Pointer to match
 * @param ball Pointer to ball entity
 * @param player Pointer to player entity
 * @param entities Pointer to array of entities
 * @param entityCount Number of entities
 * @param deltaTime Time elapsed since last update
 */
void MatchUpdate(Match* match, Entity* ball, Entity* player, Entity** entities,
    int entityCount, float deltaTime) {
    if (!match || !ball || !player) return;

    // Debug: Check if ball has changed state
    static BallState lastBallState = BALL_STATE_NEUTRAL;
    BallData* ballData = BallGetData(ball);
    if (ballData && ballData->state != lastBallState) {
        const char* stateText = "UNKNOWN";
        if (ballData->state == BALL_STATE_PLAYER) stateText = "PLAYER (BLUE)";
        else if (ballData->state == BALL_STATE_SNAKE) stateText = "SNAKE (RED)";
        else stateText = "NEUTRAL (WHITE)";

        TraceLog(LOG_INFO, "Ball state changed to: %s", stateText);
        lastBallState = ballData->state;
    }

    // Debug: Check keyboard for direct testing of goal scoring
    if (IsKeyPressed(KEY_G)) {
        // Force a goal by the player for testing
        TraceLog(LOG_INFO, "TEST: Forcing player goal via key press");
        if (ballData) {
            ballData->state = BALL_STATE_PLAYER;
            ballData->innerColor = BLUE;
            ballData->outerColor = SKYBLUE;
        }
        MatchHandleGoal(match, GOAL_SCORER_PLAYER, ball, player, entities, entityCount);
    }

    if (IsKeyPressed(KEY_H)) {
        // Force a goal by the enemy for testing
        TraceLog(LOG_INFO, "TEST: Forcing enemy goal via key press");
        if (ballData) {
            ballData->state = BALL_STATE_SNAKE;
            ballData->innerColor = RED;
            ballData->outerColor = MAROON;
        }
        MatchHandleGoal(match, GOAL_SCORER_ENEMY, ball, player, entities, entityCount);
    }

    switch (match->state) {
    case MATCH_STATE_PLAYING:
        UpdatePlayingState(match, ball, player, entities, entityCount, deltaTime);
        break;

    case MATCH_STATE_GOAL:
        UpdateGoalState(match, ball, player, entities, entityCount, deltaTime);
        break;

    case MATCH_STATE_FINISHED:
        // Match is over, nothing to update
        break;
    }
}

/**
 * @brief Render score and time display
 *
 * Displays the current score and remaining time
 *
 * @param match Pointer to match
 */
static void RenderScoreAndTime(Match* match) {
    // Get screen dimensions
    int screenWidth = GetScreenWidth();

    // Calculate position for score display
    int scoreX = screenWidth - 150;
    int scoreY = 20;

    // Format time string
    char timeBuffer[10];
    FormatMatchTime(match->currentTime, timeBuffer, sizeof(timeBuffer));

    // Draw score background
    DrawRectangle(scoreX - 10, scoreY - 10, 140, 60, Fade(BLACK, 0.5f));

    // Draw score text
    DrawText(TextFormat("SCORE: %d - %d", match->playerScore, match->enemyScore),
        scoreX, scoreY, 20, WHITE);

    // Draw time text
    DrawText(timeBuffer, scoreX + 30, scoreY + 30, 20, WHITE);
}

/**
 * @brief Render match finished display
 *
 * Shows match over message when game ends
 *
 * @param match Pointer to match
 */
static void RenderMatchFinished(Match* match) {
    // Get screen dimensions
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    const char* finishedText = "MATCH FINISHED!";
    int textWidth = MeasureText(finishedText, 40);

    // Draw text with background
    DrawRectangle(screenWidth / 2 - textWidth / 2 - 10, screenHeight / 2 - 30,
        textWidth + 20, 60, Fade(BLACK, 0.7f));
    DrawText(finishedText, screenWidth / 2 - textWidth / 2, screenHeight / 2 - 20, 40, WHITE);

    // Draw final score
    const char* scoreText = TextFormat("Final Score: %d - %d",
        match->playerScore, match->enemyScore);
    int scoreTextWidth = MeasureText(scoreText, 30);
    DrawText(scoreText, screenWidth / 2 - scoreTextWidth / 2,
        screenHeight / 2 + 30, 30, WHITE);
}

/**
 * @brief Render match UI elements
 *
 * @param match Pointer to match
 * @param ball Pointer to ball entity
 */
void MatchRenderUI(Match* match, Entity* ball) {
    if (!match) return;

    // Draw score and time
    RenderScoreAndTime(match);

    // Draw goal with custom colors
    // First draw the green goal area (interior)
    Color goalAreaColor = (Color){ 50, 205, 50, 255 }; // Vivid green
    DrawRectangleRec(match->goal.netEntrance, goalAreaColor);

    // Then draw the white frame/posts (border)
    Color goalFrameColor = WHITE;
    float frameThickness = 1.0f;

    // Draw the goal outline
    //DrawRectangleLinesEx(match->goal.area, frameThickness, goalFrameColor);

    /*/ Draw the middle goal line
    float middleY = match->goal.position.y + match->goal.height * TILE_HEIGHT / 2.0f;
    DrawLineEx(
        (Vector2) {
        match->goal.position.x + TILE_WIDTH, middleY
    },
        (Vector2) {
        match->goal.position.x + match->goal.width * TILE_WIDTH - TILE_WIDTH, middleY
    },
        frameThickness, goalFrameColor
    );//*/

    // Add arrow pointing to the goal from the bottom to show entry point
    float arrowX = match->goal.position.x + match->goal.width * TILE_WIDTH / 2.0f;
    float arrowY = match->goal.position.y + match->goal.height * TILE_HEIGHT + 30;
    float arrowLength = 25.0f;
    float arrowWidth = 15.0f;

    // Draw arrow shaft
    DrawLineEx(
        (Vector2) {
        arrowX, arrowY
    },
        (Vector2) {
        arrowX, arrowY - arrowLength
    },
        frameThickness, YELLOW
    );

    // Draw arrow head
    DrawTriangle(
        (Vector2) {
        arrowX, arrowY - arrowLength - 10
    },
        (Vector2) {
        arrowX - arrowWidth / 2, arrowY - arrowLength + 5
    },
        (Vector2) {
        arrowX + arrowWidth / 2, arrowY - arrowLength + 5
    },
        YELLOW
    );

    // Add text to identify the goal
    const char* goalText = "GOAL AREA";
    int fontSize = 20;
    DrawText(goalText,
        (int)(match->goal.position.x + match->goal.width * TILE_WIDTH / 2 - MeasureText(goalText, fontSize) / 2),
        (int)(match->goal.position.y - 25),
        fontSize, goalFrameColor);

    // Display ball state if available
    if (ball) {
        BallData* ballData = BallGetData(ball);
        if (ballData) {
            const char* stateText = "BALL: NEUTRAL";
            Color stateColor = WHITE;

            if (ballData->state == BALL_STATE_PLAYER) {
                stateText = "BALL: PLAYER (BLUE)";
                stateColor = BLUE;
            }
            else if (ballData->state == BALL_STATE_SNAKE) {
                stateText = "BALL: ENEMY (RED)";
                stateColor = RED;
            }

            // Draw ball state indicator near the ball
            DrawText(stateText, 10, 150, 20, stateColor);

            // Draw line from ball to goal to help visualize
            DrawLine((int)ball->x, (int)ball->y,
                (int)(match->goal.netEntrance.x + match->goal.netEntrance.width / 2),
                (int)(match->goal.netEntrance.y + match->goal.netEntrance.height / 2),
                Fade(stateColor, 0.5f));
        }
    }

    // Render goal celebration if active
    if (match->state == MATCH_STATE_GOAL) {
        MatchRenderGoalCelebration(match);
    }

    // Render "MATCH FINISHED" if the match is over
    if (match->state == MATCH_STATE_FINISHED) {
        RenderMatchFinished(match);
    }
}
/**
 * @brief Check if a goal was scored
 *
 * @param match Pointer to match
 * @param ball Pointer to ball entity
 * @return GoalScorer Who scored the goal (or GOAL_SCORER_NONE)
 */
GoalScorer MatchCheckGoal(Match* match, Entity* ball) {
    if (!match || !ball || match->state != MATCH_STATE_PLAYING) return GOAL_SCORER_NONE;

    // Get ball data
    BallData* ballData = BallGetData(ball);
    if (!ballData) return GOAL_SCORER_NONE;

    // Create a circle representing the ball for collision detection
    Vector2 ballPosition = { ball->x, ball->y };
    float ballRadius = ballData->radius;

    // Check if ball is inside goal area (for the horizontally centered goal)
    if (CheckCollisionCircleRec(ballPosition, ballRadius, match->goal.netEntrance)) {
        // Make sure the ball is fully inside the goal area
        // This prevents counting goals when the ball just touches the outside edge
        float goalCenterX = match->goal.position.x + (match->goal.width * TILE_WIDTH) / 2.0f;
        float goalCenterY = match->goal.position.y + (match->goal.height * TILE_HEIGHT) / 2.0f;//*/

        // Only count as goal if ball is within certain distance of goal center
        float distanceToGoalCenter = sqrtf(powf(ball->x - goalCenterX, 2) + powf(ball->y - goalCenterY, 2));
        float minDistance = (match->goal.width * TILE_WIDTH + match->goal.height * TILE_HEIGHT) / 5.0f;

        if (distanceToGoalCenter < minDistance) {
            // Determine who scored based on ball state
            if (ballData->state == BALL_STATE_PLAYER) {
                TraceLog(LOG_INFO, "PLAYER SCORED A GOAL!");
                return GOAL_SCORER_PLAYER;
            }
            else if (ballData->state == BALL_STATE_SNAKE) {
                TraceLog(LOG_INFO, "ENEMY SCORED A GOAL!");
                return GOAL_SCORER_ENEMY;
            }
            else {
                // Neutral ball just gets reset
                TraceLog(LOG_INFO, "Neutral ball in goal, resetting");
                float centerX = WORLD_WIDTH * TILE_WIDTH / 2.0f;  // Center of world
                float centerY = WORLD_HEIGHT * TILE_HEIGHT / 2.0f - 50; // Slightly above center
                BallReset(ball, centerX, centerY);
                return GOAL_SCORER_NONE;
            }
        }
    }

    return GOAL_SCORER_NONE;
}

/**
 * @brief Apply damage to all enemies
 *
 * Applies damage to all enemy entities when player scores
 *
 * @param damage Amount of damage to apply
 * @param entities Array of entities
 * @param entityCount Number of entities in the array
 */
static void ApplyDamageToEnemies(float damage, Entity** entities, int entityCount) {
    if (!entities) return;

    for (int i = 0; i < entityCount; i++) {
        Entity* entity = entities[i];
        if (!entity || entity->type != ENTITY_ENEMY) continue;

        // Check if it's a snake boss
        if (IsSnakeBoss(entity)) {
            // Make snake boss shrink multiple times based on damage
            SnakeBossData* bossData = SnakeBossGetData(entity);
            if (!bossData) continue;

            int shrinkCount = (int)(damage / 10.0f);  // Shrink once per 10 damage points
            for (int j = 0; j < shrinkCount && bossData->segmentCount > 1; j++) {
                SnakeBossShrink(entity);
            }

            TraceLog(LOG_INFO, "Applied goal damage to snake boss, shrunk %d segments", shrinkCount);
        }
        // Add handling for other enemy types here as needed
    }
}

/**
 * @brief Update score and apply effects
 *
 * Updates score and applies appropriate damage effects
 *
 * @param match Pointer to match
 * @param scorer Who scored the goal
 * @param player Pointer to player entity
 * @param entities Pointer to array of entities
 * @param entityCount Number of entities
 */
static void UpdateScoreAndApplyEffects(Match* match, GoalScorer scorer, Entity* player,
    Entity** entities, int entityCount) {
    if (scorer == GOAL_SCORER_PLAYER) {
        match->playerScore++;

        // Apply damage to all enemies
        ApplyDamageToEnemies(GOAL_PLAYER_SCORE_DAMAGE, entities, entityCount);

    }
    else if (scorer == GOAL_SCORER_ENEMY) {
        match->enemyScore++;

        // Apply damage to player
        PlayerData* playerData = PlayerGetData(player);
        if (playerData) {
            playerData->currentHealth -= GOAL_ENEMY_SCORE_DAMAGE;
            if (playerData->currentHealth < 0) {
                playerData->currentHealth = 0;
            }
            TraceLog(LOG_INFO, "Player took goal damage! Health: %.1f", playerData->currentHealth);
        }
    }
}

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
    Entity* player, Entity** entities, int entityCount) {
    if (!match || !ball || !player) return;

    // Update match state
    match->state = MATCH_STATE_GOAL;
    match->lastScorer = scorer;
    match->goalCelebrationTime = 0.0f;

    // Update score and apply damage effects
    UpdateScoreAndApplyEffects(match, scorer, player, entities, entityCount);

    // Log detailed information
    TraceLog(LOG_INFO, "GOAL SCORED! Scorer: %s, Ball State: %s",
        (scorer == GOAL_SCORER_PLAYER) ? "PLAYER" : "ENEMY",
        (BallGetData(ball)->state == BALL_STATE_PLAYER) ? "PLAYER (BLUE)" :
        ((BallGetData(ball)->state == BALL_STATE_SNAKE) ? "SNAKE (RED)" : "NEUTRAL"));

    TraceLog(LOG_INFO, "Goal handled! Score: Player %d - Enemy %d",
        match->playerScore, match->enemyScore);
}

/**
 * @brief Reset snake boss position
 *
 * Places snake boss in its starting position
 *
 * @param entity Pointer to snake boss entity
 * @param centerX Center X position
 * @param centerY Center Y position
 */
static void ResetSnakeBossPosition(Entity* entity, float centerX, float centerY) {
    SnakeBossData* bossData = SnakeBossGetData(entity);
    if (!bossData || bossData->segmentCount <= 0) return;

    // Position snake boss to the right of center
    int gridX = (int)((centerX + 50) / TILE_WIDTH);
    int gridY = (int)(centerY / TILE_HEIGHT);

    // Update grid position of head
    if (bossData->segmentCount > 0) {
        bossData->segments[0].gridX = gridX;
        bossData->segments[0].gridY = gridY;

        // Position rest of body segments to the right of head
        for (int j = 1; j < bossData->segmentCount; j++) {
            bossData->segments[j].gridX = gridX + j;
            bossData->segments[j].gridY = gridY;
        }

        // Update world coordinates
        SnakeBossUpdateSegments(entity);
    }
}

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
    Entity** entities, int entityCount) {
    if (!match || !ball || !player) return;

    // Calculate center of world/play area
    float centerX = WORLD_WIDTH * TILE_WIDTH / 2.0f;
    float centerY = WORLD_HEIGHT * TILE_HEIGHT / 2.0f;

    // Calculate positions to avoid starting inside the goal
    float goalLeftX = match->goal.position.x;
    float goalRightX = match->goal.position.x + match->goal.width * TILE_WIDTH;
    float goalTopY = match->goal.position.y;
    float goalBottomY = match->goal.position.y + match->goal.height * TILE_HEIGHT;

    // Reset ball to appropriate position (in front of goal)
    BallReset(ball, centerX, centerY - 50); // Place slightly above center to avoid goal

    // Reset player to left side
    PlayerReset(player, centerX - 100, centerY);

    // Reset enemies to right side
    for (int i = 0; i < entityCount; i++) {
        Entity* entity = entities[i];
        if (!entity || entity->type != ENTITY_ENEMY) continue;

        // Check if it's a snake boss
        if (IsSnakeBoss(entity)) {
            ResetSnakeBossPosition(entity, centerX, centerY + 100); // Below center
        }
        // Add handling for other enemy types here as needed
    }

    TraceLog(LOG_INFO, "Reset positions after goal");
}

/**
 * @brief Initialize the goal in the world
 *
 * @param match Pointer to match
 * @param world Pointer to world
 */
void MatchInitializeGoal(Match* match, World* world) {
    if (!match || !world) return;

    // Use the config values for dimensions
    int goalWidthTiles = GOAL_WIDTH_TILES;
    int goalHeightTiles = GOAL_HEIGHT_TILES;

    // Position at the top of the visible area
    int goalX = (world->width - goalWidthTiles) / 2;  // Center horizontally
    int goalY = goalHeightTiles + 15;  // top of screen

    // Store goal position and dimensions
    match->goal.position = (Vector2){ goalX * TILE_WIDTH, goalY * TILE_HEIGHT };
    match->goal.width = goalWidthTiles;
    match->goal.height = goalHeightTiles;

    // Full rectangular area of the goal
    match->goal.area = (Rectangle){
        match->goal.position.x,
        match->goal.position.y,
        goalWidthTiles * TILE_WIDTH,
        goalHeightTiles * TILE_HEIGHT
    };

    /*/ entrance area
    match->goal.netEntrance = (Rectangle){
        match->goal.position.x,
        match->goal.position.y,
        goalWidthTiles* TILE_WIDTH,
        goalHeightTiles* TILE_HEIGHT
    };//*/

    // The entrance is the area between the goal posts (excluding the frame)
    match->goal.netEntrance = (Rectangle){
        (goalX + 1) * TILE_WIDTH,              // One tile in from left
        (goalY + 1) * TILE_HEIGHT,             // One tile down from top
        (goalWidthTiles - 2) * TILE_WIDTH,     // Width minus two tiles for posts
        (goalHeightTiles - 2) * TILE_HEIGHT    // Height minus two tiles for crossbars
    };//*/

    // Clear any existing walls in this area
    for (int x = 0; x < world->width; x++) {
        for (int y = 0; y < world->height; y++) {
            // Only clear walls in the area we'll use for the goal
            if (x >= goalX && x < goalX + goalWidthTiles &&
                y >= goalY && y < goalY + goalHeightTiles) {
                WorldSetTileType(world, x, y, TILE_TYPE_EMPTY);
            }
        }
    }

    // Draw the goal frame with walls
    // Top crossbar
    for (int x = goalX; x < goalX + goalWidthTiles; x++) {
        WorldSetTileType(world, x, goalY, TILE_TYPE_WALL);
    }

    // Bottom crossbar
    for (int x = goalX; x < goalX + goalWidthTiles; x++) {
        WorldSetTileType(world, x, goalY + goalHeightTiles - 1, TILE_TYPE_WALL);
    }

    // Left post
    for (int y = goalY; y < goalY + goalHeightTiles; y++) {
        WorldSetTileType(world, goalX, y, TILE_TYPE_WALL);
    }

    // Right post
    for (int y = goalY; y < goalY + goalHeightTiles; y++) {
        WorldSetTileType(world, goalX + goalWidthTiles - 1, y, TILE_TYPE_WALL);
    }

    // Additional visual elements - goal line (middle)
    for (int x = goalX + 2; x < goalX + goalWidthTiles - 2; x++) {
        WorldSetTileType(world, x, goalY + goalHeightTiles / 2, TILE_TYPE_WALL);
    }

    TraceLog(LOG_INFO, "Goal initialized at (%d,%d) with size %dx%d tiles",
        goalX, goalY, goalWidthTiles, goalHeightTiles);
    TraceLog(LOG_INFO, "Goal world position: (%.1f,%.1f), size: %.1f x %.1f pixels",
        match->goal.position.x, match->goal.position.y,
        goalWidthTiles * TILE_WIDTH, goalHeightTiles * TILE_HEIGHT);

    // Extremely important debugging info - log the exact goal net entrance coordinates
    TraceLog(LOG_INFO, "GOAL NET ENTRANCE: (%.1f,%.1f,%.1f,%.1f)",
        match->goal.netEntrance.x, match->goal.netEntrance.y,
        match->goal.netEntrance.width, match->goal.netEntrance.height);
}
/**
 * @brief Draw celebration text
 *
 * Renders the "GOOOOOL!" text and scorer info
 *
 * @param match Pointer to match
 * @param shouldShow Whether text should be shown (for blinking effect)
 */
static void DrawCelebrationText(Match* match, bool shouldShow) {
    if (!shouldShow) return;

    // Get screen dimensions
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Draw big "GOOOOOL!" text
    const char* goalText = "GOOOOOL!";
    int fontSize = 60;
    int textWidth = MeasureText(goalText, fontSize);

    // Draw text with background
    DrawRectangle(screenWidth / 2 - textWidth / 2 - 10, screenHeight / 2 - 40,
        textWidth + 20, 80, Fade(BLACK, 0.7f));
    DrawText(goalText, screenWidth / 2 - textWidth / 2, screenHeight / 2 - 30, fontSize, YELLOW);

    // Draw scorer info
    const char* scorerText;
    if (match->lastScorer == GOAL_SCORER_PLAYER) {
        scorerText = "Player Scores!";
    }
    else {
        scorerText = "Enemy Scores!";
    }

    int scorerTextWidth = MeasureText(scorerText, 30);
    DrawText(scorerText, screenWidth / 2 - scorerTextWidth / 2,
        screenHeight / 2 + 40, 30, WHITE);
}

/**
 * @brief Draw celebration particles
 *
 * Renders particles for goal celebration effect
 *
 * @param match Pointer to match
 */
static void DrawCelebrationParticles(Match* match) {
    // Get screen dimensions
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Draw celebration particles
    for (int i = 0; i < 20; i++) {
        float x = GetRandomValue(0, screenWidth);
        float y = GetRandomValue(0, screenHeight);
        Color particleColor;

        if (match->lastScorer == GOAL_SCORER_PLAYER) {
            particleColor = BLUE;
        }
        else {
            particleColor = RED;
        }

        DrawCircle((int)x, (int)y, GetRandomValue(2, 5), particleColor);
    }
}

/**
 * @brief Render goal celebration effects
 *
 * @param match Pointer to match
 */
void MatchRenderGoalCelebration(Match* match) {
    if (!match || match->state != MATCH_STATE_GOAL) return;

    // Calculate blink speed (3 blinks per second)
    bool shouldShow = ((int)(match->goalCelebrationTime * 6) % 2) == 0;

    // Draw celebration text
    DrawCelebrationText(match, shouldShow);

    // Draw celebration particles
    DrawCelebrationParticles(match);
}

/**
 * @brief Initialize world layout with goal
 *
 * Helper function to set up the initial world layout including the goal
 *
 * @param world Pointer to world
 * @param camera Pointer to camera
 * @param match Pointer to match
 */
void InitializeWorldLayoutWithGoal(World* world, GameCamera* camera, Match* match) {
    if (!world) return;

    // First, initialize the standard world layout
    InitializeWorldLayout(world, camera);

    // Then, if we have a match, initialize the goal
    // Important: this must come after InitializeWorldLayout to override any walls
    if (match) {
        MatchInitializeGoal(match, world);
        TraceLog(LOG_INFO, "World layout with goal initialized successfully");
    }
}