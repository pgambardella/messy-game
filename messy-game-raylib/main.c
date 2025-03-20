// main.c : Messy Game
// a Zelda clone where you are a mage that controls an enchanted ball like a PRO
#include "raylib.h"
#include "raymath.h"

#define TILE_WIDTH 8
#define TILE_HEIGHT 8

void GameStartup();
void GameUpdate();
void GameRender();
void GameShutdown();

void DrawTile(int source_x, int source_y, int dest_x, int dest_y, Color c);

const int screenWidth = 600;
const int screenHeight = 960;

#define MAX_TEXTURES 1
typedef enum {
	TEXTURE_TILEMAP = 0
} texture_asset;

Texture2D textures[MAX_TEXTURES];

#define WORLD_WIDTH 76 //* TILE_WIDTH
#define WORLD_HEIGHT 120//* TILE_HEIGHT

typedef struct {
	int x;
	int y;
} sTile;

sTile world[WORLD_WIDTH][WORLD_HEIGHT];

Camera2D camera = { 0 };

typedef struct {
	float x;
	float y;
} sEntity;

sEntity player;

float acceleration = 700.0f;
float maxSpeed = 150.0f;
float decel = 250.0f;
Vector2 speed = { 0.0f, 0.0f };
float lerpAlpha = 1.2f;


void GameStartup() {
	InitAudioDevice();

	Image image = LoadImage("Assets/Spritesheets/colored_tilemap_packed.PNG");
	textures[TEXTURE_TILEMAP] = LoadTextureFromImage(image);
	UnloadImage(image);

	for (int i = 0; i < WORLD_WIDTH; i++) {
		for (int j = 0; j < WORLD_HEIGHT; j++) {
			world[i][j] = (sTile){
				.x = i,
				.y = j,
			};	
		}
	}

	player = (sEntity){
		.x = screenWidth / 2,
		.y = screenHeight / 2,
	};

	camera.target = (Vector2){ (float)screenWidth/2, (float)screenHeight/2 };
	camera.offset = (Vector2){ (float)screenWidth/2, (float)screenHeight/2 };
	camera.rotation = 0.0f;
	camera.zoom = 5.0f;
}
void GameUpdate() {
	Vector2 newPosition = (Vector2){ player.x, player.y };
	Vector2 prevPosition = newPosition;

	float deltaTime = GetFrameTime();
	
	// Input WASD
	if (IsKeyDown(KEY_W)) speed.y -= acceleration * deltaTime;
	if (IsKeyDown(KEY_S)) speed.y += acceleration * deltaTime;
	if (IsKeyDown(KEY_A)) speed.x -= acceleration * deltaTime;
	if (IsKeyDown(KEY_D)) speed.x += acceleration * deltaTime;

	// Decelerazione
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

	// Limita la velocità
	if (speed.x > maxSpeed) speed.x = maxSpeed;
	if (speed.x < -maxSpeed) speed.x = -maxSpeed;
	if (speed.y > maxSpeed) speed.y = maxSpeed;
	if (speed.y < -maxSpeed) speed.y = -maxSpeed;//*/

	// Aggiorna la posizione
	newPosition.x += speed.x * deltaTime;
	newPosition.y += speed.y * deltaTime;

	newPosition = Vector2Lerp(prevPosition, newPosition, lerpAlpha);

	player.x = newPosition.x;
	player.y = newPosition.y;

}
void GameRender() {

	BeginMode2D(camera);

	sTile tile;
	int texture_index_x = 0;
	int texture_index_y = 0;

	for (int i = 0; i < WORLD_WIDTH; i++) {
		for (int j = 0; j < WORLD_HEIGHT; j++) {
			tile = world[i][j];
			texture_index_x = 4;
			texture_index_y = 4;

			DrawTile(texture_index_x, texture_index_y, tile.x * TILE_WIDTH, tile.y * TILE_HEIGHT, (Color){ 255, 255, 255, 50 });
		}
	}

	DrawTile(4,0,player.x, player.y, WHITE);
	//DrawRectangle(player.x, player.y, 8, 8, BLACK);

	EndMode2D();

	//info UI
	DrawRectangle(0, 0, screenWidth, 64, Fade(SKYBLUE, 0.5f));
	DrawRectangleLines(0, 0, screenWidth, 64, YELLOW);
	DrawText("Messy Game", 5, 5, 20, YELLOW);
	DrawText("CONTROLS: WASD", 200, 5, 20, YELLOW);
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
	Rectangle dest = { (float)(dest_x ), (float)(dest_y), (float)TILE_WIDTH, (float)TILE_HEIGHT };
	Vector2 origin = { 0,0 };
	DrawTexturePro(textures[TEXTURE_TILEMAP], source, dest, origin, 0.0f, c);
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
