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

	camera.target = (Vector2){ (float)screenWidth/2, (float)screenHeight/2 };
	camera.offset = (Vector2){ (float)screenWidth/2, (float)screenHeight/2 };
	camera.rotation = 0.0f;
	camera.zoom = 5.0f;
}
void GameUpdate() {}
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

			Rectangle source = { (float)(texture_index_x * TILE_WIDTH), (float)(texture_index_y * TILE_WIDTH), (float)TILE_WIDTH, (float)TILE_HEIGHT };
			Rectangle dest = { (float)(tile.x * TILE_WIDTH), (float)(tile.y * TILE_HEIGHT), (float)TILE_WIDTH, (float)TILE_HEIGHT };
			Vector2 origin = { 0,0 };
			DrawTexturePro(textures[TEXTURE_TILEMAP], source, dest, origin, 0.0f, (Color){ 255, 255, 255, 50 });
		}
	}
}
void GameShutdown() {

	for (int i = 0; i < MAX_TEXTURES; i++) {
		UnloadTexture(textures[i]);
	}

	CloseAudioDevice();
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
