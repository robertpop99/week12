#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct cursor{
 int x;   int y;
 int dx;  int dy;
 int pen;
} cursor;

int getOp(unsigned char b)
{
  return ((0xC0 & b) >> 6);
}

int getPos(unsigned char b)
{
  int p = (0x0000001F & b);
  if(0x20 & b) p -= 32;
  return p;
}

void draw(display *d)
{
  if(cursor.pen) line(d, cursor.x, cursor.y,
                        cursor.x + cursor.dx, cursor.y + cursor.dy);
  cursor.x += cursor.dx;
  cursor.y += cursor.dy;
  cursor.dx = 0;
  cursor.dy = 0;
}

void run(display *d, unsigned char b)
{
    int op = getOp(b);
    int pos = getPos(b);
    if(op == 0) cursor.dx += pos;
    if(op == 1)
    {
      cursor.dy = pos;
      draw(d);
    }
    if(op == 3) cursor.pen = !cursor.pen;
}

void input(char *file)
{
  display *d = newDisplay(file, 200, 200);
  cursor = (struct cursor) {0, 0, 0, 0, 0};
  FILE *in = fopen(file,"rb");
  unsigned char b = fgetc(in);
  while(!feof(in))
  {
    run(d, b);
    b = fgetc(in);
  }
  end(d);
  free(d);
  fclose(in);
}

//------------------------------------------------------------------
//testing

void testget()
{
  assert(getOp(0x03) == 0); assert(getOp(0x3D) == 0);
  assert(getOp(0x7D) == 1); assert(getOp(0x44) == 1);
  assert(getOp(0x8F) == 2); assert(getOp(0xBC) == 2);
  assert(getOp(0xFF) == 3); assert(getOp(0xD0) == 3);
  assert(getPos(0x03) == 3); assert(getPos(0x3D) == -3);
  assert(getPos(0x7D) == -3); assert(getPos(0x44) == 4);
  assert(getPos(0x8F) == 15); assert(getPos(0xBC) == -4);
  assert(getPos(0xFF) == -1); assert(getPos(0xD0) == 16);
}

void testrun()
{
  cursor = (struct cursor) {0, 0, 0, 0, 0};
  run(NULL,0x03);
  assert(cursor.dx == 3);
  run(NULL,0xC0);
  assert(cursor.pen == 1);
  run(NULL,0x3F);
  assert(cursor.dx == 2);
  run(NULL,0xC0);
  assert(cursor.pen == 0);
}

void test()
{
  testget();
  testrun();
  printf("All tests passed!\n");
}

int main(int n, char *args[n]) {
    if (n != 2) { fprintf(stdout, "Use ./sketch filename\n"); exit(1); }
    if (!strcmp(args[1],"test")) test();
    else input(args[1]);
}
