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

#ifndef GOL_H
#define GOL_H

#define SIZE 8
#define SIZEUNIT (1 << SIZE)
#define SIZEMASK (SIZEUNIT - 1)
static const int cx = SIZEUNIT / 2, cy = SIZEUNIT / 2;

enum
{
  DEAD,
  ALIVE,
  SURVIVE
};

extern unsigned char grid[SIZEUNIT][SIZEUNIT];

#endif
