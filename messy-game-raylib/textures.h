/**
 * @file textures.h
 * @brief Texture management system
 *
 * This file defines the texture management system for the game,
 * handling loading, unloading, and accessing textures.
 */

#ifndef MESSY_GAME_TEXTURES_H
#define MESSY_GAME_TEXTURES_H

#include "raylib.h"

 /**
  * @brief Texture IDs enumeration
  *
  * Defines all available textures in the game.
  */
typedef enum {
    TEXTURE_NONE = -1,
    TEXTURE_TILEMAP = 0,
    TEXTURE_PLAYER = 1,
    TEXTURE_ENEMIES = 2,
    TEXTURE_BALLS = 3,
    TEXTURE_EFFECTS = 4,
    TEXTURE_UI = 5,
    TEXTURE_POWERUPS = 6,
    // Add more texture IDs as needed
    TEXTURE_COUNT
} TextureID;

/**
 * @brief Texture information structure
 *
 * Contains information about a specific texture.
 */
typedef struct {
    Texture2D texture;       // Raylib texture
    const char* filePath;    // Path to texture file
    bool loaded;             // Whether texture is loaded
    int tileWidth;           // Width of tiles in texture (if tileset)
    int tileHeight;          // Height of tiles in texture (if tileset)
    int columns;             // Number of columns in tileset
    int rows;                // Number of rows in tileset
    // Add more texture attributes as needed
} TextureInfo;

/**
 * @brief Texture manager structure
 *
 * Manages all game textures.
 */
typedef struct {
    TextureInfo* textures;   // Array of texture information
    int count;               // Number of textures
    int capacity;            // Capacity of textures array
    // Add more texture manager attributes as needed
} TextureManager;

/**
 * @brief Create a new texture manager
 *
 * @param initialCapacity Initial capacity for textures
 * @return TextureManager* Pointer to created texture manager
 */
TextureManager* TextureManagerCreate(int initialCapacity);

/**
 * @brief Destroy texture manager and free resources
 *
 * @param manager Pointer to texture manager
 */
void TextureManagerDestroy(TextureManager* manager);

/**
 * @brief Load a texture
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 * @param filePath Path to texture file
 * @param tileWidth Width of tiles (if tileset) or 0
 * @param tileHeight Height of tiles (if tileset) or 0
 * @return bool Whether load was successful
 */
bool TextureManagerLoad(TextureManager* manager, TextureID id, const char* filePath, int tileWidth, int tileHeight);

/**
 * @brief Unload a texture
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 */
void TextureManagerUnload(TextureManager* manager, TextureID id);

/**
 * @brief Unload all textures
 *
 * @param manager Pointer to texture manager
 */
void TextureManagerUnloadAll(TextureManager* manager);

/**
 * @brief Get texture by ID
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 * @return Texture2D Texture (invalid texture if not found)
 */
Texture2D TextureManagerGet(TextureManager* manager, TextureID id);

/**
 * @brief Get texture info by ID
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 * @return TextureInfo* Pointer to texture info or NULL if not found
 */
TextureInfo* TextureManagerGetInfo(TextureManager* manager, TextureID id);

/**
 * @brief Check if texture is loaded
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 * @return bool Whether texture is loaded
 */
bool TextureManagerIsLoaded(TextureManager* manager, TextureID id);

/**
 * @brief Get source rectangle for a tile from a tileset
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 * @param tileX Tile X position in tileset
 * @param tileY Tile Y position in tileset
 * @return Rectangle Source rectangle
 */
Rectangle TextureManagerGetTileRect(TextureManager* manager, TextureID id, int tileX, int tileY);

/**
 * @brief Load game assets
 *
 * Convenience function to load all initial game assets.
 *
 * @param manager Pointer to texture manager
 * @return bool Whether all assets were loaded successfully
 */
bool TextureManagerLoadGameAssets(TextureManager* manager);

TextureManager* GetTextureManager(void);

#endif // MESSY_GAME_TEXTURES_H