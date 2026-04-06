#include <math.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_ROWS 1024
#define MAX_COLS 1024

struct Vec3 {
  float x;
  float y;
  float z;
};

float A, B, C;
const char ch = '#';
const float far = 1e9f;
const float face = 35.0f;
const float mid = face / 2.0f;

float buffer[MAX_ROWS][MAX_COLS];
struct Vec3 camera = {0, 0, 1};
struct Vec3 vertices[8];
struct Vec3 normales[6];
struct Vec3 base_vertices[8] = {{-mid, -mid, mid}, {-mid, -mid, -mid}, {mid, -mid, -mid}, {mid, -mid, mid},
                                {-mid, mid, mid},  {-mid, mid, -mid},  {mid, mid, -mid},  {mid, mid, mid}};

void rotate_x(struct Vec3 *vertex) {
  float verticesY = vertex->y, verticesZ = vertex->z;
  vertex->y = verticesY * cos(A) - verticesZ * sin(A);
  vertex->z = verticesY * sin(A) + verticesZ * cos(A);
}

void rotate_y(struct Vec3 *vertex) {
  float verticesX = vertex->x, verticesZ = vertex->z;
  vertex->x = verticesX * cos(B) + verticesZ * sin(B);
  vertex->z = verticesZ * cos(B) - verticesX * sin(B);
}

void rotate_z(struct Vec3 *vertex) {
  float verticesX = vertex->x, verticesY = vertex->y;
  vertex->x = verticesX * cos(C) - verticesY * sin(C);
  vertex->y = verticesX * sin(C) + verticesY * cos(C);
}

int dot_vec3(struct Vec3 a, struct Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
struct Vec3 subtract_vec3(struct Vec3 a, struct Vec3 b) { return (struct Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
struct Vec3 cross_vec3(struct Vec3 a, struct Vec3 b) { return (struct Vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
struct Vec3 get_normale(struct Vec3 a, struct Vec3 b, struct Vec3 c) { return cross_vec3(subtract_vec3(b, a), subtract_vec3(c, a)); }

void configure_normales() {
  normales[0] = get_normale(vertices[2], vertices[1], vertices[0]);
  normales[1] = get_normale(vertices[4], vertices[5], vertices[6]);
  normales[2] = get_normale(vertices[4], vertices[0], vertices[1]);
  normales[3] = get_normale(vertices[5], vertices[1], vertices[2]);
  normales[4] = get_normale(vertices[2], vertices[3], vertices[7]);
  normales[5] = get_normale(vertices[3], vertices[0], vertices[4]);
}

void update_depth_buffer(int x, int y, float z) {
  float curr = buffer[y][x];
  buffer[y][x] = curr > z ? z : curr;
}

void draw_line(struct Vec3 a, struct Vec3 b) {
  int cols, rows;
  getmaxyx(stdscr, rows, cols);
  int az = a.z;
  int ax = cols / 2 + (int)a.x;
  int ay = rows / 2 + (int)a.y / 2;
  int bx = cols / 2 + (int)b.x;
  int by = rows / 2 + (int)b.y / 2;
  int deltaX = abs(bx - ax);
  int deltaY = abs(by - ay);
  int signX = ax < bx ? 1 : -1;
  int signY = ay < by ? 1 : -1;
  int error = deltaX - deltaY;
  int steps = deltaX > deltaY ? deltaX : deltaY;
  float step = (b.z - a.z) / steps;

  while (1) {
    update_depth_buffer(ax, ay, az);
    az += step;
    if (ax == bx && ay == by) break;
    int vertices = error * 2;
    if (vertices > -deltaY) {
      error -= deltaY;
      ax += signX;
    }
    if (vertices < deltaX) {
      error += deltaX;
      ay += signY;
    }
  }
}

void draw_edges() {
  int top = dot_vec3(camera, normales[0]);
  int bottom = dot_vec3(camera, normales[1]);
  int left = dot_vec3(camera, normales[2]);
  int behind = dot_vec3(camera, normales[3]);
  int right = dot_vec3(camera, normales[4]);
  int front = dot_vec3(camera, normales[5]);

  if (top < 0) {
    draw_line(vertices[0], vertices[1]);
    draw_line(vertices[1], vertices[2]);
    draw_line(vertices[2], vertices[3]);
    draw_line(vertices[3], vertices[0]);
  }
  if (bottom < 0) {
    draw_line(vertices[4], vertices[5]);
    draw_line(vertices[5], vertices[6]);
    draw_line(vertices[6], vertices[7]);
    draw_line(vertices[7], vertices[4]);
  }
  if (left < 0) {
    draw_line(vertices[4], vertices[0]);
    draw_line(vertices[0], vertices[1]);
    draw_line(vertices[1], vertices[5]);
    draw_line(vertices[5], vertices[4]);
  }
  if (behind < 0) {
    draw_line(vertices[5], vertices[1]);
    draw_line(vertices[1], vertices[2]);
    draw_line(vertices[2], vertices[6]);
    draw_line(vertices[6], vertices[5]);
  }
  if (right < 0) {
    draw_line(vertices[7], vertices[3]);
    draw_line(vertices[3], vertices[2]);
    draw_line(vertices[2], vertices[6]);
    draw_line(vertices[6], vertices[7]);
  }
  if (front < 0) {
    draw_line(vertices[0], vertices[3]);
    draw_line(vertices[3], vertices[7]);
    draw_line(vertices[7], vertices[4]);
    draw_line(vertices[4], vertices[0]);
  }
}

void draw_faces() {}

int main() {
  initscr();
  noecho();
  cbreak();

  while (1) {
    memcpy(vertices, base_vertices, sizeof(vertices));
    for (int i = 0; i < 8; i++) {
      rotate_x(&vertices[i]);
      rotate_y(&vertices[i]);
      rotate_z(&vertices[i]);
    }

    for (int i = 0; i < MAX_ROWS; i++) {
      for (int j = 0; j < MAX_COLS; j++) {
        buffer[i][j] = far;
      }
    }

    configure_normales();
    draw_edges();
    int cols, rows;
    getmaxyx(stdscr, rows, cols);
    clear();
    mvprintw(0, 0, "rotation (radians)\n");
    mvprintw(1, 0, "x: %.1f, y: %.1f, z: %.1f\n", A, B, C);
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        float depth = buffer[i][j];
        if (depth != far) mvaddch(i, j, '#');
      }
    }
    refresh();

    A += 0.1f;
    B += 0.1f;
    usleep(100000);
  }
  endwin();
}
