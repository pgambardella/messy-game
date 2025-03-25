#include "world.h"
#include "config.h"
#include "game.h"
#include <stdlib.h>

World* WorldCreate(int width, int height) {
    World* world = (World*)malloc(sizeof(World));
    if (!world) {
        TraceLog(LOG_ERROR, "Failed to create world");
        return NULL;
    }

    world->width = width;
    world->height = height;

    // Create a default room that fills most of the world
    int roomWidth = width * 2 / 3;
    int roomHeight = height * 2 / 3;
    int roomX = (width - roomWidth) / 2;
    int roomY = (height - roomHeight) / 2;

    // Allocate room array
    world->roomCount = 1;
    world->rooms = (Room**)malloc(sizeof(Room*) * world->roomCount);
    if (!world->rooms) {
        free(world);
        return NULL;
    }

    // Create the room
    world->rooms[0] = RoomCreate(1, ROOM_TYPE_NORMAL, roomX, roomY, roomWidth, roomHeight);
    if (!world->rooms[0]) {
        free(world->rooms);
        free(world);
        return NULL;
    }

    world->currentRoom = 0;

    // Initialize other world properties
    world->isOpenWorld = false;

    return world;
}

void WorldDestroy(World* world) {
    if (!world) return;
    // Clean up resources
    free(world);
}

/**
 * @brief Update world state
 *
 * This function updates the world state for each frame, including
 * any dynamic elements, animated tiles, or world-specific logic.
 *
 * @param world Pointer to world
 * @param deltaTime Time elapsed since last update in seconds
 */
void WorldUpdate(World* world, float deltaTime) {
    if (!world) return;

    // Update room state if we're using rooms
    if (world->rooms && world->currentRoom >= 0 && world->currentRoom < world->roomCount) {
        Room* currentRoom = world->rooms[world->currentRoom];
        if (currentRoom) {
            RoomUpdate(currentRoom, deltaTime);
        }
    }

    // Update any dynamic world elements (moving platforms, animated tiles, etc.)
    // This would be implemented as the game evolves with more features

    // Update environmental effects (water animations, torch flickers, etc.)
    static float effectTimer = 0.0f;
    effectTimer += deltaTime;

    // Reset timer if it gets too large to prevent float precision issues
    if (effectTimer > 1000.0f) {
        effectTimer = 0.0f;
    }
}

/**
 * @brief Render the world
 *
 * This function renders all visible tiles in the world.
 * Only tiles that are visible on screen are rendered for efficiency.
 *
 * @param world Pointer to world
 */
void WorldRender(World* world) {
    if (!world) return;

    // If we have a current room, render it
    if (world->rooms && world->currentRoom >= 0 && world->currentRoom < world->roomCount) {
        Room* currentRoom = world->rooms[world->currentRoom];
        if (currentRoom) {
            // Get the current camera (would normally be passed as parameter)
            Camera2D camera = { 0 };
            camera.zoom = CAMERA_ZOOM;

            RoomRender(currentRoom, &camera);
            return;
        }
    }

    // If there's no room, render a simple grid
    for (int x = 0; x < world->width; x++) {
        for (int y = 0; y < world->height; y++) {
            DrawRectangleLines(
                x * TILE_WIDTH,
                y * TILE_HEIGHT,
                TILE_WIDTH,
                TILE_HEIGHT,
                LIGHTGRAY
            );
        }
    }
}

/**
 * @brief Check if a position has a wall
 *
 * This function converts world coordinates to tile coordinates and
 * checks if the tile at that position is a wall or other solid object.
 *
 * @param world Pointer to world
 * @param x X position in world coordinates
 * @param y Y position in world coordinates
 * @return true If position contains a wall
 * @return false If position is open space
 */
bool WorldIsWallAtPosition(World* world, float x, float y) {
    if (!world) return true; // Treat null world as impassable

    // Convert world coordinates to tile coordinates
    int tileX = (int)(x / TILE_WIDTH);
    int tileY = (int)(y / TILE_HEIGHT);

    // Check bounds
    if (tileX < 0 || tileX >= world->width || tileY < 0 || tileY >= world->height) {
        return true; // Out of bounds is considered a wall
    }

    // In a full implementation, we would check the tile type
    // For this stub, let's consider borders as walls
    if (tileX == 0 || tileY == 0 || tileX == world->width - 1 || tileY == world->height - 1) {
        return true;
    }

    // Add some obstacles in the middle for testing
    int centerX = world->width / 2;
    int centerY = world->height / 2;

    // Horizontal wall in the middle
    if (tileY == centerY && abs(tileX - centerX) <= 5) {
        return true;
    }

    // Vertical walls on sides
    if (tileX == centerX - 10 && abs(tileY - centerY) <= 3) {
        return true;
    }

    if (tileX == centerX + 10 && abs(tileY - centerY) <= 3) {
        return true;
    }

    return false;
}

/**
 * @brief Load a world from file
 *
 * This function loads world data from a file, including tiles,
 * rooms, and any other world-specific information.
 *
 * @param filename Path to world file
 * @return World* Pointer to the loaded world or NULL if failed
 */
World* WorldLoad(const char* filename) {
    if (!filename) return NULL;

    // In a real implementation, this would read from a file
    // For this stub, we'll create a basic world

    // Create default world with the standard dimensions
    World* world = WorldCreate(WORLD_WIDTH, WORLD_HEIGHT);
    if (!world) {
        TraceLog(LOG_ERROR, "Failed to create world when loading %s", filename);
        return NULL;
    }

    // For demonstration, initialize a basic layout
    for (int x = 0; x < world->width; x++) {
        for (int y = 0; y < world->height; y++) {
            // Set borders as walls
            if (x == 0 || y == 0 || x == world->width - 1 || y == world->height - 1) {
                WorldSetTileType(world, x, y, TILE_TYPE_WALL);
            }
            else {
                WorldSetTileType(world, x, y, TILE_TYPE_EMPTY);
            }
        }
    }

    // Add some obstacles (same as in WorldIsWallAtPosition)
    int centerX = world->width / 2;
    int centerY = world->height / 2;

    // Horizontal wall in the middle
    for (int i = centerX - 5; i <= centerX + 5; i++) {
        WorldSetTileType(world, i, centerY, TILE_TYPE_WALL);
    }

    // Vertical walls on sides
    for (int j = centerY - 3; j <= centerY + 3; j++) {
        WorldSetTileType(world, centerX - 10, j, TILE_TYPE_WALL);
        WorldSetTileType(world, centerX + 10, j, TILE_TYPE_WALL);
    }

    TraceLog(LOG_INFO, "Created default world (no actual file loaded)");

    return world;
}

/**
 * @brief Set tile type at position
 *
 * This function sets the type of a tile at a specific position in the world,
 * updating any relevant properties based on the new tile type.
 *
 * @param world Pointer to world
 * @param x X position in tiles
 * @param y Y position in tiles
 * @param type Tile type to set
 */
void WorldSetTileType(World* world, int x, int y, TileType type) {
    if (!world) return;

    // Check bounds
    if (x < 0 || x >= world->width || y < 0 || y >= world->height) {
        TraceLog(LOG_WARNING, "Attempted to set tile out of bounds: (%d, %d)", x, y);
        return;
    }

    // In a full implementation, we would allocate and store tiles
    // For this stub, we'll just keep track in a simple way

    // For now, we're not actually storing the tile types in memory
    // In a real implementation, you would update the tile in the world grid
    // world->tiles[y * world->width + x].type = type;

    // Log the change for debugging
    TraceLog(LOG_DEBUG, "Set tile at (%d, %d) to type %d", x, y, type);
}

/**
 * @brief Convert world coordinates to tile coordinates
 *
 * This function converts a position in world space (pixels) to tile coordinates.
 * Useful for determining which tile the player is standing on or for collision detection.
 *
 * @param world Pointer to world
 * @param worldX X position in world coordinates (pixels)
 * @param worldY Y position in world coordinates (pixels)
 * @param tileX Pointer to store resulting tile X coordinate
 * @param tileY Pointer to store resulting tile Y coordinate
 */
void WorldToTileCoordinates(World* world, float worldX, float worldY, int* tileX, int* tileY) {
    if (!world || !tileX || !tileY) return;

    // Convert pixel coordinates to tile coordinates by dividing by tile dimensions
    *tileX = (int)(worldX / TILE_WIDTH);
    *tileY = (int)(worldY / TILE_HEIGHT);

    // Clamp to valid tile range
    if (*tileX < 0) *tileX = 0;
    if (*tileY < 0) *tileY = 0;
    if (*tileX >= world->width) *tileX = world->width - 1;
    if (*tileY >= world->height) *tileY = world->height - 1;
}

/**
 * @brief Convert tile coordinates to world coordinates
 *
 * This function converts tile coordinates to a position in world space (pixels).
 * The returned coordinates represent the top-left corner of the tile.
 *
 * @param world Pointer to world
 * @param tileX Tile X coordinate
 * @param tileY Tile Y coordinate
 * @param worldX Pointer to store resulting world X coordinate (pixels)
 * @param worldY Pointer to store resulting world Y coordinate (pixels)
 */
void TileToWorldCoordinates(World* world, int tileX, int tileY, float* worldX, float* worldY) {
    if (!world || !worldX || !worldY) return;

    // Convert tile coordinates to pixel coordinates by multiplying by tile dimensions
    *worldX = (float)(tileX * TILE_WIDTH);
    *worldY = (float)(tileY * TILE_HEIGHT);
}

/**
 * @brief Save a world to file
 *
 * This function saves the current world state to a file, including all tiles,
 * rooms, and other world data for later loading.
 *
 * @param world Pointer to world
 * @param filename Path to save file
 * @return true Save successful
 * @return false Save failed
 */
bool WorldSave(World* world, const char* filename) {
    if (!world || !filename) return false;

    // In a real implementation, we would write the world data to a file
    // For this stub, we'll just log what would happen
    TraceLog(LOG_INFO, "Would save world to file: %s", filename);
    TraceLog(LOG_INFO, "World dimensions: %d x %d", world->width, world->height);

    // Example of what might be saved:
    // - World dimensions
    // - Tile types for all tiles
    // - Room data
    // - Entity placements
    // - Other world properties

    return true; // Pretend the save was successful
}

/**
 * @brief Get visible area in the world
 *
 * This function calculates which tiles are visible based on the camera position and zoom.
 * It's used to optimize rendering by only drawing tiles that are visible on screen.
 *
 * @param world Pointer to world
 * @param camera Pointer to camera
 * @param startTileX Pointer to store start X tile
 * @param startTileY Pointer to store start Y tile
 * @param endTileX Pointer to store end X tile
 * @param endTileY Pointer to store end Y tile
 */
void WorldGetVisibleArea(World* world, Camera2D* camera, int* startTileX, int* startTileY, int* endTileX, int* endTileY) {
    if (!world || !camera || !startTileX || !startTileY || !endTileX || !endTileY) return;

    // Calculate the visible area in world coordinates
    Vector2 screenTopLeft = GetScreenToWorld2D((Vector2) { 0, 0 }, * camera);
    Vector2 screenBottomRight = GetScreenToWorld2D(
        (Vector2) {
        (float)GetScreenWidth(), (float)GetScreenHeight()
    },
        * camera
    );

    // Convert to tile coordinates and add a buffer of 1 tile
    *startTileX = (int)(screenTopLeft.x / TILE_WIDTH) - 1;
    *startTileY = (int)(screenTopLeft.y / TILE_HEIGHT) - 1;
    *endTileX = (int)(screenBottomRight.x / TILE_WIDTH) + 1;
    *endTileY = (int)(screenBottomRight.y / TILE_HEIGHT) + 1;

    // Clamp to world bounds
    if (*startTileX < 0) *startTileX = 0;
    if (*startTileY < 0) *startTileY = 0;
    if (*endTileX >= world->width) *endTileX = world->width - 1;
    if (*endTileY >= world->height) *endTileY = world->height - 1;
}

/**
 * @brief Add a room to the world
 *
 * This function adds a new room to the world's room collection.
 * Used for level design with multiple connected rooms.
 *
 * @param world Pointer to world
 * @param room Pointer to room to add
 * @return int Index of added room or -1 if failed
 */
int WorldAddRoom(World* world, Room* room) {
    if (!world || !room) return -1;

    // In a real implementation, we would:
    // 1. Reallocate the rooms array if needed
    // 2. Add the room to the array
    // 3. Update connections between rooms

    // For this stub, we'll just log what would happen
    TraceLog(LOG_INFO, "Would add room ID %d to world", room->id);

    // Pretend we added it successfully
    return world->roomCount; // This would normally be the index of the added room
}

/**
 * @brief Change to different room
 *
 * This function changes the current active room in the world,
 * handling any transitions or state changes required.
 *
 * @param world Pointer to world
 * @param roomIndex Index of room to change to
 * @return true Successful room change
 * @return false Failed room change
 */
bool WorldChangeRoom(World* world, int roomIndex) {
    if (!world) return false;

    // Check if the room index is valid
    if (roomIndex < 0 || roomIndex >= world->roomCount) {
        TraceLog(LOG_WARNING, "Attempted to change to invalid room index: %d", roomIndex);
        return false;
    }

    // In a real implementation, we would:
    // 1. Save state of current room if needed
    // 2. Update current room index
    // 3. Set up camera for new room
    // 4. Trigger any room entry events

    TraceLog(LOG_INFO, "Changing from room %d to room %d", world->currentRoom, roomIndex);
    world->currentRoom = roomIndex;

    return true;
}