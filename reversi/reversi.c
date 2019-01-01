#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

enum player {EMPTY, BLACK, WHITE};
typedef enum player player;

enum {width = 1280, height = 720};

struct piece
{
  player type;
  int x;
  int y;
  int rnd;
};
typedef struct piece piece;

struct table
{
  int n;
  piece **grid;
};
typedef struct table table;

struct display {
    SDL_Window *w;
    SDL_Renderer *r;
    SDL_Surface *s;
};
typedef struct display display;

//creates a table with n * n squares
table *create_table(int n)
{
  table *t = malloc(sizeof(table));
  t->n = n;
  t->grid = malloc(sizeof(piece *) * n);
  for(int i = 0; i < n; i++)
  {
    t->grid[i] = malloc(sizeof(piece) * n);
    for(int j = 0; j < n; j++)
      {
        t->grid[i][j].type = EMPTY;
        t->grid[i][j].x = i;
        t->grid[i][j].y = j;
        t->grid[i][j].rnd = 0;
      }
  }
  return t;
}

//frees an allocated table
void free_table(table *t)
{
  for(int i = 0; i < t->n; i++)
    free(t->grid[i]);
  free(t->grid);
  free(t);
}

//initiates the screen
display *initiate_screen()
{
  setbuf(stdout, NULL);
  display *d = malloc(sizeof(display));
  SDL_Init(SDL_INIT_VIDEO);
  d->w = SDL_CreateWindow("Reversi",SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, width, height, 0);
  d->s = SDL_GetWindowSurface(d->w);
  d->r = SDL_CreateSoftwareRenderer(d->s);
  SDL_SetRenderDrawColor(d->r, 255, 255, 255, 255);
  SDL_RenderClear(d->r);
  SDL_UpdateWindowSurface(d->w);
  SDL_SetRenderDrawColor(d->r, 0, 0, 0, 255);
  return d;
}

//draws the table
void draw_table(display *d, table *t)
{
  int h = ( height - 100) / t->n;
  int x0 = width / 2 - h * (t->n / 2);
  int x1 = width / 2 + h * (t->n / 2);
  int y0 = height / 2 - h * (t->n / 2);
  int y1 = height / 2 + h * (t->n / 2);
  SDL_Rect rect = {x0, y0, h * t->n, h * t->n};
  SDL_FillRect(d->s, &rect, SDL_MapRGB(d->s->format, 0x22, 0x8B, 0x22));
  for(int i = 0; i <= t->n; i++)
  {
    SDL_RenderDrawLine(d->r, x0, y0 + i * h, x1, y0 + i * h);
    SDL_RenderDrawLine(d->r, x0 + i * h, y0, x0 + i * h, y1);
  }

  SDL_UpdateWindowSurface(d->w);
}

//draws a circle given the center and radius
//it uses simple geometry, calculating the distance from
//the diameter to the edge for every pixel
void draw_circle(display *d, int x, int y, int r)
{
  for(int i = y - r; i <= y + r; i++)
  {
    int x0 = sqrt(r * r - (y - i) * (y - i));
    SDL_RenderDrawLine(d->r, x - x0, i, x + x0, i);
  }
}

//draws the pieces on the board
void draw_pieces(display *d, table *t)
{
  int h = ( height - 100) / t->n;
  int x0 = width / 2 - h * (t->n / 2);
  int y0 = height / 2 - h * (t->n / 2);

  for(int i = 0; i < t->n; i++)
  {
    for(int j = 0; j < t->n; j++)
    {
      if(t->grid[i][j].type != EMPTY)
      {
        if(t->grid[i][j].type == BLACK)
          SDL_SetRenderDrawColor(d->r, 0x00, 0x00, 0x00, 0xFF);
        else
          SDL_SetRenderDrawColor(d->r, 0xFF, 0xFF, 0xFF, 0xFF);
        int x = x0 + h * i + h / 2;
        int y = y0 + h * i + h / 2;
        draw_circle(d, x, y, h / 3);
      }
    }
  }
  SDL_UpdateWindowSurface(d->w);
  SDL_SetRenderDrawColor(d->r, 0x00, 0x00, 0x00, 0xFF);
}

//checks if a move from the first 4 rounds id valid (first 2 moves
//of each player)
int valid_first_moves(table *t, int i, int j)
{
  if(t->grid[i][j].type != EMPTY) return 0;
  int v[2] = {t->n / 2, t->n / 2 - 1};
  for(int k = 0; k < 2; k++)
    for(int l = 0; l < 2; l++)
      if(i == v[k] && j == v[l]) return 1;
  return 0;
}

//checks if a piece is the end of a line of pieces
int end_line(table *t, int i, int j)
{
  if(t->grid[i][j].type == EMPTY) return 0;
  int c = 0, v[3] = {1, 0, -1};
  for(int k = 0; k < 3; k++)
    for(int l = 0; l < 3; l++)
      if(t->grid[ i + v[k] ][ j + v[l] ].type != EMPTY) c++;
  c--;
  if(c == 1) return 1;
  else return 0;
}

//checks if a move after the first 4 moves is valid
int valid_late_moves(table *t, int i, int j, player p)
{
  int op_player = 0;
  for(int k = 0; k < t->n; k++)
    for(int l = 0; l< t->n; l++)
    {
      if(t->grid[k][l].type == p) return 0;
    }
}

//checks if a move is valid
int valid_move(table *t, int i, int j, player p, int r)
{
    if(r <= 4) return valid_first_moves(t, i, j);
    else return valid_late_moves(t, i, j, p);
}

// //takes input from the user, using the pressing of the mouse
// void mouse_click(display *d, table *t)
// {
//   int ok = 0;
//   while(ok > 0)
//   { ok--; printf("%d\n",ok );
//     SDL_Event e;
//     if(e.type == SDL_MOUSEBUTTONDOWN)
//                          //&& e.button.type == SDL_BUTTON_LEFT)
//     { printf("yes\n");
//       draw_circle(d, e.button.x, e.button.y, 50);
//       SDL_UpdateWindowSurface(d->w);
//       ok = 0;
//     }
//   }
// }

//------------------------------------------
//testing

//tests if the valid_first_moves function works properly
void test_first_moves()
{
  table *t = create_table(8);
  assert(valid_first_moves(t, 0, 0) == 0);
  assert(valid_first_moves(t, 4, 3) == 1);
  assert(valid_first_moves(t, 4, 4) == 1);
  assert(valid_first_moves(t, 3, 3) == 1);
  assert(valid_first_moves(t, 3, 4) == 1);
  assert(valid_first_moves(t, 7, 2) == 0);
  assert(valid_first_moves(t, 4, 5) == 0);
  assert(valid_first_moves(t, 2, 2) == 0);
  assert(valid_first_moves(t, 1, 5) == 0);
  assert(valid_first_moves(t, 0, 4) == 0);
  free_table(t); t = create_table(4);
  assert(valid_first_moves(t, 0, 0) == 0);
  assert(valid_first_moves(t, 1, 1) == 1);
  assert(valid_first_moves(t, 1, 2) == 1);
  assert(valid_first_moves(t, 2, 1) == 1);
  assert(valid_first_moves(t, 3, 3) == 0);
  assert(valid_first_moves(t, 2, 3) == 0);
  assert(valid_first_moves(t, 1, 0) == 0);
  assert(valid_first_moves(t, 0, 3) == 0);
  assert(valid_first_moves(t, 3, 1) == 0);
  free_table(t);
}

//tests end_line function
void test_end_line()
{
  table *t = create_table(8);
  assert(end_line(t, 5, 4) == 0);
  t->grid[3][6].type = BLACK; assert(end_line(t, 3, 6) == 0);
  t->grid[3][5].type = BLACK; assert(end_line(t, 3, 6) == 1);
  t->grid[4][6].type = BLACK; assert(end_line(t, 3, 6) == 0);
  
  free_table(t);
}

//does tests
void test()
{
  test_first_moves();
  test_end_line();
  printf("All tests passed!\n");
}

int main(int n, char *args[n])
{
  // if(n == 2 && strcmp(args[1],"test") == 0) test();
  // else run();
  test();
  // table *t =  create_table(8);
  // display *d = initiate_screen();
  // draw_table(d, t);
  // t->grid[0][0].type = WHITE;
  // t->grid[7][7].type = BLACK;
  // draw_pieces(d , t);
  // //mouse_click(d, t);
  // SDL_Delay(5000);
  // SDL_Quit();
  // free_table(t);
  return 0;
}
