/**
 * @file config.h
 * @brief Game-wide configuration constants and settings
 *
 * Central place for defining all game constants for easy tuning
 * and configuration across platforms.
 */

#ifndef MESSY_GAME_CONFIG_H
#define MESSY_GAME_CONFIG_H

 // Screen configuration
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 960
#define GAME_TITLE "Raylib Messy Game"
#define TARGET_FPS 60

// World configuration
#define WORLD_WIDTH 76
#define WORLD_HEIGHT 120

// Tile configuration
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define TILE_EMPTY 0
#define TILE_WALL 1

// Sprite configuration
#define SPRITE_WIDTH 16
#define SPRITE_HEIGHT 16

// camera configuration
#define CAMERA_ZOOM 1.5f

// Player physics configuration
#define PLAYER_ACCEL 7.0f
#define PLAYER_MAX_SPEED 1.5f
#define PLAYER_DECEL 2.5f
#define PLAYER_LERP_ALPHA 1.2f

// Ball physics configuration
#define BALL_RADIUS 4.0f
#define BALL_INITIAL_SPEED 2.0f
#define BALL_MAX_SPEED 8.0f
#define BALL_BOUNCE_FACTOR 0.8f
#define BALL_FRICTION 0.98f
#define PLAYER_PUSH_FORCE 5.0f

// Asset paths
#define TILEMAP_ASSET_PATH "Assets/Spritesheets/colored_tilemap_packed.PNG"
#define PLAYER_ASSET_PATH "Assets/Spritesheets/BlueKnightRunSprite-sheet16x17.png"

// Maximum number of different textures/assets
#define MAX_TEXTURES 10  // Increased for future expansion

// Animation configuration
#define ANIM_FRAME_SPEED 6  // Frames to wait before changing animation frame

#endif // MESSY_GAME_CONFIG_H