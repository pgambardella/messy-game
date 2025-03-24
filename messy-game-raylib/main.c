// main.c : Messy Game
// a Zelda clone where you are a mage that controls an enchanted ball like a PRO
#include "raylib.h"
#include "raymath.h"

#define TILE_WIDTH 8
#define TILE_HEIGHT 8

#define SPRITE_WIDTH 16
#define SPRITE_HEIGHT 16

void GameStartup();
void GameUpdate();
void GameRender();
void GameShutdown();

void DrawTile(int source_x, int source_y, int dest_x, int dest_y, Color c);
void PlayerMovement();
bool IsWallAtPosition(float x, float y);
void UpdateBall();
void DrawPlayerSprite(int source_x, int source_y, int dest_x, int dest_y, Color c);

const int screenWidth = 600;
const int screenHeight = 960;

#define MAX_TEXTURES 2

typedef enum {
	TEXTURE_TILEMAP = 0,
	TEXTURE_PLAYER = 1
} texture_asset;

Texture2D textures[MAX_TEXTURES];

#define WORLD_WIDTH 76 //* TILE_WIDTH
#define WORLD_HEIGHT 120//* TILE_HEIGHT

#define TILE_EMPTY 0
#define TILE_WALL 1

typedef struct {
	int x;
	int y;
	int type; //0 = TILE_EMPTY, 1 = TILE_WALL
} sTile;

sTile world[WORLD_WIDTH][WORLD_HEIGHT];

Camera2D camera = { 0 };

// Add near your other definitions
typedef enum {
	ANIM_IDLE_DOWN,
	ANIM_WALK_DOWN,
	ANIM_IDLE_UP,
	ANIM_WALK_UP,
	ANIM_IDLE_LEFT,
	ANIM_WALK_LEFT,
	ANIM_IDLE_RIGHT,
	ANIM_WALK_RIGHT
} PlayerAnimation;

// Add to your player structure
typedef struct {
	float x;
	float y;
	PlayerAnimation currentAnim;
	int frameCounter;
	int currentFrame;
	int facing; // 0-down, 1-up, 2-left, 3-right
} sEntity;

sEntity player;

float acceleration = 7.0f;
float maxSpeed = 1.5f;
float decel = 2.5f;
Vector2 speed = { 0.0f, 0.0f };
float lerpAlpha = 1.2f;

typedef struct {
	float x;
	float y;
	float speedX;
	float speedY;
	float radius;
	bool active;
} sBall;

// Add this global variable with your other globals
sBall ball;

// Add these ball physics constants with your other constants
#define BALL_RADIUS 4.0f
#define BALL_INITIAL_SPEED 2.0f
#define BALL_MAX_SPEED 8.0f
#define BALL_BOUNCE_FACTOR 0.8f
#define BALL_FRICTION 0.98f
#define PLAYER_PUSH_FORCE 5.0f


void GameStartup() {
	InitAudioDevice();

	Image image = LoadImage("Assets/Spritesheets/colored_tilemap_packed.PNG");
	textures[TEXTURE_TILEMAP] = LoadTextureFromImage(image);
	UnloadImage(image);

	Image playerImage = LoadImage("Assets/Spritesheets/BlueKnightRunSprite-sheet16x17.png"); // Adjust path as needed
	textures[TEXTURE_PLAYER] = LoadTextureFromImage(playerImage);
	UnloadImage(playerImage);

	if (textures[TEXTURE_PLAYER].id == 0) {
		TraceLog(LOG_ERROR, "Failed to load player spritesheet!");
	}

	// Initialize camera first
	camera.target = (Vector2){ (float)WORLD_WIDTH * TILE_WIDTH / 2, (float)WORLD_HEIGHT * TILE_HEIGHT / 2 };
	camera.offset = (Vector2){ (float)screenWidth / 2, (float)screenHeight / 2 };
	camera.rotation = 0.0f;
	camera.zoom = 5.0f;

	// Initialize all tiles as empty
	for (int i = 0; i < WORLD_WIDTH; i++) {
		for (int j = 0; j < WORLD_HEIGHT; j++) {
			world[i][j] = (sTile){
				.x = i,
				.y = j,
				.type = TILE_EMPTY
			};
		}
	}

	// Create boundary walls to fit the screen exactly
// Calculate visible area in tiles
	int visibleWidthInTiles = screenWidth / (TILE_WIDTH * camera.zoom);
	int visibleHeightInTiles = screenHeight / (TILE_HEIGHT * camera.zoom);

	// Calculate tile positions for screen edges
	int leftEdge = (WORLD_WIDTH / 2) - (visibleWidthInTiles / 2);
	int rightEdge = (WORLD_WIDTH / 2) + (visibleWidthInTiles / 2);
	int topEdge = (WORLD_HEIGHT / 2) - (visibleHeightInTiles / 2);
	int bottomEdge = (WORLD_HEIGHT / 2) + (visibleHeightInTiles / 2);

	// Create top and bottom walls
	for (int i = leftEdge; i <= rightEdge; i++) {
		world[i][topEdge].type = TILE_WALL;
		world[i][bottomEdge].type = TILE_WALL;
	}

	// Create left and right walls
	for (int j = topEdge; j <= bottomEdge; j++) {
		world[leftEdge][j].type = TILE_WALL;
		world[rightEdge][j].type = TILE_WALL;
	}

	// Add some small walls in the middle
	int centerX = WORLD_WIDTH / 2;
	int centerY = WORLD_HEIGHT / 2;

	// Small horizontal wall
	for (int i = centerX - 5; i <= centerX + 5; i++) {
		world[i][centerY].type = TILE_WALL;
	}

	// Small vertical wall
	for (int j = centerY - 3; j <= centerY + 3; j++) {
		world[centerX + 10][j].type = TILE_WALL;
		world[centerX - 10][j].type = TILE_WALL;
	}

	player = (sEntity){
	.x = screenWidth / 2,
	.y = screenHeight / 2,
	.currentAnim = ANIM_IDLE_DOWN,
	.frameCounter = 0,
	.currentFrame = 0,
	.facing = 0
	};

	ball = (sBall){
	.x = (float)(WORLD_WIDTH * TILE_WIDTH / 2) + 20,  // Place ball near player
	.y = (float)(WORLD_HEIGHT * TILE_HEIGHT / 2) + 20,
	.speedX = 0,  // Changed from BALL_INITIAL_SPEED to 0
	.speedY = 0,  // Changed from BALL_INITIAL_SPEED to 0
	.radius = BALL_RADIUS,
	.active = true
	};
}
void GameUpdate() {
	PlayerMovement();
	UpdateBall();
}

void GameRender() {

	BeginMode2D(camera);

	//draw floor and walls
	sTile tile;
	int texture_index_x = 0;
	int texture_index_y = 0;

	// Only render tiles that are visible on screen
	// Calculate the visible area in world coordinates
	Vector2 topLeft = GetScreenToWorld2D((Vector2) { 0, 0 }, camera);
	Vector2 bottomRight = GetScreenToWorld2D((Vector2) { screenWidth, screenHeight }, camera);

	// Convert to tile indices and add a buffer
	int startTileX = (int)(topLeft.x / TILE_WIDTH) - 1;
	int startTileY = (int)(topLeft.y / TILE_HEIGHT) - 1;
	int endTileX = (int)(bottomRight.x / TILE_WIDTH) + 1;
	int endTileY = (int)(bottomRight.y / TILE_HEIGHT) + 1;

	// Clamp to world bounds
	startTileX = (startTileX < 0) ? 0 : startTileX;
	startTileY = (startTileY < 0) ? 0 : startTileY;
	endTileX = (endTileX >= WORLD_WIDTH) ? WORLD_WIDTH - 1 : endTileX;
	endTileY = (endTileY >= WORLD_HEIGHT) ? WORLD_HEIGHT - 1 : endTileY;

	// Only render visible tiles
	for (int i = startTileX; i <= endTileX; i++) {
		for (int j = startTileY; j <= endTileY; j++) {
			tile = world[i][j];

			if (tile.type == TILE_EMPTY) {
				// Floor tile
				texture_index_x = 4;
				texture_index_y = 4;
				DrawTile(texture_index_x, texture_index_y, tile.x * TILE_WIDTH, tile.y * TILE_HEIGHT, (Color) { 255, 255, 255, 50 });
			}
			else if (tile.type == TILE_WALL) {
				// Wall tile - use a clearly visible sprite
				texture_index_x = 15;  // Adjust to a distinct tile in your tilemap
				texture_index_y = 6;   // Adjust to a distinct tile in your tilemap
				DrawTile(texture_index_x, texture_index_y, tile.x * TILE_WIDTH, tile.y * TILE_HEIGHT, WHITE);
			}
		}
	}

	// Draw the player
	// Replace your player drawing code in GameRender() with this
// Get the source X and Y on the spritesheet based on animation state
	int sourceX = 0;
	int sourceY = 0;

	switch (player.currentAnim) {
	case ANIM_IDLE_DOWN:
		sourceX = 0;
		sourceY = 0;
		break;
	case ANIM_WALK_DOWN:
		sourceX = player.currentFrame * TILE_WIDTH;
		sourceY = 0;
		break;
	case ANIM_IDLE_UP:
		sourceX = 0;
		sourceY = TILE_HEIGHT;
		break;
	case ANIM_WALK_UP:
		sourceX = player.currentFrame * TILE_WIDTH;
		sourceY = TILE_HEIGHT;
		break;
	case ANIM_IDLE_LEFT:
		sourceX = 0;
		sourceY = 2 * TILE_HEIGHT;
		break;
	case ANIM_WALK_LEFT:
		sourceX = player.currentFrame * TILE_WIDTH;
		sourceY = 2 * TILE_HEIGHT;
		break;
	case ANIM_IDLE_RIGHT:
		sourceX = 0;
		sourceY = 3 * TILE_HEIGHT;
		break;
	case ANIM_WALK_RIGHT:
		sourceX = player.currentFrame * TILE_WIDTH;
		sourceY = 3 * TILE_HEIGHT;
		break;
	}

	// Draw the player with animation
	DrawPlayerSprite(sourceX / SPRITE_WIDTH, sourceY / SPRITE_HEIGHT, player.x - SPRITE_WIDTH / 2, player.y - SPRITE_HEIGHT / 2, WHITE);

	if (ball.active) {
		// Draw the ball as a circle
		DrawCircle(ball.x, ball.y, ball.radius, RED);

		// Optionally draw ball speed debug info
		// DrawText(TextFormat("Ball Speed: %.1f, %.1f", ball.speedX, ball.speedY), 10, 70, 20, BLACK);
	}

	EndMode2D();

	// Debug info
	DrawText(TextFormat("Player Position: %.1f, %.1f", player.x, player.y), 10, 10, 20, WHITE);
	DrawText(TextFormat("Player Tile: %d, %d", (int)(player.x / TILE_WIDTH), (int)(player.y / TILE_HEIGHT)), 10, 40, 20, WHITE);

}
void GameShutdown() {

	for (int i = 0; i < MAX_TEXTURES; i++) {
		UnloadTexture(textures[i]);
	}

	CloseAudioDevice();
}

void DrawTile(int source_x, int source_y, int dest_x, int dest_y, Color c)
{
	Rectangle source = { (float)(source_x * TILE_WIDTH), (float)(source_y * TILE_WIDTH), (float)TILE_WIDTH, (float)TILE_HEIGHT };
	Rectangle dest = { (float)(dest_x), (float)(dest_y), (float)TILE_WIDTH, (float)TILE_HEIGHT };
	Vector2 origin = { 0,0 };

	DrawTexturePro(textures[TEXTURE_TILEMAP], source, dest, origin, 0.0f, c);
}

void PlayerMovement()
{
	Vector2 newPosition = (Vector2){ player.x, player.y };
	Vector2 prevPosition = newPosition;

	float deltaTime = GetFrameTime();

	// Add reset functionality
	if (IsKeyPressed(KEY_R)) {
		player.x = (float)(WORLD_WIDTH * TILE_WIDTH / 2);
		player.y = (float)(WORLD_HEIGHT * TILE_HEIGHT / 2);
		speed.x = 0;
		speed.y = 0;

		// Also reset ball
		ball.x = player.x + 20;
		ball.y = player.y + 20;
		ball.speedX = 0;  // Changed from BALL_INITIAL_SPEED to 0
		ball.speedY = 0;  // Changed from BALL_INITIAL_SPEED to 0

		return;
	}

	// Input WASD
	if (IsKeyDown(KEY_W)) speed.y -= acceleration * deltaTime;
	if (IsKeyDown(KEY_S)) speed.y += acceleration * deltaTime;
	if (IsKeyDown(KEY_A)) speed.x -= acceleration * deltaTime;
	if (IsKeyDown(KEY_D)) speed.x += acceleration * deltaTime;

	// Deceleration
	if (!IsKeyDown(KEY_W) && !IsKeyDown(KEY_S))
	{
		if (speed.y > 0)
		{
			speed.y -= decel * deltaTime;
			if (speed.y < 0) speed.y = 0;
		}
		else if (speed.y < 0)
		{
			speed.y += decel * deltaTime;
			if (speed.y > 0) speed.y = 0;
		}
	}

	if (!IsKeyDown(KEY_A) && !IsKeyDown(KEY_D))
	{
		if (speed.x > 0)
		{
			speed.x -= decel * deltaTime;
			if (speed.x < 0) speed.x = 0;
		}
		else if (speed.x < 0)
		{
			speed.x += decel * deltaTime;
			if (speed.x > 0) speed.x = 0;
		}
	}

	// Speed limit
	if (speed.x > maxSpeed) speed.x = maxSpeed;
	if (speed.x < -maxSpeed) speed.x = -maxSpeed;
	if (speed.y > maxSpeed) speed.y = maxSpeed;
	if (speed.y < -maxSpeed) speed.y = -maxSpeed;

	// Calculate new position
	newPosition.x += speed.x;
	newPosition.y += speed.y;

	// Separate X and Y movement for better collision handling

	// Try X movement first
	float testX = newPosition.x;
	float testY = player.y;

	// Test if X movement would cause collision
	if (!IsWallAtPosition(testX, testY)) {
		player.x = testX;
	}
	else {
		// X movement caused collision, stop X movement
		speed.x = 0;
	}

	// Calculate visible area in tiles
	int visibleWidthInTiles = screenWidth / (TILE_WIDTH * camera.zoom);
	int visibleHeightInTiles = screenHeight / (TILE_HEIGHT * camera.zoom);
	// // Calculate tile positions for screen edges
	int leftEdge = (WORLD_WIDTH / 2) - (visibleWidthInTiles / 2);
	int rightEdge = (WORLD_WIDTH / 2) + (visibleWidthInTiles / 2);
	int topEdge = (WORLD_HEIGHT / 2) - (visibleHeightInTiles / 2);
	int bottomEdge = (WORLD_HEIGHT / 2) + (visibleHeightInTiles / 2);
	// Add bounds checking to keep player in visible area
	float leftBound = (leftEdge + 1) * TILE_WIDTH;
	float rightBound = (rightEdge - 1) * TILE_WIDTH;
	float topBound = (topEdge + 1) * TILE_HEIGHT;
	float bottomBound = (bottomEdge - 1) * TILE_HEIGHT;

	if (player.x < leftBound) player.x = leftBound;
	if (player.y < topBound) player.y = topBound;
	if (player.x > rightBound) player.x = rightBound;
	if (player.y > bottomBound) player.y = bottomBound;

	// Then try Y movement
	testX = player.x;
	testY = newPosition.y;

	// Test if Y movement would cause collision
	if (!IsWallAtPosition(testX, testY)) {
		player.y = testY;
	}
	else {
		// Y movement caused collision, stop Y movement
		speed.y = 0;
	}

	// Update camera to follow player
	//camera.target = (Vector2){ player.x, player.y };

	// Inside PlayerMovement(), after handling movement
// Update animation based on movement
	bool isMoving = (speed.x != 0 || speed.y != 0);

	// Determine facing direction based on movement
	if (fabs(speed.x) > fabs(speed.y)) {
		// Horizontal movement dominates
		if (speed.x > 0) player.facing = 3; // Right
		else if (speed.x < 0) player.facing = 2; // Left
	}
	else if (speed.y != 0) {
		// Vertical movement dominates
		if (speed.y > 0) player.facing = 0; // Down
		else if (speed.y < 0) player.facing = 1; // Up
	}

	// Set animation based on facing and movement
	if (isMoving) {
		switch (player.facing) {
		case 0: player.currentAnim = ANIM_WALK_DOWN; break;
		case 1: player.currentAnim = ANIM_WALK_UP; break;
		case 2: player.currentAnim = ANIM_WALK_LEFT; break;
		case 3: player.currentAnim = ANIM_WALK_RIGHT; break;
		}
	}
	else {
		switch (player.facing) {
		case 0: player.currentAnim = ANIM_IDLE_DOWN; break;
		case 1: player.currentAnim = ANIM_IDLE_UP; break;
		case 2: player.currentAnim = ANIM_IDLE_LEFT; break;
		case 3: player.currentAnim = ANIM_IDLE_RIGHT; break;
		}
	}

	// Animation timing
	player.frameCounter++;
	if (player.frameCounter >= 6) { // Adjust this value to control animation speed
		player.frameCounter = 0;
		player.currentFrame++;

		// Reset animation frame based on animation type
		// Assuming each animation has 3 frames
		if (player.currentFrame > 2) {
			player.currentFrame = 0;
		}
	}
}

bool IsWallAtPosition(float x, float y) {
	// Convert world coordinates to tile coordinates
	int tileX = (int)(x / TILE_WIDTH);
	int tileY = (int)(y / TILE_HEIGHT);

	// Debug output to help troubleshooting
	//TraceLog(LOG_INFO, "Checking position (%.1f, %.1f) -> Tile (%d, %d)", x, y, tileX, tileY);

	// Check boundaries
	if (tileX < 0 || tileX >= WORLD_WIDTH || tileY < 0 || tileY >= WORLD_HEIGHT) {
		//TraceLog(LOG_INFO, "Out of bounds - wall detected");
		return true; // Treat out of bounds as walls
	}

	// Check if this tile is a wall
	return world[tileX][tileY].type == TILE_WALL;
}

void UpdateBall() {
	if (!ball.active) return;

	// Store previous position for collision response
	float prevX = ball.x;
	float prevY = ball.y;

	// Apply ball physics - update position based on speed
	ball.x += ball.speedX;
	ball.y += ball.speedY;

	// Apply friction to gradually slow down the ball
	ball.speedX *= BALL_FRICTION;
	ball.speedY *= BALL_FRICTION;

	// Check for wall collisions and bounce
	// Convert ball position to tile coordinates
	int tileX = (int)(ball.x / TILE_WIDTH);
	int tileY = (int)(ball.y / TILE_HEIGHT);

	// Check tiles around the ball
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			int checkX = tileX + i;
			int checkY = tileY + j;

			// Make sure we're within world bounds
			if (checkX >= 0 && checkX < WORLD_WIDTH && checkY >= 0 && checkY < WORLD_HEIGHT) {
				// If there's a wall at this position
				if (world[checkX][checkY].type == TILE_WALL) {
					// Calculate the distance from ball center to this tile
					float tileLeft = checkX * TILE_WIDTH;
					float tileRight = tileLeft + TILE_WIDTH;
					float tileTop = checkY * TILE_HEIGHT;
					float tileBottom = tileTop + TILE_HEIGHT;

					// Simple AABB collision resolution with bounce
					// Horizontal collision
					if (ball.y > tileTop && ball.y < tileBottom) {
						if (prevX + ball.radius < tileLeft && ball.x + ball.radius >= tileLeft) {
							// Ball hit left side of tile
							ball.x = tileLeft - ball.radius;
							ball.speedX = -ball.speedX * BALL_BOUNCE_FACTOR;
						}
						else if (prevX - ball.radius > tileRight && ball.x - ball.radius <= tileRight) {
							// Ball hit right side of tile
							ball.x = tileRight + ball.radius;
							ball.speedX = -ball.speedX * BALL_BOUNCE_FACTOR;
						}
					}

					// Vertical collision
					if (ball.x > tileLeft && ball.x < tileRight) {
						if (prevY + ball.radius < tileTop && ball.y + ball.radius >= tileTop) {
							// Ball hit top of tile
							ball.y = tileTop - ball.radius;
							ball.speedY = -ball.speedY * BALL_BOUNCE_FACTOR;
						}
						else if (prevY - ball.radius > tileBottom && ball.y - ball.radius <= tileBottom) {
							// Ball hit bottom of tile
							ball.y = tileBottom + ball.radius;
							ball.speedY = -ball.speedY * BALL_BOUNCE_FACTOR;
						}
					}
				}
			}
		}
	}

	// Check for collision with player
	float dx = ball.x - player.x;
	float dy = ball.y - player.y;
	float distance = sqrt(dx * dx + dy * dy);

	// Player radius (estimated)
	float playerRadius = (TILE_WIDTH + TILE_HEIGHT) / 4;

	// If ball and player are colliding
	if (distance < ball.radius + playerRadius) {
		// Normalize the collision vector
		if (distance > 0) {
			dx /= distance;
			dy /= distance;
		}
		else {
			dx = 0;
			dy = -1; // Push upward if direct overlap
		}

		// Push the ball away from player
		ball.x = player.x + dx * (ball.radius + playerRadius);
		ball.y = player.y + dy * (ball.radius + playerRadius);

		// Calculate ball's new speed based on player's movement
		ball.speedX += dx * PLAYER_PUSH_FORCE + speed.x * 0.5f;
		ball.speedY += dy * PLAYER_PUSH_FORCE + speed.y * 0.5f;

		// Cap ball speed
		float ballSpeed = sqrt(ball.speedX * ball.speedX + ball.speedY * ball.speedY);
		if (ballSpeed > BALL_MAX_SPEED) {
			ball.speedX = (ball.speedX / ballSpeed) * BALL_MAX_SPEED;
			ball.speedY = (ball.speedY / ballSpeed) * BALL_MAX_SPEED;
		}
	}

	// Keep ball in bounds
	if (ball.x < ball.radius) {
		ball.x = ball.radius;
		ball.speedX = -ball.speedX * BALL_BOUNCE_FACTOR;
	}
	if (ball.x > WORLD_WIDTH * TILE_WIDTH - ball.radius) {
		ball.x = WORLD_WIDTH * TILE_WIDTH - ball.radius;
		ball.speedX = -ball.speedX * BALL_BOUNCE_FACTOR;
	}
	if (ball.y < ball.radius) {
		ball.y = ball.radius;
		ball.speedY = -ball.speedY * BALL_BOUNCE_FACTOR;
	}
	if (ball.y > WORLD_HEIGHT * TILE_HEIGHT - ball.radius) {
		ball.y = WORLD_HEIGHT * TILE_HEIGHT - ball.radius;
		ball.speedY = -ball.speedY * BALL_BOUNCE_FACTOR;
	}

	// If ball is very slow, stop it completely to prevent tiny endless movement
	if (fabs(ball.speedX) < 0.1f) ball.speedX = 0;
	if (fabs(ball.speedY) < 0.1f) ball.speedY = 0;
}

void DrawPlayerSprite(int source_x, int source_y, int dest_x, int dest_y, Color c)
{
	// Debug - print the sprite coordinates
	TraceLog(LOG_INFO, "Drawing player sprite at source (%d, %d), dest (%d, %d)",
		source_x, source_y, dest_x, dest_y);

	Rectangle source = { (float)(source_x * SPRITE_WIDTH), (float)(source_y * SPRITE_HEIGHT),
						 (float)SPRITE_WIDTH, (float)SPRITE_HEIGHT };
	Rectangle dest = { (float)(dest_x), (float)(dest_y),
					   (float)SPRITE_WIDTH, (float)SPRITE_HEIGHT };
	Vector2 origin = { 0, 0 };

	DrawTexturePro(textures[TEXTURE_PLAYER], source, dest, origin, 0.0f, c);
}

int main()
{
	InitWindow(screenWidth, screenHeight, "Raylib Messy Game");
	SetTargetFPS(60);

	GameStartup();

	while (!WindowShouldClose()) {
		GameUpdate();
		BeginDrawing();
		ClearBackground(LIME);

		GameRender();

		EndDrawing();

	}

	GameShutdown();

	CloseWindow();
	return 0;
}
