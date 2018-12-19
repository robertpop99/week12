#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

struct cursor{
 long x;   long y;
 long dx;  long dy;
 bool pen;
 long pr; bool pr_init; int pr_len;
 long dt;
} cursor;

//returs the first two bits
long getOp(unsigned char b)
{
  return ((0xC0 & b) >> 6);
}

//returns the last 6 bits
long getPos(unsigned char b)
{
  return (0x3F & b);
}

//draws a line on the screen and updates the cursor
void draw(display *d)
{
  if(cursor.pen) line(d, cursor.x, cursor.y,
                        cursor.x + cursor.dx, cursor.y + cursor.dy);
  cursor.x += cursor.dx;
  cursor.y += cursor.dy;
  cursor.dx = 0;
  cursor.dy = 0;
}

//stores pos the current PR
void op2(long pos)
{
  cursor.pr_init = true;
  cursor.pr = (cursor.pr << 6) | pos;
  cursor.pr_len += 6;
}

//resets PR
void reset_pr()
{
  cursor.pr = 0;
  cursor.pr_len = 0;
  cursor.pr_init = false;
}

//returns the value inside PR with pos after it
long getValue(long pos)
{
  long res = 0;
  if(cursor.pr_init)
  {
    if(cursor.pr == 0)
      res = pos;
    else
    {
      op2(pos);
      if( cursor.pr & ((long)1 << (cursor.pr_len - 1)) )
        res = ( cursor.pr & ~((long)1 << (cursor.pr_len - 1)) )
                      - ((long)1 << (cursor.pr_len - 1));
      else
        res = cursor.pr;
    }
    reset_pr();
  }
  else
  {
    if(pos & 0x20)
      res = (pos & 0x1F) - 32;
    else
      res = pos;
  }
  return res;
}

//opcode 0, changes dx
void op0(long pos)
{
  cursor.dx = getValue(pos);
}

//opcode 1, changes dy and draws a line
void op1(display *d, long pos)
{
  cursor.dy = getValue(pos);
  draw(d);
}

//opcode 4, adds a pause
void do1(display *d)
{
  if(cursor.pr != 0) cursor.dt = cursor.pr;
  reset_pr();
  pause(d,(int)cursor.dt);
}

//opcode 7, changes the color
void do4(display *d)
{
  colour(d, (int)cursor.pr);
  reset_pr();
}

///opcode 3, based on the operand choses the opcode
void op3(display *d, long pos)
{
  if(pos == 0) cursor.pen = !cursor.pen;
  else if(pos == 1) do1(d);
  else if(pos == 2) clear(d);
  else if(pos == 3) key(d);
  else if(pos == 4) do4(d);
}

//given a byte, runs the operation represented by it
void run(display *d, unsigned char b)
{
    long op = getOp(b);
    long pos = getPos(b);
    switch(op)
    {
      case 0 : op0(pos); break;
      case 1 : op1(d, pos); break;
      case 2 : op2(pos); break;
      case 3 : op3(d, pos); break;
    }
}

//reads the file, byte by byte
void input(char *file)
{
  display *d = newDisplay(file, 1280, 720);
  cursor = (struct cursor) {0, 0, 0, 0, 0, 0, 0, 0, 0};
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

//tests getoOp and getPos
void testget()
{
  assert(getOp(0x03) == 0); assert(getOp(0x3D) == 0);
  assert(getOp(0x7D) == 1); assert(getOp(0x44) == 1);
  assert(getOp(0x8F) == 2); assert(getOp(0xBC) == 2);
  assert(getOp(0xFF) == 3); assert(getOp(0xD0) == 3);
  assert(getPos(0x03) == 3); assert(getPos(0x3D) == 61);
  assert(getPos(0x7D) == 61); assert(getPos(0x44) == 4);
  assert(getPos(0x8F) == 15); assert(getPos(0xBC) == 60);
  assert(getPos(0xFF) == 63); assert(getPos(0xD0) == 16);
}

//tests opcode 0 and 3
void testrun()
{
  cursor = (struct cursor) {0, 0, 0, 0, 0, 0, 0, 0, 0};
  run(NULL,0x03);
  assert(cursor.dx == 3);
  run(NULL,0xC0);
  assert(cursor.pen == 1);
  run(NULL,0x3F);
  assert(cursor.dx == -1);
  run(NULL,0xC0);
  assert(cursor.pen == 0);
}

//tests getValue
void testvalue()
{
  cursor = (struct cursor) {0, 0, 0, 0, 0, 0, 0, 0, 0};
  run(NULL,0x9F);
  assert(cursor.pr == 31);
  run(NULL,0x81);
  assert(cursor.pr == 0x7C1);
  reset_pr();
  run(NULL,0x80);
  run(NULL,0x3F);
  assert(cursor.dx == 0x3F);
  reset_pr();
  run(NULL,0xA0);
  assert(getValue(0x04) == -2044);
}

void test()
{
  testget();
  testrun();
  testvalue();
  printf("All tests passed!\n");
}

int main(int n, char *args[n]) {
    if (n != 2) { fprintf(stdout, "Use ./sketch filename\n"); exit(1); }
    if (!strcmp(args[1],"test")) test();
    else input(args[1]);
}
