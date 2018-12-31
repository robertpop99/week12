#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

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
    int k = 4;
    while(prs[k] == 0 && k > 0) k--;
    if(k != 4) k++;
    for(int i = k; i > 0; i--)
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
  int k = 4;
  while(prs[k] == 0 && k > 0) k--;
  if(k != 4) k++;
  for(int i = k; i > 0; i--)
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
  int k = 4;
  while(k >= 0 && prs[k] == 0) k--;
  if(k != 4) k++;
  for(int i = k; i >= 0; i--)
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

  if(cursor.pen) pen();
  setx(x1 - cursor.x);
  sety(y1 - cursor.y);
  pen();
  setx(x2 - x1);
  sety(y2 - y1);
  pen();
}

//given a number of lines, it animates them simultaneously
void draw_lines(int n, int v[n][4])
{
  double times = sqrt( (square(v[0][0] - v[0][2]) +
                       square(v[0][1] - v[0][3])) ) / 5;

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
  int h_tr = h - 50, l = 2 * h_tr * sqrt(3) / 3;
  tso[0][0][0] = w / 2; tso[0][0][1] = (h - h_tr) / 2;
  tso[0][1][0] = (w - l) / 2; tso[0][1][1] = (h + h_tr) / 2;
  tso[0][2][0] = (w + l) / 2; tso[0][2][1] = (h + h_tr) / 2;
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

//draws n squares
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

//given two points, creates a sqaure between them
void crt_square(int x1, int y1, int x2, int y2, int sqr[4][2])
{
	sqr[0][0] = x1; sqr[1][0] = x1;
	sqr[0][1] = y1; sqr[1][1] = y2;
	sqr[2][0] = x2; sqr[3][0] = x2;
	sqr[2][1] = y2; sqr[3][1] = y1;
}

//given one square creates another 8 alongside the sides of the
//given one
void create_square(int sqr[4][2], int n, int tso[n][4][2])
{
	int l = (sqr[3][0] - sqr[0][0]) / 3;

	int pos = n - 1;

	crt_square(sqr[0][0], sqr[0][1], sqr[0][0] + l,
				sqr[0][1] + l, tso[pos]); pos--;
	crt_square(sqr[0][0], sqr[0][1] + l, sqr[0][0] + l,
				sqr[0][1] + 2 * l, tso[pos]); pos--;
	crt_square(sqr[0][0], sqr[0][1] + 2 * l, sqr[0][0] + l,
				sqr[1][1], tso[pos]); pos--;
	crt_square(sqr[1][0] + l, sqr[1][1] - l, sqr[1][0] + 2 * l,
				sqr[1][1], tso[pos]); pos--;
	crt_square(sqr[1][0] + 2 * l, sqr[1][1] - l, sqr[2][0],
				sqr[2][1], tso[pos]); pos--;
	crt_square(sqr[2][0] - l, sqr[2][1] - 2 * l, sqr[2][0],
				sqr[2][1] - l, tso[pos]); pos--;
	crt_square(sqr[2][0] - l, sqr[3][1], sqr[2][0],
				sqr[3][1] + l, tso[pos]); pos--;
	crt_square(sqr[0][0] + l, sqr[0][1], sqr[0][0] + 2 * l,
				sqr[0][1] + l, tso[pos]);
}

//draws a sqare fractal
void frac_square(int n, int w, int h)
{
  int nsq = pow(8,n-1), acc = 1, pos = 0;
  int tso[nsq][4][2];
  int l = h - 50;
  tso[0][0][0] = (w - l) / 2; tso[0][0][1] = (h - l) / 2;
  tso[0][1][0] = (w - l) / 2; tso[0][1][1] = (h + l) / 2;
  tso[0][2][0] = (w + l) / 2; tso[0][2][1] = (h + l) / 2;
  tso[0][3][0] = (w + l) / 2; tso[0][3][1] = (h - l) / 2;
  squares(acc,tso);
  for(int i = 1; i < n; i++)
  {
    pos = acc * 8 - 1;
    for(int j = acc - 1; j >= 0; j--)
    {
      create_square(tso[j], pos + 1, tso);
      pos -= 8;
    }
    acc *= 8;
    squares(acc, tso);
  }
}

//checks if the type of the fractal is correct
int check_type(char *imp)
{
  if(!strcmp(imp,"square")) return 1;
  else if(!strcmp(imp,"triangle")) return 1;
  else return 0;
}

//asks the type of the fractal
int ask_type()
{
  char imp[100];
  printf("What kind of fractal do you want? <square,triangle>\n");
  fgets(imp,sizeof(imp),stdin);
  imp[strcspn(imp, "\r\n")] = '\0';
  while(!check_type(imp))
  {
    printf("Spelling error, try again\n");
    printf("What kind of fractal do you want? <square,triangle>\n");
    fgets(imp,sizeof(imp),stdin);
    imp[strcspn(imp, "\r\n")] = '\0';
  }
  if(!strcmp(imp,"triangle")) return 1;
  else return 0;
}

//asks the depth of the fractal
int ask_depth()
{
  char imp[100];
  printf("How many times should the fractal repeat? (between 1 and 9)\n");
  printf("Big numbers might result in the program runnig out of memory");
  printf(" for the sqaure type. 6 and below should be safe\n");
  fgets(imp,sizeof(imp),stdin);
  imp[strcspn(imp, "\r\n")] = '\0';
  while(strlen(imp) != 1 || (imp[0] < '1' || imp[0] > '9'))
  {
    printf("Spelling error, try again\n");
    printf("How many times should the fractal repeat? (between 1 and 9)\n");
    fgets(imp,sizeof(imp),stdin);
    imp[strcspn(imp, "\r\n")] = '\0';
  }
  return imp[0] - '0';
}

//return the natural numbers represented by a string
//or -1 if the input is not valid
int is_natural(char *s)
{
  for(int i = 0; i < strlen(s); i++)
    if(!isdigit(s[i])) return -1;

  int n = atoi(s);
  if(n <= 0) return -1;
  else return n;
}

//asks for the width of the screen
int ask_width()
{
  char imp[100];
  int n;
  printf("What is the width of your screen?\n");
  fgets(imp,sizeof(imp),stdin);
  imp[strcspn(imp, "\r\n")] = '\0';
  n = is_natural(imp);
  while(n == -1)
  {
    printf("Spelling error, try again\n");
    printf("What is the width of your screen?\n");
    fgets(imp,sizeof(imp),stdin);
    imp[strcspn(imp, "\r\n")] = '\0';
    n = is_natural(imp);
  }
  return n;
}

//asks for the height of the screen
int ask_height()
{
  char imp[100];
  int n;
  printf("What is the height of your screen?\n");
  fgets(imp,sizeof(imp),stdin);
  imp[strcspn(imp, "\r\n")] = '\0';
  n = is_natural(imp);
  while(n == -1)
  {
    printf("Spelling error, try again\n");
    printf("What is the height of your screen?\n");
    fgets(imp,sizeof(imp),stdin);
    imp[strcspn(imp, "\r\n")] = '\0';
    n = is_natural(imp);
  }
  return n;
}

//asks for the pause between draws
int ask_pause()
{
  char imp[100];
  int n;
  printf("How big do you want the pause between draws (milliseconds)?\n");
  fgets(imp,sizeof(imp),stdin);
  imp[strcspn(imp, "\r\n")] = '\0';
  n = is_natural(imp);
  while(n == -1)
  {
    printf("Spelling error, try again\n");
    printf("How big do you want the pause between draws (milliseconds)?\n");
    fgets(imp,sizeof(imp),stdin);
    imp[strcspn(imp, "\r\n")] = '\0';
    n = is_natural(imp);
  }
  return n;
}

//runs the program with user input
void run()
{
  char imp[100];
  int depth, type, w, h, pause;
  printf("What is the name of the resulting file?");
  printf(" (.sketch added automatically)\n");
  fgets(imp,sizeof(imp),stdin);
  imp[strcspn(imp, "\r\n")] = '\0';
  strcat(imp,".sketch");

  type = ask_type();
  depth = ask_depth();
  w = ask_width();
  h = ask_height();
  pause = ask_pause();

  fout = fopen(imp,"wb");
  setdt(pause);

  if(type == 0) frac_square(depth, w, h);
  else frac_triangle(depth, w, h);

  fclose(fout);
}

//-----------------------------------------------------------------
//testing

//tests pen
void test_pen()
{
    fout = fopen("test.sketch","wb");
    pen();
    assert(cursor.pen == 1);
    pen();
    assert(cursor.pen == 0);
    fclose(fout);
    FILE *fin = fopen("test.sketch","rb");
    unsigned char b = fgetc(fin);
    assert(b == 0xC0);
    b = fgetc(fin);
    assert(b == 0xC0);
    fclose(fin);
}

//tests setpr
void test_pr()
{
  fout = fopen("test.sketch","wb");
  setpr(0x3F);
  setpr(0x06);
  setpr(0x21);
  setpr(0x00);
  fclose(fout);
  FILE *fin = fopen("test.sketch","rb");
  unsigned char b = fgetc(fin); assert(b == 0xBF);
  b = fgetc(fin); assert(b == 0x86);
  b = fgetc(fin); assert(b == 0xA1);
  b = fgetc(fin); assert(b == 0x80);
  fclose(fin);
}

//tests setx
void testx()
{
  fout = fopen("test.sketch","wb");
  setx(0x3FFFFFFF); assert(cursor.x == 0x3FFFFFFF);
  setx(0x0FF00000); assert(cursor.x == 0x4FEFFFFF);
  setx(0x00003B35); assert(cursor.x == 0x4FF03B34);
  fclose(fout);
  FILE *fin = fopen("test.sketch","rb");
  unsigned char b = fgetc(fin); assert(b == 0xBF);
  b = fgetc(fin); assert(b == 0xBF); b = fgetc(fin); assert(b == 0xBF);
  b = fgetc(fin); assert(b == 0xBF); b = fgetc(fin); assert(b == 0x3F);

  b = fgetc(fin); assert(b == 0x8F); b = fgetc(fin); assert(b == 0xBC);
  b = fgetc(fin); assert(b == 0x80); b = fgetc(fin);
  b = fgetc(fin); assert(b == 0x00);

  b = fgetc(fin); assert(b == 0x80); b = fgetc(fin); assert(b == 0x83);
  b = fgetc(fin); assert(b == 0xAC); b = fgetc(fin); assert(b == 0x35);
  fclose(fin);
}

//tests sety
void testy()
{
  fout = fopen("test.sketch","wb");
  sety(0x3FFFFFFF); assert(cursor.y == 0x3FFFFFFF);
  sety(0x000ABC00); assert(cursor.y == 0x400ABBFF);
  sety(0x00003BF5); assert(cursor.y == 0x400AF7F4);
  fclose(fout);
  FILE *fin = fopen("test.sketch","rb");
  unsigned char b = fgetc(fin); assert(b == 0xBF);
  b = fgetc(fin); assert(b == 0xBF); b = fgetc(fin); assert(b == 0xBF);
  b = fgetc(fin); assert(b == 0xBF); b = fgetc(fin); assert(b == 0x7F);

  b = fgetc(fin); assert(b == 0x80);
  b = fgetc(fin); assert(b == 0x82); b = fgetc(fin); assert(b == 0xAB);
  b = fgetc(fin); assert(b == 0xB0); b = fgetc(fin); assert(b == 0x40);

  b = fgetc(fin); assert(b == 0x80); b = fgetc(fin); assert(b == 0x83);
  b = fgetc(fin); assert(b == 0xAF); b = fgetc(fin); assert(b == 0x75);
  fclose(fin);
}

//tests setdt and calldt
void testdt()
{
  fout = fopen("test.sketch","wb");
  setdt(0xF); calldt();
  setdt(0x0); calldt();
  setdt(0xABC);
  fclose(fout);
  FILE *fin = fopen("test.sketch","rb");
  unsigned char b = fgetc(fin); assert(b == 0x80);
  b = fgetc(fin); assert(b == 0x8F); b = fgetc(fin); assert(b == 0xC1);
  b = fgetc(fin); assert(b == 0x80); b = fgetc(fin); assert(b == 0xC1);

  b = fgetc(fin); assert(b == 0x80); b = fgetc(fin); assert(b == 0xC1);
  b = fgetc(fin); assert(b == 0x80); b = fgetc(fin); assert(b == 0xC1);

  b = fgetc(fin); assert(b == 0x80); b = fgetc(fin); assert(b == 0xAA);
  b = fgetc(fin); assert(b == 0xBC); b = fgetc(fin); assert(b == 0xC1);
  fclose(fin);
}

//tests is_natural
void test_isnatural()
{
  char a[50];
  strcpy(a,"345");
  assert(is_natural(a) == 345);
  strcpy(a,"4.6456");
  assert(is_natural(a) == -1);
  strcpy(a,"234r5");
  assert(is_natural(a) == -1);
  strcpy(a,"-234");
  assert(is_natural(a) == -1);
  strcpy(a,"0");
  assert(is_natural(a) == -1);
  strcpy(a,"678888");
  assert(is_natural(a) == 678888);
  strcpy(a,"asc");
  assert(is_natural(a) == -1);
  strcpy(a,"0x34");
  assert(is_natural(a) == -1);
}

//run all tests
void test()
{
  test_pen();
  test_pr();
  testx();
  testy();
  testdt();
  test_isnatural();
  printf("All tests passed!\n");
}

int main(int n, char *args[n])
{
  if(n == 2 && strcmp(args[1],"test") == 0) test();
  else run();
  return 0;
}
