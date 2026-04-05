#include <math.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct Vertex {
  float x;
  float y;
  float z;
};

float A, B, C;
const char ch = '#';
const float face = 30.0f;
const float mid = face / 2.0f;
struct Vertex vertices[8] = {
    {-mid, -mid, mid}, {-mid, -mid, -mid}, {mid, -mid, -mid}, {mid, -mid, mid},
    {-mid, mid, mid},  {-mid, mid, -mid},  {mid, mid, -mid},  {mid, mid, mid},
};

void rotate_x(struct Vertex *vertex) {
  float y, z;
  y = vertex->y * cos(A) - vertex->z * sin(A);
  z = vertex->y * sin(A) + vertex->z * cos(A);
  vertex->y = y;
  vertex->z = z;
}

void rotate_y(struct Vertex *vertex) {
  float x, z;
  x = vertex->x * cos(B) + vertex->z * sin(B);
  z = vertex->z * cos(B) - vertex->x * sin(B);
  vertex->x = x;
  vertex->z = z;
}

void rotate_z(struct Vertex *vertex) {
  float x, y;
  x = vertex->x * cos(C) - vertex->y * sin(C);
  y = vertex->x * sin(C) + vertex->y * cos(C);
  vertex->x = x;
  vertex->y = y;
}

void draw_line(struct Vertex a, struct Vertex b) {
  int cols, rows;
  getmaxyx(stdscr, rows, cols);
  int ax = cols / 2 + (int)a.x;
  int ay = rows / 2 + (int)a.y / 2;
  int bx = cols / 2 + (int)b.x;
  int by = rows / 2 + (int)b.y / 2;
  int deltaX = abs(bx - ax);
  int deltaY = abs(by - ay);
  int signX = ax < bx ? 1 : -1;
  int signY = ay < by ? 1 : -1;
  int error = deltaX - deltaY;
  mvaddch(by, bx, ch);
  while (ax != bx || ay != by) {
    mvaddch(ay, ax, ch);
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

int main() {
  initscr();
  noecho();
  cbreak();
  while (1) {
    struct Vertex temp[8];
    memcpy(temp, vertices, sizeof(temp));
    for (int i = 0; i < 8; i++) {
      rotate_x(&temp[i]);
      rotate_y(&temp[i]);
      rotate_z(&temp[i]);
    }
    clear();
    mvprintw(0, 0, "X: %f, Y: %f, Z: %f\n", A, B, C);
    draw_line(temp[0], temp[1]);
    draw_line(temp[1], temp[2]);
    draw_line(temp[2], temp[3]);
    draw_line(temp[3], temp[0]);
    draw_line(temp[4], temp[5]);
    draw_line(temp[5], temp[6]);
    draw_line(temp[6], temp[7]);
    draw_line(temp[7], temp[4]);
    draw_line(temp[0], temp[4]);
    draw_line(temp[1], temp[5]);
    draw_line(temp[2], temp[6]);
    draw_line(temp[3], temp[7]);
    refresh();
    A += 0.2f;
    B += 0.2f;
    C += 0.2f;
    usleep(100000);
  }
  endwin();
}
