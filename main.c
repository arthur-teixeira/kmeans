#include <assert.h>
#include <float.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define da_append(da, item)                                                    \
  do {                                                                         \
    if ((da)->count >= (da)->capacity) {                                       \
      (da)->capacity = (da)->capacity == 0 ? 32 : (da)->capacity * 2;          \
      (da)->items =                                                            \
          realloc((da)->items, (da)->capacity * sizeof(*(da)->items));         \
      assert((da)->items != NULL && "Buy more RAM lol");                       \
    }                                                                          \
                                                                               \
    (da)->items[(da)->count++] = (item);                                       \
  } while (0)

#define K 8

typedef struct {
  Color key;
} ColorPoint;

typedef struct {
  Vector3 *items;
  size_t count;
  size_t capacity;
} ColorSamples;

float rand_float() { return (float)rand() / RAND_MAX; }

ColorSamples samples = {0};
ColorSamples clusters[K] = {0};
Vector3 centroids[K] = {0};

void generate_centroids(double cluster_radius) {
  for (int i = 0; i < K; i++) {
    if (clusters[i].count > 0) {
      centroids[i] = Vector3Zero();
      for (size_t j = 0; j < clusters[i].count; ++j) {
        centroids[i] = Vector3Add(centroids[i], clusters[i].items[j]);
      }

      centroids[i] =
          Vector3Scale(centroids[i], (1.0f / (float)clusters[i].count));
    } else {
      Vector3 new_centroid = {
          .x = Lerp(-cluster_radius, cluster_radius, rand_float()),
          .y = Lerp(-cluster_radius, cluster_radius, rand_float()),
          .z = Lerp(-cluster_radius, cluster_radius, rand_float()),
      };
      centroids[i] = new_centroid;
    }
  }
}

void recluster_state() {
  for (int i = 0; i < K; i++) {
    clusters[i].count = 0;
  }
  for (int i = 0; i < samples.count; i++) {
    Vector3 sample = samples.items[i];
    int k = -1;
    float min_distance = FLT_MAX;

    for (size_t j = 0; j < K; j++) {
      Vector3 centroid = centroids[j];
      float dist = Vector3LengthSqr(Vector3Subtract(sample, centroid));
      if (dist < min_distance) {
        min_distance = dist;
        k = j;
      }
    }

    da_append(&clusters[k], sample);
  }
}

int cluster_of_color(Color c, int cluster_radius) {
  int k = -1;
  float min_distance = FLT_MAX;

  Vector3 sample = {
      .x = c.r / 255.0f * cluster_radius,
      .y = c.g / 255.0f * cluster_radius,
      .z = c.b / 255.0f * cluster_radius,
  };

  for (size_t j = 0; j < K; j++) {
    Vector3 centroid = centroids[j];
    float dist = Vector3LengthSqr(Vector3Subtract(sample, centroid));
    if (dist < min_distance) {
      min_distance = dist;
      k = j;
    }
  }

  return k;
}

int main() {
  srand(time(NULL));

  double cluster_radius = 20;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "K-Means clustering");

  Image img = LoadImage("./images/Lena_512.png");
  ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  Color *pixels = img.data;
  size_t pixels_count = img.width * img.height;

  ColorPoint *unique_colors = NULL;
  for (int i = 0; i < pixels_count; i++) {
    Color c = pixels[i];
    if (hmgeti(unique_colors, c) < 0) {
      hmputs(unique_colors, (ColorPoint){c});
    }
  }

  for (int i = 0; i < hmlen(unique_colors); i++) {
    Color c = unique_colors[i].key;
    Vector3 sample = {
        .x = c.r / 255.0f * cluster_radius,
        .y = c.g / 255.0f * cluster_radius,
        .z = c.b / 255.0f * cluster_radius,
    };

    da_append(&samples, sample);
  }

  Camera3D camera = {0};
  camera.position = (Vector3){0.0f, 10.0f, 10.0f}; // Camera position
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};     // Camera looking at point
  camera.up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;             // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE; // Camera mode type

  DisableCursor();
  generate_centroids(cluster_radius);
  recluster_state();

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

    if (IsKeyPressed(KEY_C)) {
      generate_centroids(cluster_radius);
      recluster_state();
    }
    if (IsKeyPressed(KEY_ONE)) {
      for (size_t i = 0; i < pixels_count; i++) {
        int j = cluster_of_color(pixels[i], cluster_radius);
        if (j < 0) {
          fprintf(stderr, "ERROR: Color outside of cluster\n");
          exit(EXIT_FAILURE);
        }

        pixels[i] = (Color){
            .r = centroids[j].x * 255.0f / cluster_radius,
            .g = centroids[j].y * 255.0f / cluster_radius,
            .b = centroids[j].z * 255.0f / cluster_radius,
            .a = 255,
        };
      }

      ExportImage(img, "Output.png");
    }

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode3D(camera);

    for (size_t i = 0; i < K; i++) {
      Vector3 centroid = centroids[i];
      Color mean_color = {
          .r = centroid.x * 255.0f / cluster_radius,
          .g = centroid.y * 255.0f / cluster_radius,
          .b = centroid.z * 255.0f / cluster_radius,
          .a = 255.0f,
      };
      DrawSphere(centroid, 0.5, RAYWHITE);

      ColorSamples cluster = clusters[i];
      for (size_t j = 0; j < cluster.count; j++) {
        Vector3 c = cluster.items[j];
        DrawCube(c, 1, 1, 1, mean_color);
      }
    }
    EndMode3D();
    EndDrawing();
  }
}
