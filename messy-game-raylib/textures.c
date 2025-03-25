/**
 * @file textures.c
 * @brief Implementation of texture management system
 */

#include <stdlib.h>
#include <string.h>
#include "textures.h"
#include "config.h"

 // Singleton instance for global access
static TextureManager* gTextureManager = NULL;

/**
 * @brief Get global texture manager instance
 *
 * @return TextureManager* Pointer to the global texture manager
 */
TextureManager* GetTextureManager(void) {
    return gTextureManager;
}

/**
 * @brief Set global texture manager instance
 *
 * @param manager Pointer to texture manager
 */
void SetTextureManager(TextureManager* manager) {
    gTextureManager = manager;
}

/**
 * @brief Create a new texture manager
 *
 * @param initialCapacity Initial capacity for textures
 * @return TextureManager* Pointer to created texture manager
 */
TextureManager* TextureManagerCreate(int initialCapacity) {
    TextureManager* manager = (TextureManager*)malloc(sizeof(TextureManager));
    if (!manager) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for texture manager");
        return NULL;
    }

    // Allocate textures array
    manager->textures = (TextureInfo*)malloc(sizeof(TextureInfo) * initialCapacity);
    if (!manager->textures) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for textures");
        free(manager);
        return NULL;
    }

    // Initialize all textures as not loaded
    for (int i = 0; i < initialCapacity; i++) {
        manager->textures[i].loaded = false;
        manager->textures[i].filePath = NULL;
        manager->textures[i].tileWidth = 0;
        manager->textures[i].tileHeight = 0;
        manager->textures[i].columns = 0;
        manager->textures[i].rows = 0;
    }

    // Initialize manager
    manager->count = 0;
    manager->capacity = initialCapacity;

    // Set as global instance
    SetTextureManager(manager);

    return manager;
}

/**
 * @brief Destroy texture manager and free resources
 *
 * @param manager Pointer to texture manager
 */
void TextureManagerDestroy(TextureManager* manager) {
    if (!manager) return;

    // Unload all textures
    TextureManagerUnloadAll(manager);

    // Free resources
    free(manager->textures);

    // Clear global reference if this is the current manager
    if (gTextureManager == manager) {
        gTextureManager = NULL;
    }

    // Free manager
    free(manager);
}

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
bool TextureManagerLoad(TextureManager* manager, TextureID id, const char* filePath, int tileWidth, int tileHeight) {
    if (!manager || id < 0 || id >= TEXTURE_COUNT || !filePath) return false;

    // Unload existing texture if loaded
    if (manager->textures[id].loaded) {
        TextureManagerUnload(manager, id);
    }

    // Load texture
    Image image = LoadImage(filePath);
    if (image.data == NULL) {
        TraceLog(LOG_ERROR, "Failed to load image: %s", filePath);
        return false;
    }

    // Create texture from image
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    if (texture.id == 0) {
        TraceLog(LOG_ERROR, "Failed to create texture from image: %s", filePath);
        return false;
    }

    // Store texture info
    manager->textures[id].texture = texture;
    manager->textures[id].filePath = _strdup(filePath);
    manager->textures[id].loaded = true;
    manager->textures[id].tileWidth = tileWidth;
    manager->textures[id].tileHeight = tileHeight;

    // Calculate columns and rows if this is a tileset
    if (tileWidth > 0 && tileHeight > 0) {
        manager->textures[id].columns = texture.width / tileWidth;
        manager->textures[id].rows = texture.height / tileHeight;
    }
    else {
        manager->textures[id].columns = 1;
        manager->textures[id].rows = 1;
    }

    // Update count if needed
    if (id >= manager->count) {
        manager->count = id + 1;
    }

    return true;
}

/**
 * @brief Unload a texture
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 */
void TextureManagerUnload(TextureManager* manager, TextureID id) {
    if (!manager || id < 0 || id >= manager->count) return;

    if (manager->textures[id].loaded) {
        UnloadTexture(manager->textures[id].texture);

        if (manager->textures[id].filePath) {
            free((void*)manager->textures[id].filePath);
            manager->textures[id].filePath = NULL;
        }

        manager->textures[id].loaded = false;
        manager->textures[id].tileWidth = 0;
        manager->textures[id].tileHeight = 0;
        manager->textures[id].columns = 0;
        manager->textures[id].rows = 0;
    }
}

/**
 * @brief Unload all textures
 *
 * @param manager Pointer to texture manager
 */
void TextureManagerUnloadAll(TextureManager* manager) {
    if (!manager) return;

    for (int i = 0; i < manager->count; i++) {
        TextureManagerUnload(manager, i);
    }

    manager->count = 0;
}

/**
 * @brief Get texture by ID
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 * @return Texture2D Texture (invalid texture if not found)
 */
Texture2D TextureManagerGet(TextureManager* manager, TextureID id) {
    if (!manager || id < 0 || id >= manager->count || !manager->textures[id].loaded) {
        // Return an invalid texture
        Texture2D invalidTexture = { 0 };
        return invalidTexture;
    }

    return manager->textures[id].texture;
}

/**
 * @brief Get texture info by ID
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 * @return TextureInfo* Pointer to texture info or NULL if not found
 */
TextureInfo* TextureManagerGetInfo(TextureManager* manager, TextureID id) {
    if (!manager || id < 0 || id >= manager->count || !manager->textures[id].loaded) {
        return NULL;
    }

    return &manager->textures[id];
}

/**
 * @brief Check if texture is loaded
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 * @return bool Whether texture is loaded
 */
bool TextureManagerIsLoaded(TextureManager* manager, TextureID id) {
    if (!manager || id < 0 || id >= manager->count) return false;
    return manager->textures[id].loaded;
}

/**
 * @brief Get source rectangle for a tile from a tileset
 *
 * @param manager Pointer to texture manager
 * @param id Texture ID
 * @param tileX Tile X position in tileset
 * @param tileY Tile Y position in tileset
 * @return Rectangle Source rectangle
 */
Rectangle TextureManagerGetTileRect(TextureManager* manager, TextureID id, int tileX, int tileY) {
    Rectangle rect = { 0, 0, 0, 0 };

    if (!manager || id < 0 || id >= manager->count || !manager->textures[id].loaded) {
        return rect;
    }

    TextureInfo* info = &manager->textures[id];

    // Clamp tile coordinates to valid range
    if (info->tileWidth <= 0 || info->tileHeight <= 0) {
        // If not a tileset, return the entire texture
        rect.width = (float)info->texture.width;
        rect.height = (float)info->texture.height;
        return rect;
    }

    // Clamp to valid tile range
    if (tileX < 0) tileX = 0;
    if (tileY < 0) tileY = 0;
    if (tileX >= info->columns) tileX = info->columns - 1;
    if (tileY >= info->rows) tileY = info->rows - 1;

    // Calculate tile rectangle
    rect.x = tileX * info->tileWidth;
    rect.y = tileY * info->tileHeight;
    rect.width = info->tileWidth;
    rect.height = info->tileHeight;

    return rect;
}

/**
 * @brief Load game assets
 *
 * Convenience function to load all initial game assets.
 *
 * @param manager Pointer to texture manager
 * @return bool Whether all assets were loaded successfully
 */
bool TextureManagerLoadGameAssets(TextureManager* manager) {
    if (!manager) return false;

    bool success = true;

    // Load tilemap
    success &= TextureManagerLoad(manager, TEXTURE_TILEMAP, TILEMAP_ASSET_PATH, TILE_WIDTH, TILE_HEIGHT);

    // Load player sprites
    success &= TextureManagerLoad(manager, TEXTURE_PLAYER, PLAYER_ASSET_PATH, SPRITE_WIDTH, SPRITE_HEIGHT);

    // Add more asset loading here as needed

    return success;
}