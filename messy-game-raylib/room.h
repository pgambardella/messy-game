/**
 * @file room.h
 * @brief Room definitions and functions
 *
 * This file defines room structures and functions for managing
 * individual rooms or sections of the game world.
 */

#ifndef MESSY_GAME_ROOM_H
#define MESSY_GAME_ROOM_H

#include <stdbool.h>
#include "raylib.h"
#include "tile.h"

 /**
  * @brief Room types enumeration
  *
  * Defines different types of rooms with unique properties.
  */
typedef enum {
    ROOM_TYPE_NORMAL,
    ROOM_TYPE_BOSS,
    ROOM_TYPE_TREASURE,
    ROOM_TYPE_SHOP,
    ROOM_TYPE_SECRET,
    // Add more room types as needed
    ROOM_TYPE_COUNT
} RoomType;

/**
 * @brief Connection direction enumeration
 *
 * Defines directions a room can connect to other rooms.
 */
typedef enum {
    CONNECTION_NONE = 0,
    CONNECTION_NORTH = (1 << 0),
    CONNECTION_EAST = (1 << 1),
    CONNECTION_SOUTH = (1 << 2),
    CONNECTION_WEST = (1 << 3),
    // Add more directions as needed
} ConnectionDirection;

/**
 * @brief Room structure
 *
 * Represents a single room or section in the game world.
 */
typedef struct Room {
    int id;                         // Unique room ID
    RoomType type;                  // Type of room
    int x;                          // Room X position in world grid
    int y;                          // Room Y position in world grid
    int width;                      // Width of room in tiles
    int height;                     // Height of room in tiles
    Tile** tiles;                   // 2D grid of tiles in room
    unsigned int connections;       // Bitfield of ConnectionDirection
    int* connectedRooms;            // Array of connected room IDs
    bool isDiscovered;              // Whether player has discovered this room
    bool isCleared;                 // Whether room is cleared of enemies
    struct Room** exits;            // Array of pointers to connected rooms
    Rectangle bounds;               // Room bounds in world coordinates
    // Add more room attributes as needed
} Room;

/**
 * @brief Create a new room
 *
 * @param id Unique room ID
 * @param type Room type
 * @param x Room X position in world grid
 * @param y Room Y position in world grid
 * @param width Width of room in tiles
 * @param height Height of room in tiles
 * @return Room* Pointer to the created room
 */
Room* RoomCreate(int id, RoomType type, int x, int y, int width, int height);

/**
 * @brief Destroy room and free resources
 *
 * @param room Pointer to room
 */
void RoomDestroy(Room* room);

/**
 * @brief Update room state
 *
 * @param room Pointer to room
 * @param deltaTime Time elapsed since last update
 */
void RoomUpdate(Room* room, float deltaTime);

/**
 * @brief Render room
 *
 * @param room Pointer to room
 * @param camera Pointer to camera
 */
void RoomRender(Room* room, Camera2D* camera);

/**
 * @brief Set tile at position in room
 *
 * @param room Pointer to room
 * @param x X position in room
 * @param y Y position in room
 * @param type Tile type to set
 * @return true Set successful
 * @return false Set failed (out of bounds)
 */
bool RoomSetTile(Room* room, int x, int y, TileType type);

/**
 * @brief Get tile at position in room
 *
 * @param room Pointer to room
 * @param x X position in room
 * @param y Y position in room
 * @return Tile* Pointer to tile or NULL if out of bounds
 */
Tile* RoomGetTile(Room* room, int x, int y);

/**
 * @brief Add connection to another room
 *
 * @param room Pointer to room
 * @param direction Direction of connection
 * @param connectedRoom Pointer to connected room
 * @return true Connection added successfully
 * @return false Connection failed
 */
bool RoomAddConnection(Room* room, ConnectionDirection direction, Room* connectedRoom);

/**
 * @brief Check if room has connection in direction
 *
 * @param room Pointer to room
 * @param direction Direction to check
 * @return true Connection exists
 * @return false No connection
 */
bool RoomHasConnection(Room* room, ConnectionDirection direction);

/**
 * @brief Get connected room in direction
 *
 * @param room Pointer to room
 * @param direction Direction to check
 * @return Room* Pointer to connected room or NULL if no connection
 */
Room* RoomGetConnected(Room* room, ConnectionDirection direction);

/**
 * @brief Generate room layout based on type
 *
 * @param room Pointer to room
 */
void RoomGenerateLayout(Room* room);

/**
 * @brief Load room from file
 *
 * @param filename Path to room file
 * @return Room* Pointer to loaded room
 */
Room* RoomLoad(const char* filename);

/**
 * @brief Save room to file
 *
 * @param room Pointer to room
 * @param filename Path to save file
 * @return true Save successful
 * @return false Save failed
 */
bool RoomSave(Room* room, const char* filename);

#endif // MESSY_GAME_ROOM_H