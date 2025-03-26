#include "renderer.h"
#include "textures.h"
#include "entity.h"
#include "player.h"
#include "tile.h"
#include <stdlib.h>
#include <math.h>

static Renderer* gRenderer = NULL;

Renderer* GetRenderer(void) {
    return gRenderer;
}

void SetRenderer(Renderer* renderer) {
    gRenderer = renderer;
}

Renderer* RendererCreate(int screenWidth, int screenHeight, TextureManager* textures) {
    Renderer* renderer = (Renderer*)malloc(sizeof(Renderer));
    if (renderer) {
        renderer->screenWidth = screenWidth;
        renderer->screenHeight = screenHeight;
        renderer->textures = textures;
        SetRenderer(renderer);
    }
    return renderer;
}

void RendererDestroy(Renderer* renderer) {
    if (!renderer) return;
    if (gRenderer == renderer) gRenderer = NULL;
    free(renderer);
}

void RendererBeginFrame(Renderer* renderer) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
}

void RendererEndFrame(Renderer* renderer) {
    EndDrawing();
}

/**
 * @brief Draw HUD and UI elements
 *
 * This function draws the Heads-Up Display (HUD) and other UI elements
 * that provide game information to the player, such as health, score,
 * inventory, etc.
 *
 * @param renderer Pointer to renderer
 * @param player Pointer to player entity
 */
void RendererDrawHUD(Renderer* renderer, Entity* player) {
    if (!renderer || !player) return;

    // Get player-specific data (assuming it's a player entity)
    PlayerData* playerData = NULL;
    if (player->type == ENTITY_PLAYER) {
        playerData = (PlayerData*)player->typeData;
    }

    // Draw frame rate
    DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, WHITE);

    // Draw player position
    DrawText(
        TextFormat("Position: %.1f, %.1f", player->x, player->y),
        10, 40, 20, WHITE
    );

    // Draw health bar (if player data available)
    if (playerData) {
        // Draw health bar background
        DrawRectangle(10, 70, 200, 20, GRAY);

        // Draw health bar fill (based on current/max health)
        float healthPercent = playerData->currentHealth / playerData->maxHealth;
        DrawRectangle(10, 70, (int)(200 * healthPercent), 20, RED);

        // Draw health text
        DrawText(
            TextFormat("Health: %.0f/%.0f", playerData->currentHealth, playerData->maxHealth),
            15, 72, 16, WHITE
        );

        // Draw XP bar background (replacing mana bar)
        DrawRectangle(10, 100, 200, 20, GRAY);

        // Draw XP bar fill
        float xpPercent = playerData->currentXP / playerData->maxXP;
        DrawRectangle(10, 100, (int)(200 * xpPercent), 20, GOLD);

        // Draw XP text
        DrawText(
            TextFormat("XP: %.0f/%.0f", playerData->currentXP, playerData->maxXP),
            15, 102, 16, WHITE
        );

        // Draw player level
        DrawText(
            TextFormat("Level: %d", playerData->level),
            10, 130, 20, GOLD
        );
    }

    // Draw game controls reminder (bottom of screen)
    //DrawText("WASD: Move | SPACE: Attack | E: Interact | R: Reset", 10, GetScreenHeight() - 30, 16, WHITE);
}

/**
 * @brief Draw player sprite with animation
 *
 * This function draws the player sprite from a spritesheet based on the
 * specified source and destination coordinates.
 *
 * @param renderer Pointer to renderer
 * @param textureID Texture ID for spritesheet
 * @param sourceX Source X position in spritesheet (in tiles, not pixels)
 * @param sourceY Source Y position in spritesheet (in tiles, not pixels)
 * @param destX Destination X position on screen
 * @param destY Destination Y position on screen
 * @param tint Color tint for rendering
 */
void RendererDrawPlayerSprite(Renderer* renderer, TextureID textureID, int sourceX, int sourceY, int destX, int destY, Color tint) {
    if (!renderer) return;

    // Get texture manager
    TextureManager* textures = GetTextureManager();
    if (!textures) return;

    // Get texture info
    TextureInfo* info = TextureManagerGetInfo(textures, textureID);
    if (!info || !info->loaded) {
        // Texture not loaded, draw a placeholder rectangle
        DrawRectangle(destX, destY, SPRITE_WIDTH, SPRITE_HEIGHT, PURPLE);
        DrawRectangleLines(destX, destY, SPRITE_WIDTH, SPRITE_HEIGHT, WHITE);
        return;
    }

    // Calculate source rectangle in pixels
    Rectangle source = {
        (float)(sourceX * SPRITE_WIDTH),
        (float)(sourceY * SPRITE_HEIGHT),
        (float)SPRITE_WIDTH,
        (float)SPRITE_HEIGHT
    };

    // Calculate destination rectangle
    Rectangle dest = {
        (float)destX,
        (float)destY,
        (float)SPRITE_WIDTH,
        (float)SPRITE_HEIGHT
    };

    // Draw the sprite
    DrawTexturePro(
        info->texture,
        source,
        dest,
        (Vector2) {
        0, 0
    },  // Origin (0,0 for top-left corner)
        0.0f,             // Rotation (0 for no rotation)
        tint              // Color tint
    );

    // Debug outline if debug mode is enabled
    if (renderer->debugMode) {
        DrawRectangleLines(destX, destY, SPRITE_WIDTH, SPRITE_HEIGHT, GREEN);
    }

    // Log sprite drawing for debugging
    TraceLog(LOG_DEBUG, "Drawing player sprite at source (%d, %d), dest (%d, %d)",
        sourceX, sourceY, destX, destY);
}
/**
 * @brief Draw a tile
 */
void RendererDrawTile(Renderer* renderer, Tile* tile, int destX, int destY) {
    if (!renderer || !tile) return;

    // Get texture coordinates from tile
    int textureX = tile->textureX;
    int textureY = tile->textureY;

    // Draw the tile using the tilemap texture
    RendererDrawTileFromSheet(renderer, TEXTURE_TILEMAP, textureX, textureY, destX, destY, tile->tint);
}

/**
 * @brief Draw a tile using texture coordinates
 */
void RendererDrawTileFromSheet(Renderer* renderer, TextureID textureID, int sourceX, int sourceY, int destX, int destY, Color tint) {
    if (!renderer) return;

    TextureManager* textures = GetTextureManager();
    if (!textures) return;

    TextureInfo* info = TextureManagerGetInfo(textures, textureID);
    if (!info || !info->loaded) return;

    // Calculate source rectangle
    Rectangle source = {
        (float)(sourceX * info->tileWidth),
        (float)(sourceY * info->tileHeight),
        (float)info->tileWidth,
        (float)info->tileHeight
    };

    // Calculate destination rectangle
    Rectangle dest = {
        (float)destX,
        (float)destY,
        (float)info->tileWidth,
        (float)info->tileHeight
    };

    // Draw the tile
    DrawTexturePro(info->texture, source, dest, (Vector2) { 0, 0 }, 0.0f, tint);
}

/**
 * @brief Draw an entity sprite
 */
void RendererDrawEntity(Renderer* renderer, Entity* entity) {
    if (!renderer || !entity || !entity->active) return;

    // Basic rectangle rendering for generic entities
    // Specific entity types should use their own render functions
    DrawRectangle(
        (int)(entity->x - entity->width / 2),
        (int)(entity->y - entity->height / 2),
        (int)entity->width,
        (int)entity->height,
        entity->tint
    );
}

/**
 * @brief Draw debug information
 */
void RendererDrawDebugInfo(Renderer* renderer, Entity* entity) {
    if (!renderer || !entity || !renderer->debugMode) return;

    // Draw entity position and speed
    DrawText(
        TextFormat("Pos: %.1f, %.1f", entity->x, entity->y),
        (int)entity->x + 10,
        (int)entity->y - 20,
        10,
        WHITE
    );

    DrawText(
        TextFormat("Speed: %.1f, %.1f", entity->speedX, entity->speedY),
        (int)entity->x + 10,
        (int)entity->y - 10,
        10,
        WHITE
    );

    // Draw entity bounding box
    DrawRectangleLines(
        (int)(entity->x - entity->width / 2),
        (int)(entity->y - entity->height / 2),
        (int)entity->width,
        (int)entity->height,
        GREEN
    );
}

/**
 * @brief Draw world grid for debugging
 */
void RendererDrawGrid(Renderer* renderer, int tileWidth, int tileHeight, Color gridColor) {
    if (!renderer || !renderer->debugMode) return;

    int screenWidth = renderer->screenWidth;
    int screenHeight = renderer->screenHeight;

    // Draw vertical grid lines
    for (int x = 0; x < screenWidth; x += tileWidth) {
        DrawLine(x, 0, x, screenHeight, gridColor);
    }

    // Draw horizontal grid lines
    for (int y = 0; y < screenHeight; y += tileHeight) {
        DrawLine(0, y, screenWidth, y, gridColor);
    }
}

/**
 * @brief Draw special effects
 */
void RendererDrawEffect(Renderer* renderer, float x, float y, int effectType, Color color) {
    if (!renderer || !renderer->enableEffects) return;

    // Simple particle effect example
    switch (effectType) {
    case 0: // Simple flash
        DrawCircle((int)x, (int)y, 10.0f, color);
        break;

    case 1: // Explosion
        for (int i = 0; i < 8; i++) {
            float angle = (float)i * 45.0f * DEG2RAD;
            float dx = cosf(angle) * 15.0f;
            float dy = sinf(angle) * 15.0f;
            DrawLine((int)x, (int)y, (int)(x + dx), (int)(y + dy), color);
        }
        break;

    case 2: // Sparkle
        DrawCircleLines((int)x, (int)y, 5.0f, color);
        DrawCircleLines((int)x, (int)y, 10.0f, color);
        break;
    }
}

/**
 * @brief Set debug mode
 */
void RendererSetDebugMode(Renderer* renderer, bool enabled) {
    if (!renderer) return;
    renderer->debugMode = enabled;
}

/**
 * @brief Toggle effects rendering
 */
void RendererSetEffects(Renderer* renderer, bool enabled) {
    if (!renderer) return;
    renderer->enableEffects = enabled;
}