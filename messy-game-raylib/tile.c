/**
 * @file tile.c
 * @brief Implementation of tile system
 *
 * This file contains the implementation of tile-related functions for
 * creating, managing, and rendering individual tiles in the game world.
 */

#include "tile.h"
#include "config.h"
#include "textures.h"
#include "renderer.h"
#include <stdlib.h>

 /**
  * @brief Create a new tile
  *
  * Allocates and initializes a new tile with the specified properties.
  *
  * @param x X position in tile grid
  * @param y Y position in tile grid
  * @param type Type of tile
  * @return Tile* Pointer to the created tile
  */
Tile* TileCreate(int x, int y, TileType type) {
    Tile* tile = (Tile*)malloc(sizeof(Tile));
    if (!tile) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for tile");
        return NULL;
    }

    // Initialize tile properties
    tile->x = x;
    tile->y = y;
    tile->type = type;
    tile->tint = WHITE;
    tile->data = 0;

    // Set default flags based on type
    tile->flags = TileGetDefaultFlags(type);

    // Set default texture coordinates based on type
    TileGetDefaultTexture(type, &tile->textureX, &tile->textureY);

    return tile;
}

/**
 * @brief Destroy tile and free resources
 *
 * @param tile Pointer to tile
 */
void TileDestroy(Tile* tile) {
    if (!tile) return;

    // Free tile struct
    free(tile);
}

/**
 * @brief Render tile at specified position
 *
 * @param tile Pointer to tile
 * @param posX X position to render
 * @param posY Y position to render
 */
void TileRender(Tile* tile, int posX, int posY) {
    if (!tile) return;

    // Get renderer (normally would be passed as parameter)
    Renderer* renderer = GetRenderer();
    if (!renderer) return;

    // Render using the tile's texture coordinates
    RendererDrawTileFromSheet(
        renderer,
        TEXTURE_TILEMAP,
        tile->textureX,
        tile->textureY,
        posX,
        posY,
        tile->tint
    );

    // Draw additional indicators for special tile properties
    if (tile->flags & TILE_FLAG_DAMAGE) {
        // Draw damage indicator (red outline)
        DrawRectangleLines(
            posX,
            posY,
            TILE_WIDTH,
            TILE_HEIGHT,
            Fade(RED, 0.7f)
        );
    }

    if (tile->flags & TILE_FLAG_SLIPPERY) {
        // Draw slippery indicator (blue corners)
        DrawLine(posX, posY, posX + 4, posY, BLUE);
        DrawLine(posX, posY, posX, posY + 4, BLUE);

        DrawLine(posX + TILE_WIDTH - 4, posY, posX + TILE_WIDTH, posY, BLUE);
        DrawLine(posX + TILE_WIDTH, posY, posX + TILE_WIDTH, posY + 4, BLUE);

        DrawLine(posX, posY + TILE_HEIGHT, posX + 4, posY + TILE_HEIGHT, BLUE);
        DrawLine(posX, posY + TILE_HEIGHT - 4, posX, posY + TILE_HEIGHT, BLUE);

        DrawLine(posX + TILE_WIDTH - 4, posY + TILE_HEIGHT, posX + TILE_WIDTH, posY + TILE_HEIGHT, BLUE);
        DrawLine(posX + TILE_WIDTH, posY + TILE_HEIGHT - 4, posX + TILE_WIDTH, posY + TILE_HEIGHT, BLUE);
    }

    if (tile->flags & TILE_FLAG_TRIGGER) {
        // Draw trigger indicator (yellow dot in center)
        DrawCircle(
            posX + TILE_WIDTH / 2,
            posY + TILE_HEIGHT / 2,
            2,
            YELLOW
        );
    }

    if (tile->flags & TILE_FLAG_TRANSITION) {
        // Draw transition indicator (green diamond in center)
        DrawPoly(
            (Vector2) {
            posX + TILE_WIDTH / 2, posY + TILE_HEIGHT / 2
        },
            4,
            3,
            45.0f,
            GREEN
        );
    }
}

/**
 * @brief Set tile texture coordinates
 *
 * @param tile Pointer to tile
 * @param textureX X position in tileset
 * @param textureY Y position in tileset
 */
void TileSetTexture(Tile* tile, int textureX, int textureY) {
    if (!tile) return;

    tile->textureX = textureX;
    tile->textureY = textureY;
}

/**
 * @brief Set tile flags
 *
 * @param tile Pointer to tile
 * @param flags Flags to set
 */
void TileSetFlags(Tile* tile, unsigned int flags) {
    if (!tile) return;

    tile->flags = flags;
}

/**
 * @brief Check if tile has specific flags
 *
 * @param tile Pointer to tile
 * @param flags Flags to check
 * @return true Tile has all specified flags
 * @return false Tile does not have all specified flags
 */
bool TileHasFlags(Tile* tile, unsigned int flags) {
    if (!tile) return false;

    return (tile->flags & flags) == flags;
}

/**
 * @brief Get default flags for a tile type
 *
 * @param type Tile type
 * @return unsigned int Default flags for this tile type
 */
unsigned int TileGetDefaultFlags(TileType type) {
    switch (type) {
    case TILE_TYPE_EMPTY:
        return TILE_FLAG_NONE;

    case TILE_TYPE_WALL:
        return TILE_FLAG_SOLID;

    case TILE_TYPE_WATER:
        return TILE_FLAG_DAMAGE;

    case TILE_TYPE_LAVA:
        return TILE_FLAG_DAMAGE;

    case TILE_TYPE_ICE:
        return TILE_FLAG_SLIPPERY;

    case TILE_TYPE_DOOR:
        return TILE_FLAG_TRANSITION;

    case TILE_TYPE_SWITCH:
        return TILE_FLAG_TRIGGER;

    default:
        return TILE_FLAG_NONE;
    }
}

/**
 * @brief Get default texture coordinates for a tile type
 *
 * @param type Tile type
 * @param textureX Pointer to store texture X
 * @param textureY Pointer to store texture Y
 */
void TileGetDefaultTexture(TileType type, int* textureX, int* textureY) {
    if (!textureX || !textureY) return;

    // These coordinates are based on the specific tileset used in the game
    // The real implementation would use the actual coordinates from your tileset
    switch (type) {
    case TILE_TYPE_EMPTY:
        *textureX = 4;
        *textureY = 4;
        break;

    case TILE_TYPE_WALL:
        *textureX = 15;
        *textureY = 6;
        break;

    case TILE_TYPE_WATER:
        *textureX = 10;
        *textureY = 4;
        break;

    case TILE_TYPE_LAVA:
        *textureX = 10;
        *textureY = 7;
        break;

    case TILE_TYPE_ICE:
        *textureX = 7;
        *textureY = 4;
        break;

    case TILE_TYPE_DOOR:
        *textureX = 9;
        *textureY = 1;
        break;

    case TILE_TYPE_SWITCH:
        *textureX = 8;
        *textureY = 6;
        break;

    default:
        *textureX = 0;
        *textureY = 0;
        break;
    }
}