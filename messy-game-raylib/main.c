// main.c : Messy Game
// a Zelda clone where you are a mage that controls an enchanted ball like a PRO
#include "raylib.h"
#include "raymath.h"

void GameStartup() {}
void GameUpdate() {}
void GameRender() {}
void GameShutdown() {}

const int screenWidth = 600;
const int screenHeight = 960;

int main()
{
    InitWindow(screenWidth, screenHeight, "Raylib Messy Game");
    SetTargetFPS(60);

    GameStartup();

	while (!WindowShouldClose()) {
		GameUpdate();
		BeginDrawing();
		ClearBackground(GRAY);

		GameRender();

		EndDrawing();

	}

	GameShutdown();

	CloseWindow();
	return 0;
}
