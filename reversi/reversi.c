#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

enum colour {EMPTY, BLACK, WHITE};
typedef enum colour colour;

enum {width = 1280, height = 720};

struct piece
{
  colour type;
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

void draw_screen(display *d, table *t)
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

//------------------------------------------
//testing

//does tests
void test()
{
  printf("All tests passed!\n");
}

int main(int n, char *args[n])
{
  // if(n == 2 && strcmp(args[1],"test") == 0) test();
  // else run();
  //test();
  table *t =  create_table(8);
  display *d = initiate_screen();
  draw_screen(d,t);
  SDL_Delay(5000);
  SDL_Quit();
  free_table(t);
  return 0;
}
