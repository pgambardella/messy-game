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

// Tile color configuration
#define TILE_FLOOR_COLOR     (Color){ 144, 238, 144, 255 }  // Light green
#define TILE_FLOOR_BORDER_COLOR (Color){ 0, 0, 0, 0 }       // Transparent
#define TILE_WALL_COLOR      (Color){ 64, 64, 64, 255 }     // Dark gray
#define TILE_WALL_BORDER_COLOR (Color){ 0, 0, 0, 0 }        // Transparent

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

// Win condition hole configuration
#define WIN_HOLE_RADIUS 12.0f                // Radius of the hole
#define WIN_HOLE_DEFAULT_X 1.0f              // Position as percentage of room width (center)
#define WIN_HOLE_DEFAULT_Y 0.5f              // Position as percentage of room height (center)
#define WIN_HOLE_COLOR DARKPURPLE			 // win condition hole

// Win condition damage settings
#define WIN_PLAYER_DAMAGE_TO_ENEMIES 50.0f   // Damage to all enemies when player scores
#define WIN_PLAYER_SEGMENTS_SNAKEBOSS 3	     // Segments to shrink from all snake bosses
#define WIN_ENEMY_DAMAGE_TO_PLAYER 30.0f     // Damage to player when enemy scores

// Win condition timers
#define WIN_NEUTRAL_BALL_HOLD_TIME 2.0f      // Time in seconds before white ball is ejected
#define WIN_FLASH_TEXT_DURATION 2.0f         // Time in seconds for "FLASH!!" text to display

// Win condition visual effects
#define WIN_THUNDER_PARTICLE_COUNT 500        // Number of particles for thunder effect
#define WIN_THUNDER_PARTICLE_SPEED 0.5f      // Speed of thunder particles
#define WIN_THUNDER_PARTICLE_SIZE 1.5f       // Size of thunder particles
#define WIN_THUNDER_PARTICLE_COLOR DARKPURPLE  // Yellow
#define WIN_FLASH_TEXT_COLOR DARKPURPLE		 // Yellow
#define WIN_FLASH_TEXT_SIZE 50               // Font size for "FLASH!!" text

#endif // MESSY_GAME_CONFIG_H