#include <raylib.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct {
  Color key;
} ColorPoint;

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "K-Means clustering");

  Image img = LoadImage("./images/Lena_512.png");
  ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  Color *pixels = img.data;

  ColorPoint *unique_colors = NULL;
  for (int i = 0; i < img.width * img.height; i++) {
    Color c = pixels[i];
    if (hmgeti(unique_colors, c) < 0) {
      hmputs(unique_colors, (ColorPoint){c});
    }
  }

  Camera3D camera = {0};
  camera.position = (Vector3){0.0f, 10.0f, 10.0f}; // Camera position
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};     // Camera looking at point
  camera.up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;             // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE; // Camera mode type

  DisableCursor();

  while (!WindowShouldClose()) {
    UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    float dt = 10 * GetFrameTime();

    if (IsKeyDown(KEY_SPACE)) {
        camera.position.y += dt;
        camera.target.y += dt;
    } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
        camera.position.y -= dt;
        camera.target.y -= dt;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode3D(camera);

    for (int i = 0; i < hmlen(unique_colors); i++) {
      Color c = unique_colors[i].key;
      DrawCube((Vector3){c.r, c.g, c.b}, 1, 1, 1, c);
    }

    EndMode3D();
    EndDrawing();
  }
}
