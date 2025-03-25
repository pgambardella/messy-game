#include "camera.h"
#include <stdlib.h>

GameCamera* CameraCreate(int screenWidth, int screenHeight, float initialZoom) {
    GameCamera* camera = (GameCamera*)malloc(sizeof(GameCamera));
    if (camera) {
        camera->camera.zoom = initialZoom;
        camera->camera.rotation = 0.0f;
        camera->camera.target = (Vector2){ 0, 0 };
        camera->camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    }
    return camera;
}

void CameraDestroy(GameCamera* camera) {
    if (camera) free(camera);
}

void CameraUpdate(GameCamera* camera, float deltaTime) {
    // Stub implementation
}

void CameraBeginMode(GameCamera* camera) {
    BeginMode2D(camera->camera);
}

void CameraEndMode(void) {
    EndMode2D();
}

void CameraFollowTarget(GameCamera* camera, Entity* target) {
    if (!camera || !target) return;
    camera->target = target;
    camera->mode = CAMERA_MODE_FOLLOW;
}