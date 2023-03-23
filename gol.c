#include "SDL.h"

#include "crc32.h"

static const char const title[] = "Game of Life";

static SDL_Window *window;
static SDL_Renderer *renderer;

static int myargc;
static char **myargv;

#define SIZE 8
#define SIZEUNIT (1 << SIZE)
#define SIZEMASK (SIZEUNIT - 1)

enum
{
  DEAD,
  ALIVE,
  SURVIVE
};

static unsigned char grid[SIZEUNIT][SIZEUNIT];

static void init_grid(void)
{
  int cx = SIZEUNIT / 2, cy = SIZEUNIT / 2;

  memset(grid, 0, sizeof(grid));

// glider
/*
  grid[cx]  [cy]   = ALIVE;
  grid[cx+1][cy+1] = ALIVE;
  grid[cx-1][cy+2] = ALIVE;
  grid[cx]  [cy+2] = ALIVE;
  grid[cx+1][cy+2] = ALIVE;
*/

// f-pentomino
/*
  grid[cx-1][cy]   = ALIVE;
  grid[cx]  [cy-1] = ALIVE;
  grid[cx]  [cy]   = ALIVE;
  grid[cx]  [cy+1] = ALIVE;
  grid[cx+1][cy-1] = ALIVE;
*/

// double-u
/*
  grid[cx-1][cy-3] = ALIVE;
  grid[cx]  [cy-3] = ALIVE;
  grid[cx+1][cy-3] = ALIVE;
  grid[cx-1][cy-2] = ALIVE;
  grid[cx+1][cy-2] = ALIVE;
  grid[cx-1][cy-1] = ALIVE;
  grid[cx+1][cy-1] = ALIVE;
  grid[cx-1][cy+3] = ALIVE;
  grid[cx]  [cy+3] = ALIVE;
  grid[cx+1][cy+3] = ALIVE;
  grid[cx-1][cy+2] = ALIVE;
  grid[cx+1][cy+2] = ALIVE;
  grid[cx-1][cy+1] = ALIVE;
  grid[cx+1][cy+1] = ALIVE;
*/

// 42
/*
  cx -= 3;
  grid[cx-1][cy-2] = ALIVE;
  grid[cx-1][cy-1] = ALIVE;
  grid[cx-1][cy]   = ALIVE;
  grid[cx]  [cy]   = ALIVE;
  grid[cx+1][cy-2] = ALIVE;
  grid[cx+1][cy-1] = ALIVE;
  grid[cx+1][cy]   = ALIVE;
  grid[cx+1][cy+1] = ALIVE;
  grid[cx+1][cy+2] = ALIVE;
  cx += 4;
  grid[cx-1][cy+2] = ALIVE;
  grid[cx]  [cy+2] = ALIVE;
  grid[cx+1][cy+2] = ALIVE;
  grid[cx+1][cy-1] = ALIVE;
  grid[cx-1][cy]   = ALIVE;
  grid[cx]  [cy]   = ALIVE;
  grid[cx+1][cy]   = ALIVE;
  grid[cx-1][cy+1] = ALIVE;
  grid[cx-1][cy-2] = ALIVE;
  grid[cx]  [cy-2] = ALIVE;
  grid[cx+1][cy-2] = ALIVE;
*/

// acorn
  grid[cx-3][cy+1] = ALIVE;
  grid[cx-2][cy-1] = ALIVE;
  grid[cx-2][cy+1] = ALIVE;
  grid[cx]  [cy]   = ALIVE;
  grid[cx+1][cy+1] = ALIVE;
  grid[cx+2][cy+1] = ALIVE;
  grid[cx+3][cy+1] = ALIVE;
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
  {0x55, 0x57, 0x53}, // Aluminium
  {0xcc, 0x00, 0x00}, // Scarlet Red
  {0x75, 0x50, 0x7b}, // Plum
  {0x34, 0x65, 0xa4}, // Sky Blue
  {0x73, 0xd2, 0x16}, // Chameleon
  {0xc1, 0x7d, 0x11}, // Chocolate
  {0xf5, 0x79, 0x00}, // Orange
  {0xed, 0xd4, 0x00}, // Butter
};

static void draw_grid (void)
{
  int x, y;

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
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

static uint32_t crc[8];

static int sequence_detected (int step)
{
  int i;

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
      return 1;
    }
  }

  return 0;
}

int main (int argc, char **argv)
{
  int quit = 0;
  int step = 0, alive;
  char msg[64];
  int flags = SDL_WINDOW_RESIZABLE;

  myargc = argc;
  myargv = argv;

  if (command_line_parameter("-fullscreen"))
  {
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  }

  SDL_Event event;

  SDL_Init(SDL_INIT_VIDEO);

  SDL_CreateWindowAndRenderer(SIZEUNIT, SIZEUNIT, flags, &window, &renderer);
  SDL_RenderSetLogicalSize(renderer, SIZEUNIT, SIZEUNIT);
  SDL_SetWindowTitle(window, title);

  init_grid();

  while (!quit)
  {
    int x, y;

    crc[step & 7] = crc32(grid, SIZEUNIT * SIZEUNIT);

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

    if (alive == 0 || sequence_detected(step & 7))
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
