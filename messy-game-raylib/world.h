/**
 * @file world.h
 * @brief Game world definitions and functions
 *
 * This file defines the world structure and functions for managing
 * game levels, rooms, and world objects.
 */

#ifndef MESSY_GAME_WORLD_H
#define MESSY_GAME_WORLD_H

#include <stdbool.h>
#include "room.h"
#include "tile.h"

 /**
  * @brief World structure
  *
  * Contains all data about the game world including rooms, tiles,
  * and world state.
  */
typedef struct {
    Tile** tiles;              // 2D grid of all tiles in the world
    int width;                 // Width of world in tiles
    int height;                // Height of world in tiles
    Room** rooms;              // Array of rooms in the world
    int roomCount;             // Number of rooms
    int currentRoom;           // Index of current room
    bool isOpenWorld;          // Whether the world is an open area or room-based
    // Add more world attributes as needed
} World;

/**
 * @brief Create a new game world
 *
 * @param width World width in tiles
 * @param height World height in tiles
 * @return World* Pointer to the created world
 */
World* WorldCreate(int width, int height);

/**
 * @brief Destroy world and free resources
 *
 * @param world Pointer to world
 */
void WorldDestroy(World* world);

/**
 * @brief Update world state
 *
 * @param world Pointer to world
 * @param deltaTime Time elapsed since last update
 */
void WorldUpdate(World* world, float deltaTime);

/**
 * @brief Render the world
 *
 * @param world Pointer to world
 */
void WorldRender(World* world);

/**
 * @brief Load a world from file
 *
 * @param filename Path to world file
 * @return World* Pointer to the loaded world
 */
World* WorldLoad(const char* filename);

/**
 * @brief Save a world to file
 *
 * @param world Pointer to world
 * @param filename Path to save file
 * @return true Save successful
 * @return false Save failed
 */
bool WorldSave(World* world, const char* filename);

/**
 * @brief Check if a position has a wall
 *
 * @param world Pointer to world
 * @param x X position in world
 * @param y Y position in world
 * @return true Wall is present
 * @return false No wall
 */
bool WorldIsWallAtPosition(World* world, float x, float y);

/**
 * @brief Set tile type at position
 *
 * @param world Pointer to world
 * @param x X position in tiles
 * @param y Y position in tiles
 * @param type Tile type to set
 */
void WorldSetTileType(World* world, int x, int y, TileType type);

/**
 * @brief Convert world coordinates to tile coordinates
 *
 * @param world Pointer to world
 * @param worldX X position in world
 * @param worldY Y position in world
 * @param tileX Pointer to store tile X
 * @param tileY Pointer to store tile Y
 */
void WorldToTileCoordinates(World* world, float worldX, float worldY, int* tileX, int* tileY);

/**
 * @brief Convert tile coordinates to world coordinates
 *
 * @param world Pointer to world
 * @param tileX Tile X
 * @param tileY Tile Y
 * @param worldX Pointer to store world X
 * @param worldY Pointer to store world Y
 */
void TileToWorldCoordinates(World* world, int tileX, int tileY, float* worldX, float* worldY);

/**
 * @brief Add a room to the world
 *
 * @param world Pointer to world
 * @param room Pointer to room to add
 * @return int Index of added room or -1 if failed
 */
int WorldAddRoom(World* world, Room* room);

/**
 * @brief Change to different room
 *
 * @param world Pointer to world
 * @param roomIndex Index of room to change to
 * @return true Successful room change
 * @return false Failed room change
 */
bool WorldChangeRoom(World* world, int roomIndex);

/**
 * @brief Get visible area in the world
 *
 * @param world Pointer to world
 * @param camera Pointer to camera
 * @param startTileX Pointer to store start X tile
 * @param startTileY Pointer to store start Y tile
 * @param endTileX Pointer to store end X tile
 * @param endTileY Pointer to store end Y tile
 */
void WorldGetVisibleArea(World* world, Camera2D* camera, int* startTileX, int* startTileY, int* endTileX, int* endTileY);

/**
* @brief Draw debug visualization of collision areas
*
* This function explicitly renders all collision areas in the world.
* It is separate from normal rendering and only used in debug mode.
*
* @param world Pointer to world
*/
void DebugVisualizeCollisions(World* world);

#endif // MESSY_GAME_WORLD_H