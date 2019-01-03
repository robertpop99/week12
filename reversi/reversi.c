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

int width = 1280, height = 720;

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
  SDL_SetRenderDrawBlendMode(d->r, SDL_BLENDMODE_BLEND);
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
        if(t->grid[i][j].type == BLACK)
          SDL_SetRenderDrawColor(d->r, 0x00, 0x00, 0x00, 0xFF);
        else if(t->grid[i][j].type == WHITE)
          SDL_SetRenderDrawColor(d->r, 0xFF, 0xFF, 0xFF, 0xFF);
        else
          SDL_SetRenderDrawColor(d->r, 0x22, 0x8B, 0x22, 0xFF);
        int x = x0 + h * i + h / 2;
        int y = y0 + h * j + h / 2;
        draw_circle(d, x, y, h / 3);
    }
  }
  SDL_UpdateWindowSurface(d->w);
  SDL_SetRenderDrawColor(d->r, 0x00, 0x00, 0x00, 0xFF);
}

//checks if a move from the first 4 rounds id valid (first 2 moves
//of each player)
int valid_first_moves(table *t, int x, int y)
{
  if(t->grid[x][y].type != EMPTY) return 0;
  int v[2] = {t->n / 2, t->n / 2 - 1};
  for(int i = 0; i < 2; i++)
    for(int j = 0; j < 2; j++)
      if(x == v[i] && y == v[j]) return 1;
  return 0;
}

//checks if there is a line between two points
int is_line(int x0, int y0, int x1, int y1)
{
    if(x0 == x1 && y0 != y1) return 1;
    if(y0 == y1 && x0 != x1) return 1;
    if(x1 - x0 == y1 - y0 && x1 - x0 != 0) return 1;
    if(x1 - x0 == y0 - y1 && x1 - x0 != 0) return 1;
    return 0;
}

//checks if a vertical line between two pieces is a valid move
int valid_line(table *t, int x0, int y0, int x1, int y1, player p)
{
  int op_player = 0, c1 = 1, c2 = 1;
  int same_player = 0, empty_player = 0;
  if(x1 < x0) c1 = -1;
  else if(x1 == x0) c1 = 0;
  if(y1 < y0) c2 = -1;
  else if(y1 == y0) c2 = 0;

  for(int k = x0; k != x1; k += c1)
    for(int l = y0; l != y1; l += c2)
      {
        if(t->grid[k][l].type == EMPTY) empty_player++;
        if(t->grid[k][l].type != p) op_player++;
        if(t->grid[k][l].type == p) same_player++;
      }

  if(op_player && same_player == 1 && empty_player == 1) return 1;
  else return 0;
}

//checks if a coordinate is inside the table, n being the size of the table
int bound(int n, int x)
{
  if(x < 0 || x >= n) return 0;
  return 1;
}

//checks if a move after the first 4 moves is valid
int valid_late_moves(table *t, int x, int y, player p)
{
  if(t->grid[x][y].type != EMPTY) return 0;
  int v[3] = {-1, 0, 1};
  player op = BLACK; if(p == BLACK) op = WHITE;
  int cont = 0;
  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++)
    {
      if(v[i] == 0 && v[j] == 0) continue;
      int c = 0, xd = v[i], yd = v[j];
      while(bound(t->n, x + xd) && bound(t->n, y + yd)
            && t->grid[x + xd][y + yd].type == op)
      {
        c++;
        xd += v[i];
        yd += v[j];
      }
      if(c != 0 && bound(t->n,x + xd) && bound(t->n,y + yd)
         && t->grid[x + xd][y + yd].type == p) cont++;
    }
  if(cont == 0) return 0;
  else return 1;
}

//checks if a move is valid
int valid_move(table *t, int x, int y, player p, int r)
{
    if(x < 0 || x >= t->n || y < 0 || y >= t->n) return 0;
    if(r <= 4) return valid_first_moves(t, x, y);
    else return valid_late_moves(t, x, y, p);
}

//places a piece on the board and flips what needs to be flipped
void make_move(table *t, int x, int y, player p)
{
  t->grid[x][y].type = p;
  int v[3] = {-1, 0, 1};
  player op = BLACK; if(p == BLACK) op = WHITE;
  for(int i = 0; i < 3; i++)
    for(int j = 0; j < 3; j++)
    {
      if(v[i] == 0 && v[j] == 0) continue;
      int c = 0, xd = v[i], yd = v[j];
      while(bound(t->n, x + xd) && bound(t->n, y + yd)
            && t->grid[x + xd][y + yd].type == op){
        c++;
        xd += v[i]; yd += v[j];
      }
      if(c != 0 && bound(t->n,x + xd) && bound(t->n,y + yd)
         && t->grid[x + xd][y + yd].type == p)
      {
        xd = v[i], yd = v[j];
        while(t->grid[x + xd][y + yd].type == op){
            t->grid[x + xd][y + yd].type = p;
            xd += v[i]; yd += v[j];
          }
      }
    }
}

//verifies if the mouse has been clicked
//and, if so, returns the coordinates of the click
void mouse_click(display *d, int click[2])
{
  int flag = 0;
  SDL_Event e;
  while(SDL_PollEvent(&e))
  {
    if(e.type == SDL_MOUSEBUTTONDOWN
        && e.button.button == SDL_BUTTON_LEFT && !flag)
    {
      flag = 1;// printf("%s\n", "yes");
      click[0] = e.button.x;
      click[1] = e.button.y;
    }
  }
}

//transforms the x coordinate from the input from the mouse
//in a row in the table
int transform_x(table *t, int i)
{
  int h = ( height - 100) / t->n;
  int x0 = width / 2 - h * (t->n / 2);
  int x = 0;
  if(i < x0 || i > x0 + h * t->n)  return -1;
  while(i - h * (x + 1) > x0) x++;
  return x;
}

//transforms the y coordinate from the input from the mouse
//in a column the table
int transform_y(table *t, int j)
{
  int h = ( height - 100) / t->n;
  int y0 = height / 2 - h * (t->n / 2);
  int y = 0;
  if(j < y0 || j > y0 + h * t->n) return -1;
  while(j - h * (y + 1) > y0) y++;
  return y;
}

//checks if the game is finished
int finished_game(table *t, int r)
{
    int cont = 0, flag = 0;
    for(int i = 0; i < t->n; i++)
      for(int j = 0; j < t->n; j++)
      {
        if(t->grid[i][j].type == EMPTY)
        {
          cont++;
          if(valid_move(t, i, j, BLACK, r))
            flag = 1;
          else if(valid_move(t, i, j, WHITE, r))
            flag = 1;
        }
      }
    if(cont != 0 && flag == 1) return 0;
    else return 1;
}

//checks if there is an available move for the current player
//and, if so, draws a grey circle at that position
int available_move(display *d, table *t, player p, int r)
{
  int h = ( height - 100) / t->n;
  int x0 = width/2 - h*(t->n/2); int y0 = height/2 - h*(t->n/2);
  int cont = 0;
  for(int i = 0; i < t->n; i++)
    for(int j = 0; j < t->n; j++)
    {
      if(valid_move(t, i, j, p, r))
      {
        cont++;
        SDL_SetRenderDrawColor(d->r, 0xD3, 0xD3, 0xD3, 0x80);
        int x = x0 + h * i + h / 2;
        int y = y0 + h * j + h / 2;
        draw_circle(d, x, y, h / 3);
      }
    }
  if(cont){
    SDL_UpdateWindowSurface(d->w);
    SDL_SetRenderDrawColor(d->r, 0x00, 0x00, 0x00, 0xFF);
    return 1;
  }
  else
    return 0;
}

//waits for input until the move is legal ,then performs it
void next_move(display *d, table *t, player p, int r)
{
  int x = 0, y = 0;
  int v[2] = {0, 0}; int v1 = 0; int flag = 0;
  x = transform_x(t, v[0]), y = transform_y(t, v[1]);
  while(!flag)
  {
    mouse_click(d, v);
    if(v[0] != v1)
    {
      x = transform_x(t, v[0]), y = transform_y(t, v[1]);
      flag = valid_move(t, x , y, p, r);
      v1 = v[0];
    }
  }
  make_move(t, x, y, p);
  draw_pieces(d, t);
}

//shows the winenr on the screen
void show_winner(table *t)
{
  int b = 0, w = 0;
  for(int i = 0; i < t->n; i++)
    for(int j = 0; j < t->n; j++)
    {
      if(t->grid[i][j].type == BLACK) b++;
      else if(t->grid[i][j].type == WHITE) w++;
    }
  if(b > w) printf("Black player is the winner!\n");
  else if(w > b) printf("White player is the winner!\n");
  else printf("Draw!\n");
}

//shows the next player
void show_next_player(player p)
{
  if(p == BLACK) printf("Black's turn\n");
  else printf("White's turn\n");
}

//runs the program
void run(int n, int w, int h)
{
  table *t =  create_table(n);
  width = w, height = h;
  display *d = initiate_screen();
  draw_table(d, t);
  draw_pieces(d , t);
  player turn = BLACK; int r = 0;
  while(!finished_game(t, r + 1))
  {
    r++; show_next_player(turn);
    if(!available_move(d, t, turn, r))
    {
      if(turn == BLACK) turn = WHITE;
      else turn = BLACK;
      continue;
    }
    next_move(d, t, turn ,r);
    if(turn == BLACK) turn = WHITE;
    else turn = BLACK;
  }
  SDL_Delay(5000);
  SDL_Quit();
  show_winner(t);
  free_table(t);
}

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

//tests is_line
void test_is_line()
{
  assert(is_line(5, 4, 5, 4) == 0);
  assert(is_line(5, 2, 5, 7) == 1);
  assert(is_line(5, 6, 5, 0) == 1);
  assert(is_line(0, 4, 6, 4) == 1);
  assert(is_line(5, 2, 1, 2) == 1);
  assert(is_line(5, 4, 7, 6) == 1);
  assert(is_line(2, 2, 6, 6) == 1);
  assert(is_line(5, 3, 2, 0) == 1);
  assert(is_line(6, 6, 3, 3) == 1);
  assert(is_line(3, 4, 5, 2) == 1);
  assert(is_line(6, 2, 2, 6) == 1);
  assert(is_line(6, 2, 1, 6) == 0);
  assert(is_line(3, 2, 5, 5) == 0);
  assert(is_line(5, 2, 4, 7) == 0);
  assert(is_line(5, 2, 1, 1) == 0);
  assert(is_line(5, 4, 7, 7) == 0);
  assert(is_line(3, 4, 5, 3) == 0);
  assert(is_line(0, 0, 1, 2) == 0);
}

//tests valid_late_moves
void test_later_moves()
{
  table *t = create_table(8);
  t->grid[2][2].type = BLACK; t->grid[3][3].type = BLACK;
  t->grid[4][3].type = BLACK; t->grid[3][4].type = BLACK;
  t->grid[3][5].type = BLACK; t->grid[2][5].type = WHITE;
  t->grid[4][4].type = WHITE; t->grid[5][5].type = WHITE;
  t->grid[6][4].type = WHITE; t->grid[7][3].type = WHITE;
  assert(valid_late_moves(t, 4, 5, WHITE) == 1);
  assert(valid_late_moves(t, 4, 5, BLACK) == 1);
  assert(valid_late_moves(t, 6, 5, BLACK) == 0);
  assert(valid_late_moves(t, 5, 4, WHITE) == 0);
  assert(valid_late_moves(t, 0, 0, WHITE) == 0);
  assert(valid_late_moves(t, 0, 0, BLACK) == 0);
  assert(valid_late_moves(t, 5, 5, BLACK) == 0);
  assert(valid_late_moves(t, 5, 5, WHITE) == 0);
  assert(valid_late_moves(t, 1, 1, WHITE) == 1);
  assert(valid_late_moves(t, 1, 6, BLACK) == 1);
  assert(valid_late_moves(t, 2, 4, BLACK) == 0);
  assert(valid_late_moves(t, 3, 6, WHITE) == 0);
  assert(valid_late_moves(t, 4, 6, BLACK) == 0);
  assert(valid_late_moves(t, 5, 2, WHITE) == 1);
  assert(valid_late_moves(t, 7, 4, WHITE) == 0);
  assert(valid_late_moves(t, 7, 4, BLACK) == 0);
  assert(valid_late_moves(t, 3, 2, WHITE) == 0);
  assert(valid_late_moves(t, 6, 6, BLACK) == 1);
  assert(valid_late_moves(t, 7, 7, BLACK) == 0);
  assert(valid_late_moves(t, 5, 6, BLACK) == 0);
  free_table(t);
}

//tests make_move
void test_make_move()
{
  table *t = create_table(8);
  t->grid[2][2].type = BLACK; t->grid[3][3].type = BLACK;
  t->grid[4][3].type = BLACK; t->grid[3][4].type = BLACK;
  t->grid[3][5].type = BLACK; t->grid[2][5].type = WHITE;
  t->grid[4][4].type = WHITE; t->grid[5][5].type = WHITE;
  t->grid[6][4].type = WHITE; t->grid[7][3].type = WHITE;
  make_move(t, 5, 4, BLACK);
  assert(t->grid[4][4].type == BLACK); assert(t->grid[6][4].type == WHITE);
  make_move(t, 4, 5, WHITE);
  assert(t->grid[3][5].type == WHITE); assert(t->grid[4][5].type == WHITE);
  assert(t->grid[5][4].type == BLACK); assert(t->grid[4][4].type == BLACK);
  make_move(t, 1, 6, BLACK);
  assert(t->grid[1][6].type == BLACK); assert(t->grid[2][5].type == BLACK);
  make_move(t, 2, 4, WHITE);
  assert(t->grid[2][4].type == WHITE); assert(t->grid[3][4].type == WHITE);
  assert(t->grid[4][4].type == WHITE); assert(t->grid[5][4].type == WHITE);
  assert(t->grid[3][3].type == BLACK); assert(t->grid[2][5].type == BLACK);
  make_move(t, 6, 6, BLACK);
  assert(t->grid[6][6].type == BLACK); assert(t->grid[5][5].type == BLACK);
  assert(t->grid[4][4].type == BLACK); assert(t->grid[6][5].type == EMPTY);
  assert(t->grid[5][6].type == EMPTY); assert(t->grid[7][7].type == EMPTY);
  assert(t->grid[3][3].type == BLACK); assert(t->grid[1][1].type == EMPTY);
  free_table(t);
}

//tests transform_x and transform_y
void test_transformxy()
{
  table *t = create_table(8);
  assert(transform_x(t,331) == -1);
  assert(transform_x(t,332) == 0);
  assert(transform_x(t,948) == 7);
  assert(transform_x(t,949) == -1);
  assert(transform_x(t,409) == 0);
  assert(transform_x(t,410) == 1);
  assert(transform_x(t,717) == 4);
  assert(transform_x(t,718) == 5);
  assert(transform_y(t,51) == -1);
  assert(transform_y(t,52) == 0);
  assert(transform_y(t,668) == 7);
  assert(transform_y(t,669) == -1);
  assert(transform_y(t,129) == 0);
  assert(transform_y(t,130) == 1);
  assert(transform_y(t,530) == 6);
  assert(transform_y(t,600) == 7);
  free_table(t);
}

//tests finished_game
void test_finished()
{
  table *t = create_table(4);
  t->grid[1][1].type = BLACK; t->grid[1][2].type = BLACK;
  t->grid[2][1].type = BLACK; t->grid[2][2].type = BLACK;
  t->grid[3][1].type = WHITE;
  assert(finished_game(t, 6) == 0);
  t->grid[0][1].type = BLACK; t->grid[1][3].type = BLACK;
  assert(finished_game(t, 6) == 1);
  t->grid[0][0].type = WHITE; t->grid[0][2].type = BLACK;
  t->grid[0][3].type = WHITE; t->grid[1][0].type = WHITE;
  t->grid[2][0].type = WHITE; t->grid[3][0].type = BLACK;
  t->grid[3][2].type = BLACK; t->grid[3][3].type = WHITE;
  t->grid[2][3].type = WHITE;
  assert(finished_game(t, 6) == 1);
  free_table(t);
  t = create_table(4);
  t->grid[1][1].type = WHITE; t->grid[1][2].type = WHITE;
  t->grid[2][1].type = BLACK; t->grid[2][2].type = BLACK;
  assert(finished_game(t, 6) == 0);
  t->grid[3][1].type = WHITE; t->grid[1][3].type = WHITE;
  t->grid[0][2].type = BLACK; t->grid[2][3].type = BLACK;
  assert(finished_game(t, 6) == 0);
  t->grid[3][0].type = BLACK; t->grid[3][2].type = BLACK;
  t->grid[3][3].type = BLACK;
  assert(finished_game(t, 6) == 0);
  free_table(t);
}

//does tests
void test()
{
  test_first_moves();
  test_is_line();
  test_later_moves();
  test_make_move();
  test_transformxy();
  test_finished();
  printf("All tests passed!\n");
}

//main function
int main(int n, char *args[n])
{
  if(n == 1) run(8, 1280, 720);
  else if(n == 2){
      if(strcmp(args[1],"test") == 0) test();
      else{
        int n = atoi(args[1]);
        if(n % 2 == 0 && n >= 4) run(n, 1280, 720);
        else{  printf("Use ./reversi test or ./reversi n, where n is ");
               printf("an even number greater or equal to 4\n"); exit(1);}
      }
    }
  else if(n == 3){
    int w = atoi(args[1]), h = atoi(args[2]);
    if( w > 0 && h > 0 && w > h) run(8, w ,h);
    else{ printf("Use ./reversi w h where w is the width of the screen and ");
          printf("h is the height\n"); exit(1);}
  }
  else if(n == 4){
    int n = atoi(args[1]), w = atoi(args[2]), h = atoi(args[3]);
    if(n % 2 == 0 && n >= 4 && w > 0 && h > 0 && w > h) run(n, w ,h);
    else{ printf("Use ./reversi n w h, where n is an even number ");
          printf("greater or equal to 4, w is the width of the screen ");
          printf("and h is the heigh\n"); exit(1);}
  }
  else run(8, 1280, 720);
  return 0;
}
