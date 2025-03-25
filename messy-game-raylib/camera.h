/**
 * @file camera.h
 * @brief Camera management system
 *
 * This file defines the camera system for the game,
 * handling different camera modes and transitions.
 */

#ifndef MESSY_GAME_CAMERA_H
#define MESSY_GAME_CAMERA_H

#include "raylib.h"
#include "entity.h"
#include "room.h"

 /**
  * @brief Camera modes enumeration
  *
  * Defines different camera behaviors.
  */
typedef enum {
    CAMERA_MODE_FOLLOW,      // Camera follows target entity
    CAMERA_MODE_STATIC,      // Camera stays fixed on a position
    CAMERA_MODE_ROOM,        // Camera shows entire room
    CAMERA_MODE_TRANSITION   // Camera is transitioning between positions
} CameraMode;

/**
 * @brief Game camera structure
 *
 * Extended camera structure with additional properties
 * for game-specific camera behaviors.
 */
typedef struct {
    Camera2D camera;         // Base raylib camera
    CameraMode mode;         // Current camera mode
    Entity* target;          // Entity to follow (if in follow mode)
    Room* currentRoom;       // Current room (if in room mode)
    Vector2 staticPosition;  // Position for static mode
    Vector2 transitionStart; // Start position for transition
    Vector2 transitionEnd;   // End position for transition
    float transitionProgress;// Progress of transition (0.0-1.0)
    float transitionDuration;// Duration of transition in seconds
    float shakeMagnitude;    // Camera shake magnitude
    float shakeTimeRemaining;// Time remaining for camera shake
    Rectangle bounds;        // Camera bounds
    float zoomLevel;         // Current zoom level
    float targetZoom;        // Target zoom level for transitions
    // Add more camera attributes as needed
} GameCamera;

/**
 * @brief Create a new game camera
 *
 * @param screenWidth Width of screen
 * @param screenHeight Height of screen
 * @param initialZoom Initial zoom level
 * @return GameCamera* Pointer to created camera
 */
GameCamera* CameraCreate(int screenWidth, int screenHeight, float initialZoom);

/**
 * @brief Destroy camera and free resources
 *
 * @param gameCamera Pointer to game camera
 */
void CameraDestroy(GameCamera* gameCamera);

/**
 * @brief Update camera state
 *
 * @param gameCamera Pointer to game camera
 * @param deltaTime Time elapsed since last update
 */
void CameraUpdate(GameCamera* gameCamera, float deltaTime);

/**
 * @brief Begin camera drawing
 *
 * @param gameCamera Pointer to game camera
 */
void CameraBeginMode(GameCamera* gameCamera);

/**
 * @brief End camera drawing
 */
void CameraEndMode(void);

/**
 * @brief Set camera to follow an entity
 *
 * @param gameCamera Pointer to game camera
 * @param target Entity to follow
 */
void CameraFollowTarget(GameCamera* gameCamera, Entity* target);

/**
 * @brief Set camera to static position
 *
 * @param gameCamera Pointer to game camera
 * @param position Position to focus on
 */
void CameraSetStatic(GameCamera* gameCamera, Vector2 position);

/**
 * @brief Set camera to room mode
 *
 * @param gameCamera Pointer to game camera
 * @param room Room to focus on
 */
void CameraSetRoom(GameCamera* gameCamera, Room* room);

/**
 * @brief Start camera transition
 *
 * @param gameCamera Pointer to game camera
 * @param target Target position
 * @param duration Duration of transition in seconds
 */
void CameraStartTransition(GameCamera* gameCamera, Vector2 target, float duration);

/**
 * @brief Apply camera shake effect
 *
 * @param gameCamera Pointer to game camera
 * @param magnitude Shake magnitude
 * @param duration Duration of shake in seconds
 */
void CameraShake(GameCamera* gameCamera, float magnitude, float duration);

/**
 * @brief Set camera zoom level
 *
 * @param gameCamera Pointer to game camera
 * @param zoom New zoom level
 * @param duration Duration of zoom transition (0 for instant)
 */
void CameraSetZoom(GameCamera* gameCamera, float zoom, float duration);

/**
 * @brief Set camera bounds
 *
 * @param gameCamera Pointer to game camera
 * @param bounds Rectangle defining camera boundaries
 */
void CameraSetBounds(GameCamera* gameCamera, Rectangle bounds);

/**
 * @brief Convert screen position to world position
 *
 * @param gameCamera Pointer to game camera
 * @param screenPos Position on screen
 * @return Vector2 Position in world
 */
Vector2 CameraScreenToWorld(GameCamera* gameCamera, Vector2 screenPos);

/**
 * @brief Convert world position to screen position
 *
 * @param gameCamera Pointer to game camera
 * @param worldPos Position in world
 * @return Vector2 Position on screen
 */
Vector2 CameraWorldToScreen(GameCamera* gameCamera, Vector2 worldPos);

#endif // MESSY_GAME_CAMERA_H