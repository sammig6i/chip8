#include "SDL3/SDL.h"
#include "chip8.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include <stdio.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

void initSDL() {
  if (!SDL_Init()) {
    perror("SDL Initialization Failed!!");
  }

  window = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 512, 0);
  if (window == NULL) {
    perror("Window Creation Failed!!");
  }

  renderer = SDL_CreateRenderer(window, -1, 0);
  if (renderer == NULL) {
    perror("SDL Renderer Creation Failed!!");
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XBGR8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
  if (texture == NULL) {
    perror("SDL Texture Creation Failed!!");
  }
}

void deinit() {
  SDL_DestroyWindow(window);
  SDL_Quit();
}

int main(int argc, char *argv[]) {
  initSDL();

  CHIP8_t cpu;
  init(&cpu);

  bool keepOpen = true;
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 16 * 1000000;

  while (keepOpen) {
    cycle(&cpu);

    SDL_Event e;
    while (SDL_PollEvent(&e) > 0) {
      switch (e.type) {
      case SDL_EVENT_QUIT:
        keepOpen = false;
        // TODO key presses
      }
    }

    SDL_RenderClear(renderer);

    // TODO Build texture

    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    if (nanosleep(&ts, NULL) == -1) {
      perror("nanosleep");
      return EXIT_FAILURE;
    }
  }

  deinit();
  exit(EXIT_SUCCESS);
}
