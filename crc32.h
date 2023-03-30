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

#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

uint32_t crc32 (const void *buf, size_t size);

#endif
