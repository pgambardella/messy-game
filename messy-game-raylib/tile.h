/**
 * @file tile.h
 * @brief Tile definitions and functions
 *
 * This file defines the tile structure and functions for managing
 * individual tiles in the game world.
 */

#ifndef MESSY_GAME_TILE_H
#define MESSY_GAME_TILE_H

#include "raylib.h"

 /**
  * @brief Tile types enumeration
  *
  * Defines different types of tiles with unique properties.
  */
typedef enum {
    TILE_TYPE_EMPTY = 0,
    TILE_TYPE_WALL = 1,
    TILE_TYPE_WATER = 2,
    TILE_TYPE_LAVA = 3,
    TILE_TYPE_ICE = 4,
    TILE_TYPE_DOOR = 5,
    TILE_TYPE_SWITCH = 6,
    // Add more tile types as needed
    TILE_TYPE_COUNT
} TileType;

/**
 * @brief Tile flags for special properties
 *
 * Bit flags that can be combined to give tiles special properties.
 */
typedef enum {
    TILE_FLAG_NONE = 0,
    TILE_FLAG_SOLID = (1 << 0),       // Cannot be passed through
    TILE_FLAG_DAMAGE = (1 << 1),      // Causes damage
    TILE_FLAG_SLIPPERY = (1 << 2),    // Reduces friction
    TILE_FLAG_TRIGGER = (1 << 3),     // Triggers an event
    TILE_FLAG_TRANSITION = (1 << 4),  // Transitions to another room
    // Add more flags as needed
} TileFlags;

/**
 * @brief Tile structure
 *
 * Represents a single tile in the game world.
 */
typedef struct {
    int x;                  // X position in tile grid
    int y;                  // Y position in tile grid
    TileType type;          // Type of tile
    int textureX;           // X position in tileset
    int textureY;           // Y position in tileset
    Color tint;             // Color tint for rendering
    unsigned int flags;     // Combination of TileFlags
    int data;               // Additional data (e.g., room ID for doors)
    // Add more tile attributes as needed
} Tile;

/**
 * @brief Create a new tile
 *
 * @param x X position in tile grid
 * @param y Y position in tile grid
 * @param type Type of tile
 * @return Tile* Pointer to the created tile
 */
Tile* TileCreate(int x, int y, TileType type);

/**
 * @brief Destroy tile and free resources
 *
 * @param tile Pointer to tile
 */
void TileDestroy(Tile* tile);

/**
 * @brief Render tile at specified position
 *
 * @param tile Pointer to tile
 * @param posX X position to render
 * @param posY Y position to render
 */
void TileRender(Tile* tile, int posX, int posY);

/**
 * @brief Set tile texture coordinates
 *
 * @param tile Pointer to tile
 * @param textureX X position in tileset
 * @param textureY Y position in tileset
 */
void TileSetTexture(Tile* tile, int textureX, int textureY);

/**
 * @brief Set tile flags
 *
 * @param tile Pointer to tile
 * @param flags Flags to set
 */
void TileSetFlags(Tile* tile, unsigned int flags);

/**
 * @brief Check if tile has specific flags
 *
 * @param tile Pointer to tile
 * @param flags Flags to check
 * @return true Tile has all specified flags
 * @return false Tile does not have all specified flags
 */
bool TileHasFlags(Tile* tile, unsigned int flags);

/**
 * @brief Get default flags for a tile type
 *
 * @param type Tile type
 * @return unsigned int Default flags for this tile type
 */
unsigned int TileGetDefaultFlags(TileType type);

/**
 * @brief Get default texture coordinates for a tile type
 *
 * @param type Tile type
 * @param textureX Pointer to store texture X
 * @param textureY Pointer to store texture Y
 */
void TileGetDefaultTexture(TileType type, int* textureX, int* textureY);

#endif // MESSY_GAME_TILE_H