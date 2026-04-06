#include <math.h>
#include <ncurses.h>
#include <stdbool.h>
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

int cols, rows;
float A, B, C;
const char ch = '#';
const float far = -1e9f;
const float edge = 35.0f;
const float mid = edge / 2.0f;

float depth_buffer[MAX_ROWS][MAX_COLS];
struct Vec3 camera = {0, 0, -1};
struct Vec3 vertices[8];
struct Vec3 normales[6];
struct Vec3 base_vertices[8] = {{-mid, -mid, mid}, {-mid, -mid, -mid}, {mid, -mid, -mid}, {mid, -mid, mid},
                                {-mid, mid, mid},  {-mid, mid, -mid},  {mid, mid, -mid},  {mid, mid, mid}};

void rotate_x(struct Vec3 *vertex) {
  float tempY = vertex->y, tempZ = vertex->z;
  vertex->y = tempY * cos(A) - tempZ * sin(A);
  vertex->z = tempY * sin(A) + tempZ * cos(A);
}

void rotate_y(struct Vec3 *vertex) {
  float tempX = vertex->x, tempZ = vertex->z;
  vertex->x = tempX * cos(B) + tempZ * sin(B);
  vertex->z = tempZ * cos(B) - tempX * sin(B);
}

void rotate_z(struct Vec3 *vertex) {
  float tempX = vertex->x, tempY = vertex->y;
  vertex->x = tempX * cos(C) - tempY * sin(C);
  vertex->y = tempX * sin(C) + tempY * cos(C);
}

int dot_vec3(struct Vec3 a, struct Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
struct Vec3 subtract_vec3(struct Vec3 a, struct Vec3 b) { return (struct Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
struct Vec3 cross_vec3(struct Vec3 a, struct Vec3 b) { return (struct Vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
struct Vec3 get_normal(struct Vec3 a, struct Vec3 b, struct Vec3 c) { return cross_vec3(subtract_vec3(b, a), subtract_vec3(c, a)); }

void configure_normales() {
  normales[0] = get_normal(vertices[2], vertices[1], vertices[0]);
  normales[1] = get_normal(vertices[4], vertices[5], vertices[6]);
  normales[2] = get_normal(vertices[4], vertices[0], vertices[1]);
  normales[3] = get_normal(vertices[5], vertices[1], vertices[2]);
  normales[4] = get_normal(vertices[2], vertices[3], vertices[7]);
  normales[5] = get_normal(vertices[3], vertices[0], vertices[4]);
}

void update_depth_buffer(int x, int y, float z) {
  float curr = depth_buffer[y][x];
  depth_buffer[y][x] = curr > z ? curr : z;
}

void draw_line(struct Vec3 a, struct Vec3 b) {
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
    int temp = error * 2;
    if (temp > -deltaY) {
      error -= deltaY;
      ax += signX;
    }
    if (temp < deltaX) {
      error += deltaX;
      ay += signY;
    }
  }
}

void draw_edges() {
  if (dot_vec3(camera, normales[0]) > 0) {
    draw_line(vertices[0], vertices[1]);
    draw_line(vertices[1], vertices[2]);
    draw_line(vertices[2], vertices[3]);
    draw_line(vertices[3], vertices[0]);
  } else {
    draw_line(vertices[4], vertices[5]);
    draw_line(vertices[5], vertices[6]);
    draw_line(vertices[6], vertices[7]);
    draw_line(vertices[7], vertices[4]);
  }

  if (dot_vec3(camera, normales[2]) > 0) {
    draw_line(vertices[4], vertices[0]);
    draw_line(vertices[0], vertices[1]);
    draw_line(vertices[1], vertices[5]);
    draw_line(vertices[5], vertices[4]);
  } else {
    draw_line(vertices[7], vertices[3]);
    draw_line(vertices[3], vertices[2]);
    draw_line(vertices[2], vertices[6]);
    draw_line(vertices[6], vertices[7]);
  }

  if (dot_vec3(camera, normales[3]) > 0) {
    draw_line(vertices[5], vertices[1]);
    draw_line(vertices[1], vertices[2]);
    draw_line(vertices[2], vertices[6]);
    draw_line(vertices[6], vertices[5]);
  } else {
    draw_line(vertices[0], vertices[3]);
    draw_line(vertices[3], vertices[7]);
    draw_line(vertices[7], vertices[4]);
    draw_line(vertices[4], vertices[0]);
  }
}

int main() {
  initscr();
  noecho();
  cbreak();

  while (1) {
    getmaxyx(stdscr, rows, cols);
    memcpy(vertices, base_vertices, sizeof(vertices));
    for (int i = 0; i < 8; i++) {
      rotate_x(&vertices[i]);
      rotate_y(&vertices[i]);
      rotate_z(&vertices[i]);
    }

    for (int i = 0; i < MAX_ROWS; i++) {
      for (int j = 0; j < MAX_COLS; j++) {
        depth_buffer[i][j] = far;
      }
    }

    clear();
    configure_normales();
    draw_edges();
    mvprintw(0, 0, "rotation (radians)\n");
    mvprintw(1, 0, "%.1f, %.1f, %.1f\n", A, B, C);
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        float depth = depth_buffer[i][j];
        if (depth != far) {
          char ch;
          if (depth > 0) ch = '@';
          if (depth < 0) ch = '.';
          mvaddch(i, j, ch);
        }
      }
    }
    refresh();

    A += 0.05;
    B += 0.05;
    C += 0.01;
    usleep(8000 * 3);
  }
  endwin();
}
