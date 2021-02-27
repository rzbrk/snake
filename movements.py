#!/usr/bin/env python3

from math import floor

def up(old_pos, ncols, nrows):
    if old_pos > (nrows * (ncols - 1) - 1):
        new_pos = old_pos
    else:
        new_pos = 2 * ncols - old_pos - 1 + 2 * floor(old_pos/ncols) * ncols
    return new_pos

def down(old_pos, ncols, nrows):
    if old_pos < nrows:
        new_pos = old_pos
    else:
        new_pos = old_pos - 2 * (old_pos - ncols) - 1 + 2 * (floor(old_pos/ncols) - 1) * ncols
    return new_pos

def left(old_pos, ncols, nrows):
    if ((old_pos + 1) % (2 * ncols) == 0 or old_pos % (2 * ncols) == 0):
        new_pos = old_pos
    else:
        if (floor(old_pos/ncols) % 2 == 0):
            new_pos = old_pos - 1
        else:
            new_pos = old_pos + 1
    return new_pos

def right(old_pos, ncols, nrows):
    if ((old_pos + 1) % (2 * ncols) == 16 or old_pos % (2 * ncols) == 16):
        new_pos = old_pos
    else:
        if (floor(old_pos/ncols) % 2 == 0):
            new_pos = old_pos + 1
        else:
            new_pos = old_pos - 1
    return new_pos

def print_matrix(ncols, nrows):
    for row in range(nrows):
        for col in range(ncols):
            if (row % 2) != 0:
                cell = row * ncols + ncols - col - 1
            else:
                cell = row * ncols + col

            print('{0:3d}'.format(cell), end=' ')

        print("\n")
