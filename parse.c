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
#include <string.h>

#include "gol.h"

static int x, y;
static char line[1024], *p = NULL;

// RLE parser
static int parse_rle (FILE *file)
{
  char rule[16];
  int x0 = cx, y0 = cy;
  int count = 0, mult = 1, i;

  while (fgets(line, sizeof(line), file) != NULL)
  {
    if (line[0] == '#')
      continue;

    if (sscanf(line, "x = %d, y = %d, rule = %s", &x, &y, rule) == 3)
    {
      x0 = cx - x / 2;
      y0 = cy - y / 2;

      if (strcasecmp(rule, "b3/s23") != 0)
        return 1;

      continue;
    }

    if (p == NULL)
    {
      x = x0;
      y = y0;
    }

    for (p = line; *p; p++)
    {
      if (*p >= '0' && *p <= '9')
      {
        count *= mult;
        count += *p - '0';
        mult *= 10;
      }
      if (*p == 'b' || *p == 'o')
      {
        if (count == 0)
          count = 1;

        for (i = 0; i < count; i++, x++)
        {
          if (*p == 'o')
          {
            grid[x][y] = ALIVE;
            printf("O");
          }
          else
            printf(" ");
        }

        count = 0;
        mult = 1;
      }
      if (*p == '$')
      {
        if (count == 0)
          count = 1;

        printf(" %d \n", y);

        x = x0;
        y += count;

        count = 0;
        mult = 1;
      }
      if (*p == '!')
      {
        printf(" %d \n", y);
        return 0;
      }
    }
  }

  return 1;
}

// Plaintext parser
static int parse_plaintext (FILE *file)
{
  y = cy;

  while (fgets(line, sizeof(line), file) != NULL)
  {
    if (line[0] == '!')
      continue;

    x = cx;

    for (p = line; *p; p++)
    {
      if (*p == '.')
        x++;
      else if (*p == 'O')
        grid[x++][y] = ALIVE;
    }

    y++;
  }

  return 0;
}

// Life 1.06 parser
static int parse_life106 (FILE *file)
{
  while (fgets(line, sizeof(line), file) != NULL)
  {
    if (line[0] == '#')
      continue;

    if (sscanf(line, "%d %d", &x, &y) == 2)
      grid[cx+x][cy+y] = ALIVE;
  }

  return 0;
}

int parse_file (char *filename)
{
  FILE *file;
  char *ext;
  int ret;

  if (filename == NULL)
    return 1;

  file = fopen(filename, "r");
  if (file == NULL)
    return 1;

  ext = strrchr(filename, '.');
  if (ext != NULL)
    ext++;
  else
    ext = "";

  if (strcmp(ext, "rle") == 0)
  {
    ret = parse_rle(file);
  }
  else if (strcmp(ext, "cells") == 0)
  {
    ret = parse_plaintext(file);
  }
  else
  {
    ret = parse_life106(file);
  }

  fclose(file);

  return ret;
}

