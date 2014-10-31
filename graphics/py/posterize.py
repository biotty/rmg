#!/usr/bin/env python
# -*- coding: utf-8 -*-

from rmg.color import Color, InkColor, white_ink
from rmg.board import Photo, Image, Board
from rmg.plane import XY
from math import sqrt, floor
from copy import copy
from sys import exit, stderr
from optparse import OptionParser


def most_central_available(board):
    c = XY(board.n_columns, board.n_rows) * 0.5
    min_distance = abs(c) + 1
    the_most = None
    for y in range(board.n_rows):
        for x in range(board.n_columns):
            if board.int_at(x, y) == 0:
                p = XY(x, y)
                d = abs(p - c)
                if d <= min_distance:
                    min_distance = d
                    the_most = p
    return the_most
 

def status(m):
    stderr.write(m)


def fill_order(s):
    status("calculating %dx%d pattern\n" % (s, s))
    board = Board([[0]*s for i in range(s)])
    for i in range(s * s):
        p = most_central_available(board)
        board.matrix[p.y][p.x] = 1 + i
    #todo: cache this on /tmp/posterize.X_Y.cache
    #      so that we don't recalculate at each invokation
    return board


def averages(p, s, cmy_index):
    q = p.quality
    matrix = []
    for row in range(0, q.height, s):
        status("%d/%d\r" % (row, q.height))
        row_bound = min(q.height, row + s)
        a = []
        for column in range(0, q.width, s):
            column_bound = min(q.width, column + s)
            color = Color(0, 0, 0)
            for y in range(row, row_bound):
                for x in range(column, column_bound):
                    color += p.color_at(x, y)
            color *= 1/float((column_bound - column) * (row_bound - row))
            a.append(int(s * s * \
                    InkColor.from_hsv(*color.hsv()).cmy()[cmy_index]))
        matrix.append(a)
    return Board(matrix)


class Poster:
    def __init__(self, p, s, cmy_index):
        status("averaging for %s\n" % (["cyan", "magenta", "yellow"][cmy_index],))
        self.board = averages(p, s, cmy_index)
        status("%dx%d board of fill-counts\n" % \
            (self.board.n_columns, self.board.n_rows))
        self.pattern = fill_order(s)
        self.s = s
        self.cmy_index = cmy_index

    def ink(self, x, y):
        a = [0, 0, 0]
        a[self.cmy_index] = 1
        component_ink = InkColor(*a)
        n = self.board.int_at(x / self.s, y / self.s)
        s_x = x % self.s
        s_y = y % self.s
        if self.pattern.int_at(s_x, s_y) <= n:
            return component_ink
        else:
            return white_ink


def ink_post(x, y):
    ink = white_ink
    ink += cyan_poster.ink(x, y)
    ink += magenta_poster.ink(x, y)
    ink += yellow_poster.ink(x, y)
    return Color.from_hsv(*ink.hsv())


options = OptionParser()
options.add_option("-s", "--sides", default="17,19,23")
(opts, args) = options.parse_args()


if len(args) == 0:
    status("give path of photo\n")
    exit(1)

p = Photo.from_file(args[0])
q = p.quality
r = Image(q.width, q.height, None if len(args) < 2 else args[1])
s = opts.sides.split(",")

cyan_poster = Poster(p, int(s[0]), 0)
magenta_poster = Poster(p, int(s[1]), 1)
yellow_poster = Poster(p, int(s[2]), 2)

for row in range(q.height):
    status("rendering row %d\r" % (row,))
    for column in range(q.width):
        r.put(ink_post(column, row))
r.close()
status("rendered %dx%d image\n" % (q.width, q.height))

