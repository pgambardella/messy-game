/**
 * @file room.c
 * @brief Implementation of room system
 *
 * This file contains the implementation of room-related functions for
 * creating, managing, and rendering rooms in the game world.
 */

#include "room.h"
#include "config.h"
#include "camera.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

 /**
  * @brief Create a new room
  *
  * Allocates and initializes a new room with the specified properties.
  *
  * @param id Unique room ID
  * @param type Room type
  * @param x Room X position in world grid
  * @param y Room Y position in world grid
  * @param width Width of room in tiles
  * @param height Height of room in tiles
  * @return Room* Pointer to the created room or NULL if failed
  */
Room* RoomCreate(int id, RoomType type, int x, int y, int width, int height) {
    // Allocate memory for the room
    Room* room = (Room*)malloc(sizeof(Room));
    if (!room) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for room");
        return NULL;
    }

    // Initialize room properties
    room->id = id;
    room->type = type;
    room->x = x;
    room->y = y;
    room->width = width;
    room->height = height;
    room->connections = CONNECTION_NONE;
    room->isDiscovered = false;
    room->isCleared = false;

    // Set bounds rectangle
    room->bounds = (Rectangle){
        (float)(x * TILE_WIDTH),
        (float)(y * TILE_HEIGHT),
        (float)(width * TILE_WIDTH),
        (float)(height * TILE_HEIGHT)
    };

    // Allocate memory for tiles (2D grid)
    room->tiles = (Tile**)malloc(sizeof(Tile*) * width);
    if (!room->tiles) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for room tiles");
        free(room);
        return NULL;
    }

    for (int i = 0; i < width; i++) {
        room->tiles[i] = (Tile*)malloc(sizeof(Tile) * height);
        if (!room->tiles[i]) {
            // Clean up previously allocated rows
            for (int j = 0; j < i; j++) {
                free(room->tiles[j]);
            }
            free(room->tiles);
            free(room);
            TraceLog(LOG_ERROR, "Failed to allocate memory for room tiles row");
            return NULL;
        }

        // Initialize tiles
        for (int j = 0; j < height; j++) {
            room->tiles[i][j] = (Tile){
                .x = i,
                .y = j,
                .type = TILE_TYPE_EMPTY,
                .textureX = 0,
                .textureY = 0,
                .tint = WHITE,
                .flags = TILE_FLAG_NONE,
                .data = 0
            };
        }
    }

    // Allocate memory for connected rooms array
    room->connectedRooms = (int*)malloc(sizeof(int) * 4); // Max 4 connections (N,E,S,W)
    if (!room->connectedRooms) {
        // Clean up tiles
        for (int i = 0; i < width; i++) {
            free(room->tiles[i]);
        }
        free(room->tiles);
        free(room);
        TraceLog(LOG_ERROR, "Failed to allocate memory for connected rooms array");
        return NULL;
    }

    // Initialize connected rooms to -1 (no connection)
    for (int i = 0; i < 4; i++) {
        room->connectedRooms[i] = -1;
    }

    // Initialize exits array to NULL
    room->exits = (Room**)malloc(sizeof(Room*) * 4); // Max 4 exits (N,E,S,W)
    if (!room->exits) {
        // Clean up
        free(room->connectedRooms);
        for (int i = 0; i < width; i++) {
            free(room->tiles[i]);
        }
        free(room->tiles);
        free(room);
        TraceLog(LOG_ERROR, "Failed to allocate memory for room exits array");
        return NULL;
    }

    // Initialize exits to NULL
    for (int i = 0; i < 4; i++) {
        room->exits[i] = NULL;
    }

    // Generate room layout based on type
    RoomGenerateLayout(room);

    TraceLog(LOG_INFO, "Created room ID %d of type %d at position (%d, %d) with size %dx%d",
        id, type, x, y, width, height);

    return room;
}

/**
 * @brief Destroy room and free resources
 *
 * @param room Pointer to room
 */
void RoomDestroy(Room* room) {
    if (!room) return;

    // Free tiles
    if (room->tiles) {
        for (int i = 0; i < room->width; i++) {
            free(room->tiles[i]);
        }
        free(room->tiles);
    }

    // Free connected rooms array
    if (room->connectedRooms) {
        free(room->connectedRooms);
    }

    // Free exits array (but not the rooms it points to)
    if (room->exits) {
        free(room->exits);
    }

    // Free room struct
    free(room);

    TraceLog(LOG_INFO, "Destroyed room");
}

/**
 * @brief Update room state
 *
 * Updates the state of a room and its contents for the current frame.
 * Handles room-specific logic, animations, and events.
 *
 * @param room Pointer to room
 * @param deltaTime Time elapsed since last update in seconds
 */
void RoomUpdate(Room* room, float deltaTime) {
    if (!room) return;

    // In a real implementation, this would:
    // - Update the state of dynamic objects in the room
    // - Check for room-specific triggers
    // - Handle room-specific animations or effects

    // For now, just track time for potential animations
    static float roomTime = 0.0f;
    roomTime += deltaTime;

    // Reset room time if it gets too large to prevent float precision issues
    if (roomTime > 1000.0f) {
        roomTime = 0.0f;
    }

    // Nothing else to do in this stub implementation
}

/**
* @brief Render room
*
* Renders a room and all its contents to the screen.
* Only renders tiles that are visible within the camera view.
*
* @param room Pointer to room
* @param camera Pointer to Camera2D
*/
void RoomRender(Room* room, Camera2D* camera) {
    if (!room) return;

    // Calculate world position of room
    float roomX = room->x * TILE_WIDTH;
    float roomY = room->y * TILE_HEIGHT;

    // Draw floor tiles first (as a background)
    for (int x = 0; x < room->width; x++) {
        for (int y = 0; y < room->height; y++) {
            float tileX = roomX + x * TILE_WIDTH;
            float tileY = roomY + y * TILE_HEIGHT;

            // Check if this is a wall or not
            bool isWall = false;
            if (x == 0 || y == 0 || x == room->width - 1 || y == room->height - 1) {
                isWall = true; // Border walls
            }

            // Add some obstacles in middle (matching original game)
            int centerX = room->width / 2;
            int centerY = room->height / 2;

            // Horizontal wall in the middle
            if (y == centerY && abs(x - centerX) <= 5) {
                isWall = true;
            }

            // Vertical walls on sides
            if ((x == centerX - 10 || x == centerX + 10) && abs(y - centerY) <= 3) {
                isWall = true;
            }

            if (isWall) {
                // Draw wall with configured colors
                DrawRectangle(
                    (int)tileX,
                    (int)tileY,
                    TILE_WIDTH,
                    TILE_HEIGHT,
                    TILE_WALL_COLOR
                );

                // Only draw border if it's not transparent
                if (TILE_WALL_BORDER_COLOR.a > 0) {
                    DrawRectangleLines(
                        (int)tileX,
                        (int)tileY,
                        TILE_WIDTH,
                        TILE_HEIGHT,
                        TILE_WALL_BORDER_COLOR
                    );
                }
            }
            else {
                // Draw floor with configured colors
                DrawRectangle(
                    (int)tileX,
                    (int)tileY,
                    TILE_WIDTH,
                    TILE_HEIGHT,
                    TILE_FLOOR_COLOR
                );

                // Only draw border if it's not transparent
                if (TILE_FLOOR_BORDER_COLOR.a > 0) {
                    DrawRectangleLines(
                        (int)tileX,
                        (int)tileY,
                        TILE_WIDTH,
                        TILE_HEIGHT,
                        TILE_FLOOR_BORDER_COLOR
                    );
                }
            }
        }
    }

    // Draw room outline
    DrawRectangleLines(
        (int)roomX,
        (int)roomY,
        (int)(room->width * TILE_WIDTH),
        (int)(room->height * TILE_HEIGHT),
        GREEN
    );

    // Draw room ID for debugging
    DrawText(
        TextFormat("Room %d", room->id),
        (int)(roomX + (room->width * TILE_WIDTH) / 2 - 30),
        (int)(roomY + (room->height * TILE_HEIGHT) - 30),
        20,
        BLACK
    );
}
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
bool RoomSetTile(Room* room, int x, int y, TileType type) {
    if (!room || !room->tiles) return false;

    // Check bounds
    if (x < 0 || x >= room->width || y < 0 || y >= room->height) {
        TraceLog(LOG_WARNING, "Attempted to set tile outside room bounds: (%d, %d)", x, y);
        return false;
    }

    // Set tile type
    room->tiles[x][y].type = type;

    // Update tile flags based on type
    room->tiles[x][y].flags = TileGetDefaultFlags(type);

    // Update texture coordinates based on type
    int textureX, textureY;
    TileGetDefaultTexture(type, &textureX, &textureY);
    room->tiles[x][y].textureX = textureX;
    room->tiles[x][y].textureY = textureY;

    return true;
}

/**
 * @brief Get tile at position in room
 *
 * @param room Pointer to room
 * @param x X position in room
 * @param y Y position in room
 * @return Tile* Pointer to tile or NULL if out of bounds
 */
Tile* RoomGetTile(Room* room, int x, int y) {
    if (!room || !room->tiles) return NULL;

    // Check bounds
    if (x < 0 || x >= room->width || y < 0 || y >= room->height) {
        return NULL;
    }

    return &room->tiles[x][y];
}

/**
 * @brief Add connection to another room
 *
 * @param room Pointer to room
 * @param direction Direction of connection
 * @param connectedRoom Pointer to connected room
 * @return true Connection added successfully
 * @return false Connection failed
 */
bool RoomAddConnection(Room* room, ConnectionDirection direction, Room* connectedRoom) {
    if (!room || !connectedRoom) return false;

    // Check if direction is valid (not CONNECTION_NONE and only one bit set)
    if (direction == CONNECTION_NONE || (direction & (direction - 1)) != 0) {
        TraceLog(LOG_WARNING, "Invalid connection direction: %d", direction);
        return false;
    }

    // Add connection flag
    room->connections |= direction;

    // Add connected room to the appropriate slot
    int index = -1;
    switch (direction) {
    case CONNECTION_NORTH: index = 0; break;
    case CONNECTION_EAST:  index = 1; break;
    case CONNECTION_SOUTH: index = 2; break;
    case CONNECTION_WEST:  index = 3; break;
    default: break;
    }

    if (index != -1) {
        room->connectedRooms[index] = connectedRoom->id;
        room->exits[index] = connectedRoom;

        // Create door tile at the connection point
        int doorX = 0, doorY = 0;
        switch (direction) {
        case CONNECTION_NORTH:
            doorX = room->width / 2;
            doorY = 0;
            break;
        case CONNECTION_EAST:
            doorX = room->width - 1;
            doorY = room->height / 2;
            break;
        case CONNECTION_SOUTH:
            doorX = room->width / 2;
            doorY = room->height - 1;
            break;
        case CONNECTION_WEST:
            doorX = 0;
            doorY = room->height / 2;
            break;
        default:
            break;
        }

        RoomSetTile(room, doorX, doorY, TILE_TYPE_DOOR);

        TraceLog(LOG_INFO, "Added connection from room %d to room %d in direction %d",
            room->id, connectedRoom->id, direction);
        return true;
    }

    return false;
}

/**
 * @brief Check if room has connection in direction
 *
 * @param room Pointer to room
 * @param direction Direction to check
 * @return true Connection exists
 * @return false No connection
 */
bool RoomHasConnection(Room* room, ConnectionDirection direction) {
    if (!room) return false;

    return (room->connections & direction) != 0;
}

/**
 * @brief Get connected room in direction
 *
 * @param room Pointer to room
 * @param direction Direction to check
 * @return Room* Pointer to connected room or NULL if no connection
 */
Room* RoomGetConnected(Room* room, ConnectionDirection direction) {
    if (!room) return NULL;

    // Check if connection exists
    if (!RoomHasConnection(room, direction)) {
        return NULL;
    }

    // Get index based on direction
    int index = -1;
    switch (direction) {
    case CONNECTION_NORTH: index = 0; break;
    case CONNECTION_EAST:  index = 1; break;
    case CONNECTION_SOUTH: index = 2; break;
    case CONNECTION_WEST:  index = 3; break;
    default: break;
    }

    if (index != -1) {
        return room->exits[index];
    }

    return NULL;
}

/**
 * @brief Generate room layout based on type
 *
 * Creates a default layout for the room based on its type.
 *
 * @param room Pointer to room
 */
void RoomGenerateLayout(Room* room) {
    if (!room) return;

    // Create walls around the perimeter
    for (int x = 0; x < room->width; x++) {
        RoomSetTile(room, x, 0, TILE_TYPE_WALL); // Top wall
        RoomSetTile(room, x, room->height - 1, TILE_TYPE_WALL); // Bottom wall
    }

    for (int y = 0; y < room->height; y++) {
        RoomSetTile(room, 0, y, TILE_TYPE_WALL); // Left wall
        RoomSetTile(room, room->width - 1, y, TILE_TYPE_WALL); // Right wall
    }

    // Fill the rest with empty tiles
    for (int x = 1; x < room->width - 1; x++) {
        for (int y = 1; y < room->height - 1; y++) {
            RoomSetTile(room, x, y, TILE_TYPE_EMPTY);
        }
    }

    // Add type-specific features
    switch (room->type) {
    case ROOM_TYPE_NORMAL:
        // Normal room has no special features
        break;

    case ROOM_TYPE_BOSS:
        // Boss room has a larger open area with maybe some obstacles
    {
        int centerX = room->width / 2;
        int centerY = room->height / 2;

        // Add some pillars
        if (room->width > 8 && room->height > 8) {
            RoomSetTile(room, centerX - 3, centerY - 3, TILE_TYPE_WALL);
            RoomSetTile(room, centerX + 3, centerY - 3, TILE_TYPE_WALL);
            RoomSetTile(room, centerX - 3, centerY + 3, TILE_TYPE_WALL);
            RoomSetTile(room, centerX + 3, centerY + 3, TILE_TYPE_WALL);
        }
    }
    break;

    case ROOM_TYPE_TREASURE:
        // Treasure room has a special center area
    {
        int centerX = room->width / 2;
        int centerY = room->height / 2;

        // Add a special floor in the center
        if (room->width > 5 && room->height > 5) {
            for (int x = centerX - 1; x <= centerX + 1; x++) {
                for (int y = centerY - 1; y <= centerY + 1; y++) {
                    // Use a special tile type for the treasure area
                    RoomSetTile(room, x, y, TILE_TYPE_EMPTY);
                    // In a real implementation, you might have a special treasure tile type
                }
            }
        }
    }
    break;

    case ROOM_TYPE_SHOP:
        // Shop room has a counter
    {
        int centerX = room->width / 2;
        int centerY = room->height / 2;

        // Add a counter along the top
        if (room->height > 5) {
            for (int x = centerX - 2; x <= centerX + 2; x++) {
                RoomSetTile(room, x, 2, TILE_TYPE_WALL);
            }
        }
    }
    break;

    case ROOM_TYPE_SECRET:
        // Secret room might have a different floor texture
        // In a real implementation, you might have a special secret tile type
        break;

    default:
        break;
    }

    TraceLog(LOG_INFO, "Generated layout for room ID %d of type %d", room->id, room->type);
}

/**
 * @brief Load room from file
 *
 * @param filename Path to room file
 * @return Room* Pointer to loaded room
 */
Room* RoomLoad(const char* filename) {
    if (!filename) return NULL;

    // In a real implementation, this would read room data from a file
    // For this stub, we'll create a default room

    TraceLog(LOG_INFO, "Would load room from file: %s", filename);
    TraceLog(LOG_INFO, "Creating default room instead");

    // Create a default room
    Room* room = RoomCreate(1, ROOM_TYPE_NORMAL, 0, 0, 10, 10);

    return room;
}

/**
 * @brief Save room to file
 *
 * @param room Pointer to room
 * @param filename Path to save file
 * @return true Save successful
 * @return false Save failed
 */
bool RoomSave(Room* room, const char* filename) {
    if (!room || !filename) return false;

    // In a real implementation, this would write room data to a file
    // For this stub, we'll just log what would happen

    TraceLog(LOG_INFO, "Would save room ID %d to file: %s", room->id, filename);
    TraceLog(LOG_INFO, "Room dimensions: %d x %d", room->width, room->height);

    return true;
}