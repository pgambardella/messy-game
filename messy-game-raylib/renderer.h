/**
 * @file renderer.h
 * @brief Rendering system
 *
 * This file defines the rendering system for the game,
 * handling sprite rendering, effects, and debug visualization.
 */

#ifndef MESSY_GAME_RENDERER_H
#define MESSY_GAME_RENDERER_H

#include "raylib.h"
#include "entity.h"
#include "tile.h"
#include "textures.h"

 /**
  * @brief Layer depth enumeration
  *
  * Defines rendering order for different game elements.
  */
typedef enum {
    LAYER_BACKGROUND = 0,
    LAYER_FLOOR = 10,
    LAYER_OBJECTS_LOW = 20,
    LAYER_OBJECTS = 30,
    LAYER_ENTITIES = 40,
    LAYER_PLAYER = 50,
    LAYER_OBJECTS_HIGH = 60,
    LAYER_EFFECTS = 70,
    LAYER_UI = 80,
    LAYER_DEBUG = 90
} RenderLayer;

/**
 * @brief Renderer structure
 *
 * Manages rendering operations and state.
 */
typedef struct {
    int screenWidth;             // Screen width
    int screenHeight;            // Screen height
    bool debugMode;              // Whether debug rendering is enabled
    TextureManager* textures;    // Texture manager
    Color backgroundColor;       // Background color
    bool enableEffects;          // Whether to render special effects
    // Add more renderer attributes as needed
} Renderer;

/**
 * @brief Create a new renderer
 *
 * @param screenWidth Width of screen
 * @param screenHeight Height of screen
 * @param textures Texture manager
 * @return Renderer* Pointer to created renderer
 */
Renderer* RendererCreate(int screenWidth, int screenHeight, TextureManager* textures);

/**
 * @brief Destroy renderer and free resources
 *
 * @param renderer Pointer to renderer
 */
void RendererDestroy(Renderer* renderer);

/**
 * @brief Begin rendering frame
 *
 * @param renderer Pointer to renderer
 */
void RendererBeginFrame(Renderer* renderer);

/**
 * @brief End rendering frame
 *
 * @param renderer Pointer to renderer
 */
void RendererEndFrame(Renderer* renderer);

/**
 * @brief Draw a tile
 *
 * @param renderer Pointer to renderer
 * @param tile Pointer to tile
 * @param destX Destination X position
 * @param destY Destination Y position
 */
void RendererDrawTile(Renderer* renderer, Tile* tile, int destX, int destY);

/**
 * @brief Draw a tile using texture coordinates
 *
 * @param renderer Pointer to renderer
 * @param textureID Texture ID
 * @param sourceX Source X position in texture
 * @param sourceY Source Y position in texture
 * @param destX Destination X position
 * @param destY Destination Y position
 * @param tint Color tint
 */
void RendererDrawTileFromSheet(Renderer* renderer, TextureID textureID, int sourceX, int sourceY, int destX, int destY, Color tint);

/**
 * @brief Draw an entity sprite
 *
 * @param renderer Pointer to renderer
 * @param entity Pointer to entity
 */
void RendererDrawEntity(Renderer* renderer, Entity* entity);

/**
 * @brief Draw player sprite with animation
 *
 * @param renderer Pointer to renderer
 * @param textureID Texture ID
 * @param sourceX Source X position in texture
 * @param sourceY Source Y position in texture
 * @param destX Destination X position
 * @param destY Destination Y position
 * @param tint Color tint
 */
void RendererDrawPlayerSprite(Renderer* renderer, TextureID textureID, int sourceX, int sourceY, int destX, int destY, Color tint);

/**
 * @brief Draw debug information
 *
 * @param renderer Pointer to renderer
 * @param entity Pointer to entity for debug info
 */
void RendererDrawDebugInfo(Renderer* renderer, Entity* entity);

/**
 * @brief Draw world grid for debugging
 *
 * @param renderer Pointer to renderer
 * @param tileWidth Width of tiles
 * @param tileHeight Height of tiles
 * @param gridColor Color of grid
 */
void RendererDrawGrid(Renderer* renderer, int tileWidth, int tileHeight, Color gridColor);

/**
 * @brief Draw HUD and UI elements
 *
 * @param renderer Pointer to renderer
 * @param player Pointer to player entity
 */
void RendererDrawHUD(Renderer* renderer, Entity* player);

/**
 * @brief Draw special effects
 *
 * @param renderer Pointer to renderer
 * @param x X position
 * @param y Y position
 * @param effectType Type of effect
 * @param color Effect color
 */
void RendererDrawEffect(Renderer* renderer, float x, float y, int effectType, Color color);

/**
 * @brief Set debug mode
 *
 * @param renderer Pointer to renderer
 * @param enabled Whether debug mode is enabled
 */
void RendererSetDebugMode(Renderer* renderer, bool enabled);

/**
 * @brief Toggle effects rendering
 *
 * @param renderer Pointer to renderer
 * @param enabled Whether effects are enabled
 */
void RendererSetEffects(Renderer* renderer, bool enabled);

Renderer* GetRenderer(void);

#endif // MESSY_GAME_RENDERER_H