#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

FILE *fout;

struct cursor{
  int x;
  int y;
  bool pen;
} cursor = {0, 0, 0};

//writes the char for pen or opcode 3
void pen()
{
  fputc(0xC0,fout);
  cursor.pen = !cursor.pen;
}

//writes the char for extended operands, or opcode 2
void setpr(unsigned char x)
{
    x = 0x80 | x;
    fputc(x,fout);
}

//writes the char for dx or opcode 0
void setx(int x)
{
    int prs[6];
    for(int i = 0; i < 5; i++)
      prs[i] = ( (x >> (6 * i)) & 0x3F);
    for(int i = 4; i > 0; i--)
      setpr(prs[i]);
    fputc(prs[0] | 0x00,fout);
    cursor.x += x;
}

//writes the char for dy or opcode 1
void sety(int x)
{
  int prs[6];
  for(int i = 0; i < 5; i++)
    prs[i] = ( (x >> (6 * i)) & 0x3F);
  for(int i = 4; i > 0; i--)
    setpr(prs[i]);
  fputc(prs[0] | 0x40,fout);
  cursor.y += x;
}

//writes the time for a pause, or opcode 4
void setdt(int x)
{
  int prs[6];
  for(int i = 0; i < 5; i++)
    prs[i] = ( (x >> (6 * i)) & 0x3F);
  for(int i = 4; i >= 0; i--)
    setpr(prs[i]);
  fputc(0xC1,fout);
}

//inserts another pause with the last used time,
//or opcode 4 with operand 0
void calldt()
{
  fputc(0x80,fout);
  fputc(0xC1,fout);
}

//squares an integer
int square(int x){
  return x * x;
}

//given 2 points, writes the code for drawing a line
//betweem them
void draw_line(int x1, int y1, int x2, int y2)
{
  //printf("%d %d %d %d\n",x1,y1,x2,y2 );
  //printf("%d %d\n",x2 - x1, y2 - y1 );
  if(cursor.pen) pen();
  setx(x1 - cursor.x);
  sety(y1 - cursor.y);
  pen();
  setx(x2 - x1);
  sety(y2 - y1);
  pen();
}

//given a number of equal lines, it animates them simultaneously
void draw_lines(int n, int v[n][4])
{
  double times = sqrt( square(v[0][0] + v[0][2]) +
                       square(v[0][1] + v[0][3]) ) / 5;

  for(int j = 0; j < times; j++)
  {
    for(int i = 0; i < n; i++)
    {
      double dx = ( v[i][2] - v[i][0] ) / times / 2;
      double dy = ( v[i][3] - v[i][1] ) / times / 2;
      double mx = (v[i][0] + v[i][2]) / 2;
      double my = (v[i][1] + v[i][3]) / 2;
      if(j >= times - 1){
        draw_line(mx + j * dx, my + j * dy,
                  v[i][2], v[i][3]);
        draw_line(mx - j * dx, my - j * dy,
                  v[i][0], v[i][1]);
      }
      else{
        draw_line(mx + j * dx, my + j * dy,
                  mx + (j + 1) * dx, my + (j + 1) * dy);
        draw_line(mx - j * dx, my - j * dy,
                  mx - (j + 1) * dx, my - (j + 1) * dy);
      }
    }
    calldt();
  }
}

//draws n triangles
void triangle(int n, int points[n][3][2])
{
  int v[3 * n][4];
  for(int i = 0; i < n; i++)
  {
    v[3 * i][0] = points[i][0][0]; v[3 * i + 1][0] = points[i][0][0];
    v[3 * i][1] = points[i][0][1]; v[3 * i + 1][1] = points[i][0][1];
    v[3 * i][2] = points[i][1][0]; v[3 * i + 1][2] = points[i][2][0];
    v[3 * i][3] = points[i][1][1]; v[3 * i + 1][3] = points[i][2][1];
    v[3 * i + 2][0] = points[i][1][0]; v[3 * i + 2][2] = points[i][2][0];
    v[3 * i + 2][1] = points[i][1][1]; v[3 * i + 2][3] = points[i][2][1];
  }

  draw_lines(3 * n, v);
}

//given one triangle, creates other 3 with the original points and the
//middles of the sides
void create_tri(int tri[3][2], int n, int tso[n][3][2])
{
  int m[3][2];
  m[0][0] = (tri[0][0] + tri[1][0]) / 2; m[0][1] = (tri[0][1] + tri[1][1]) / 2;
  m[1][0] = (tri[0][0] + tri[2][0]) / 2; m[1][1] = (tri[0][1] + tri[2][1]) / 2;
  m[2][0] = (tri[1][0] + tri[2][0]) / 2; m[2][1] = (tri[1][1] + tri[2][1]) / 2;
  int pos = n - 1;
  tso[pos][0][0] = tri[0][0]; tso[pos][0][1] = tri[0][1];
  tso[pos][1][0] = m[0][0]; tso[pos][1][1] = m[1][1];
  tso[pos][2][0] = m[1][0]; tso[pos][2][1] = m[1][1];
  pos--;
  tso[pos][0][0] = m[0][0]; tso[pos][0][1] = m[0][1];
  tso[pos][1][0] = tri[1][0]; tso[pos][1][1] = tri[1][1];
  tso[pos][2][0] = m[2][0]; tso[pos][2][1] = m[2][1];
  pos--;
  tso[pos][0][0] = m[1][0]; tso[pos][0][1] = m[1][1];
  tso[pos][1][0] = m[2][0]; tso[pos][1][1] = m[2][1];
  tso[pos][2][0] = tri[2][0]; tso[pos][2][1] = tri[2][1];
}

//draws a triangle fractal
void frac_triangle(int n, int w, int h)
{
  int nts = pow(3,n-1), acc = 1, pos = 0;
  int tso[nts][3][2];
  tso[0][0][0] = w / 2; tso[0][0][1] = 10;
  tso[0][1][0] = 10; tso[0][1][1] = h - 10;
  tso[0][2][0] = w - 10; tso[0][2][1] = h - 10;
  triangle(acc, tso);
  for(int i = 1; i < n; i++)
  {
    pos = acc * 3 - 1;
    for(int j = acc - 1; j >= 0; j--)
    {
      create_tri(tso[j], pos + 1, tso);
      pos -= 3;
    }
    acc *= 3;
    triangle(acc, tso);
  }
}

void squares(int n, int points[n][4][2])
{
  int v[4 * n][4];
  for(int i = 0; i < n; i++)
  {
    v[4 * i][0] = points[i][0][0]; v[4 * i + 1][0] = points[i][1][0];
    v[4 * i][1] = points[i][0][1]; v[4 * i + 1][1] = points[i][1][1];
    v[4 * i][2] = points[i][1][0]; v[4 * i + 1][2] = points[i][2][0];
    v[4 * i][3] = points[i][1][1]; v[4 * i + 1][3] = points[i][2][1];
    v[4 * i + 2][0] = points[i][2][0]; v[4 * i + 3][0] = points[i][3][0];
    v[4 * i + 2][1] = points[i][2][1]; v[4 * i + 3][1] = points[i][3][1];
    v[4 * i + 2][2] = points[i][3][0]; v[4 * i + 3][2] = points[i][0][0];
    v[4 * i + 2][3] = points[i][3][1]; v[4 * i + 3][3] = points[i][0][1];
  }

  draw_lines(4 * n, v);
}

int main()
{
  fout = fopen("out.sketch","w");
  setdt(0);
  //int v[1][4];
  //v[0][0] = 55; v[0][1] = 100; v[0][2] = 100; v[0][3] = 10;
  //draw_lines(1,v);
  //frac_triangle(5, 1280, 720);
  int v[1][4][2];
  v[0][0][0] = 10;   v[0][0][1] = 10;
  v[0][1][0] = 10;   v[0][1][1] = 700;
  v[0][2][0] = 1200;   v[0][2][1] = 700;
  v[0][3][0] = 1200;   v[0][3][1] = 10;
  squares(1,v);
  fclose(fout);
  return 0;
}
