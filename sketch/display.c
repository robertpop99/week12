/* A small graphics module for line drawing, based on SDL.
SDL_MAIN_HANDLED is defined before including SDL.h, and a setbuf call is
included in newDisplay, in case this is used as the starting point for a
program in a difficult environmaent. */
#include "display.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Set safe mode to true for environments with poor OpenGL support.
// Set it to false otherwise for greater efficiency.
enum { safe = true };

struct display {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

// If SDL fails, print the SDL error message, and stop the program.
static void fail() {
    fprintf(stderr, "Error: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
}

// Check return values from SDL functions (Integer and Pointer versions).  These
// make error handling less intrusive.
static int I(int n) { if (n < 0) fail(); return n; }
static void *P(void *p) { if (p == NULL) fail(); return p; }

// Create a renderer - in software for safety or hardware for efficiency.
static void createRenderer(display *d) {
    if (safe) {
        SDL_Surface *surface = P(SDL_GetWindowSurface(d->window));
        d->renderer = P(SDL_CreateSoftwareRenderer(surface));
    }
    else d->renderer = P(SDL_CreateRenderer(d->window, -1, 0));
}

// Make recent changes appear on screen.
static void show(display *d) {
    if (safe) I(SDL_UpdateWindowSurface(d->window));
    else SDL_RenderPresent(d->renderer);
}

// Create a new display object.
display *newDisplay(char *title, int width, int height) {
    setbuf(stdout, NULL);
    display *d = malloc(sizeof(display));
    I(SDL_Init(SDL_INIT_VIDEO));
    int x = SDL_WINDOWPOS_UNDEFINED, y = SDL_WINDOWPOS_UNDEFINED;
    d->window = P(SDL_CreateWindow(title, x, y, width, height, 0));
    createRenderer(d);
    I(SDL_SetRenderDrawColor(d->renderer, 255, 255, 255, 255));
    I(SDL_RenderClear(d->renderer));
    show(d);
    I(SDL_SetRenderDrawColor(d->renderer, 0, 0, 0, 255));
    return d;
}

void line(display *d, int x0, int y0, int x1, int y1) {
    I(SDL_RenderDrawLine(d->renderer, x0, y0, x1, y1));
    show(d);
}

void colour(display *d, int rgba) {
    Uint8 r = (rgba >> 24) & 0xFF;
    Uint8 g = (rgba >> 16) & 0xFF;
    Uint8 b = (rgba >> 8) & 0xFF;
    Uint8 a = rgba & 0xFF;
    I(SDL_SetRenderDrawColor(d->renderer, r, g, b, a));
}

void clear(display *d) {
    Uint8 r, g, b, a;
    I(SDL_GetRenderDrawColor(d->renderer, &r, &g, &b, &a));
    I(SDL_SetRenderDrawColor(d->renderer, 255, 255, 255, 255));
    I(SDL_RenderClear(d->renderer));
    show(d);
    I(SDL_SetRenderDrawColor(d->renderer, r, g, b, a));
}

void pause(display *d, int ms) {
    if (ms > 0) SDL_Delay(ms);
}

char key(display *d) {
    SDL_Event event_structure;
    SDL_Event *event = &event_structure;
    while (true) {
        int r = SDL_WaitEvent(event);
        if (r == 0) fail();
        if (event->type == SDL_QUIT) {
            SDL_Quit();
            exit(0);
        }
        else if (event->type == SDL_KEYUP) {
            int sym = event->key.keysym.sym;
            return (char)sym;
        }
    }
}

void end(display *d) {
    SDL_Delay(5000);
    SDL_Quit();
}
