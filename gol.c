/*
 * This file is part of Game of Life (https://github.com/fabiangreffrath/gol).
 * Copyright © 2023, Fabian Greffrath.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include "SDL.h"

#include "crc32.h"
#include "gol.h"
#include "parse.h"

static const char title[] = "Game of Life";

static SDL_Window *window;
static SDL_Renderer *renderer;

static int myargc;
static char **myargv;

unsigned char grid[SIZEUNIT][SIZEUNIT];

static int init_grid(char *filename)
{
  memset(grid, 0, sizeof(grid));

  if (filename == NULL)
  {
    /* acorn */
    grid[cx-3][cy+1] = ALIVE;
    grid[cx-2][cy-1] = ALIVE;
    grid[cx-2][cy+1] = ALIVE;
    grid[cx]  [cy]   = ALIVE;
    grid[cx+1][cy+1] = ALIVE;
    grid[cx+2][cy+1] = ALIVE;
    grid[cx+3][cy+1] = ALIVE;

    return 0;
  }
  else
    return parse_file(filename);
}

static int number_of_neighbours_border(int x, int y)
{
  int n = 0;

  if (x > 0 && y > 0)
    if (grid[x-1][y-1] & ALIVE)
      n++;
  if (x > 0)
    if (grid[x-1][y  ] & ALIVE)
      n++;
  if (x > 0 && y < SIZEMASK)
    if (grid[x-1][y+1] & ALIVE)
      n++;
  if (y > 0)
    if (grid[x  ][y-1] & ALIVE)
      n++;
  if (y < SIZEMASK)
    if (grid[x  ][y+1] & ALIVE)
      n++;
  if (x < SIZEMASK && y > 0)
    if (grid[x+1][y-1] & ALIVE)
      n++;
  if (x < SIZEMASK)
    if (grid[x+1][y  ] & ALIVE)
      n++;
  if (x < SIZEMASK && y < SIZEMASK)
    if (grid[x+1][y+1] & ALIVE)
      n++;

  return n;
}

static int number_of_neighbours_torus(int x, int y)
{
  int n = 0;

  if (grid[(x-1) & SIZEMASK][(y-1) & SIZEMASK] & ALIVE)
    n++;
  if (grid[(x-1) & SIZEMASK][(y  ) & SIZEMASK] & ALIVE)
    n++;
  if (grid[(x-1) & SIZEMASK][(y+1) & SIZEMASK] & ALIVE)
    n++;
  if (grid[(x  ) & SIZEMASK][(y-1) & SIZEMASK] & ALIVE)
    n++;
  if (grid[(x  ) & SIZEMASK][(y+1) & SIZEMASK] & ALIVE)
    n++;
  if (grid[(x+1) & SIZEMASK][(y-1) & SIZEMASK] & ALIVE)
    n++;
  if (grid[(x+1) & SIZEMASK][(y  ) & SIZEMASK] & ALIVE)
    n++;
  if (grid[(x+1) & SIZEMASK][(y+1) & SIZEMASK] & ALIVE)
    n++;

  return n;
}

static int (*number_of_neighbours)(int x, int y) = number_of_neighbours_border;

static int survival_condition(int x, int y)
{
  const int n = number_of_neighbours(x, y);

  if (grid[x][y] & ALIVE)
    return (n == 2 || n == 3);
  else
    return (n == 3);
}

enum {R, G, B};

static const Uint8 colors[9][3] = {
  {0x55, 0x57, 0x53}, /* Aluminium */
  {0xcc, 0x00, 0x00}, /* Scarlet Red */
  {0x75, 0x50, 0x7b}, /* Plum */
  {0x34, 0x65, 0xa4}, /* Sky Blue */
  {0x73, 0xd2, 0x16}, /* Chameleon */
  {0xc1, 0x7d, 0x11}, /* Chocolate */
  {0xf5, 0x79, 0x00}, /* Orange */
  {0xed, 0xd4, 0x00}, /* Butter */
};

static void draw_grid (void)
{
  int x, y;

  SDL_SetRenderDrawColor(renderer, 32, 32, 32, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  for (x = 0; x < SIZEUNIT; x++)
  {
    for (y = 0; y < SIZEUNIT; y++)
    {
      if (grid[x][y] == ALIVE)
      {
        const int non = number_of_neighbours(x, y);
        const Uint8 *const color = colors[non];
        SDL_SetRenderDrawColor(renderer, color[R], color[G], color[B], SDL_ALPHA_OPAQUE);
        SDL_RenderDrawPoint(renderer, x, y);
      }
    }
  }

  SDL_RenderPresent(renderer);
}

static uint32_t crc[4];

static int sequence_detected (unsigned int step)
{
  size_t i;

  for (i = 0; i < sizeof(crc)/sizeof(*crc); i++)
  {
    if (i != step && crc[i] == crc[step])
    {
      return 1;
    }
  }

  return 0;
}

int command_line_parameter (const char *parm)
{
  int i;

  for (i = 1; i < myargc; i++)
  {
    if (strcmp(myargv[i], parm) == 0)
    {
      return i;
    }
  }

  return 0;
}

int main (int argc, char **argv)
{
  int quit = 0;
  int step = 0, alive = 0;
  char msg[64];
  int flags = SDL_WINDOW_RESIZABLE;
  char *filename = NULL;
  int p;

  SDL_Event event;

  myargc = argc;
  myargv = argv;

  if (command_line_parameter("-fullscreen"))
  {
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  }

  if (command_line_parameter("-torus"))
  {
    number_of_neighbours = number_of_neighbours_torus;
  }

  if ((p = command_line_parameter("-pattern")))
  {
    if (p + 1 < myargc)
    {
      filename = myargv[p + 1];
    }
  }

  SDL_Init(SDL_INIT_VIDEO);

  SDL_CreateWindowAndRenderer(SIZEUNIT, SIZEUNIT, flags, &window, &renderer);
  SDL_RenderSetLogicalSize(renderer, SIZEUNIT, SIZEUNIT);
  SDL_SetWindowTitle(window, title);

  quit = init_grid(filename);

  while (!quit)
  {
    int x, y;

    crc[step & 3] = crc32(grid, SIZEUNIT * SIZEUNIT);

    draw_grid();

    alive = 0;
    for (x = 0; x < SIZEUNIT; x++)
    {
      for (y = 0; y < SIZEUNIT; y++)
      {
        if (survival_condition(x, y))
        {
          grid[x][y] |= SURVIVE;
          alive++;
        }
      }
    }

    for (x = 0; x < SIZEUNIT; x++)
    {
      for (y = 0; y < SIZEUNIT; y++)
      {
        grid[x][y] >>= 1;
      }
    }

    SDL_Delay(step ? 50 : 2000);

    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_MOUSEBUTTONDOWN ||
          event.type == SDL_KEYDOWN ||
          event.type == SDL_QUIT)
      {
        quit = 1;
      }
    }

    if (alive == 0 || sequence_detected(step & 3))
    {
      quit = 1;
    }

    step++;
  }

  snprintf(msg, sizeof(msg), "%s: %d cells alive after %d steps.", title, alive, step);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, msg, window);
  puts(msg);

  return 0;
}
