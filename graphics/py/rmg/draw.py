#
#       © Christian Sommerfeldt Øien
#       All rights reserved
from rmg.plane import XY
from rmg.math_ import Rx, Ry, Rz, matrix_multiply, unit_angle
from rmg.space import Point, Direction, Box, origo
from rmg.color import Color
from sys import stderr
from math import pi


def notice(message):
    stderr.write(repr(message) + "\n")


class Stroke:
    def __init__(self, to, color):
        self.to = to.copy()
        self.color = color


class StrokesPath:
    def __init__(self, at, strokes = None):
        assert isinstance(at, Point)
        self.at = at.copy()
        self.strokes = strokes or []

    def stroke(self, to, color):
        self.strokes.append(Stroke(to, color))

    def draw(self, pencil):
        pencil.draw(self.at, None)
        for s in self.strokes:
            pencil.draw(s.to, s.color)

    def copy(self):
        strokes = [Stroke(s.to, s.color) for s in self.strokes]
        return StrokesPath(self.at, strokes)


class Drawing:
    def __init__(self, paths = None, adjust_factor = None):
        self.paths = paths or []
        self.frame_adapt = adjust_factor

    def all_points(self):
        points = []
        for p in self.paths:
            points.append(p.at)
            for s in p.strokes:
                points.append(s.to)
        return points

    def transformation(self):
        box = Box()
        for p in self.all_points():
            box.update(p)
        m = max(*box.dims())
        if not m: m = 1
        adjust = Direction(*box.mins()) * -1
        factor = 1.0 / m
        if self.frame_adapt:
            factor *= self.frame_adapt[1]
            adjust += self.frame_adapt[0] * (1 / factor)
        return (adjust, factor)

    def rescale(self):
        a, m = self.transformation()
        for p in self.all_points():
            p.__init__(*((p + a) * m).xyz())

    def render(self, pencil):
        for p in self.paths:
            p.draw(pencil)

    def copy(self):
        paths = [p.copy() for p in self.paths]
        return Drawing(paths, self.frame_adapt)


class Turtle:
    def __init__(self, position, color, scale, hook, value):
        self.path = StrokesPath(position)
        self.position = position
        self.nose = Direction(1, 0, 0)
        self.foot = Direction(0, 0, -1)
        self.scale = scale
        self.color = color
        self.hook = hook
        self.value = value

    def turn(self, u):
        self.nose = self.nose.rotation_on_axis(self.foot, u)
    def roll(self, u):
        self.foot = self.foot.rotation_on_axis(self.nose, u)
    def pitch(self, u):
        wing = self.nose.cross(self.foot)
        self.nose = self.nose.rotation_on_axis(wing, u)
        self.foot = self.foot.rotation_on_axis(wing, u)

    def forward(self, distance):
        if self.hook: self.hook(self)
        self.position += self.nose * (distance * self.scale)
        if not self.color: notice("Turtle.forward with no color")
        self.path.stroke(self.position, self.color)

    def forward_invisible(self, distance):
        self.color, color = None, self.color
        self.forward(distance)
        self.color = color

    def set_color(self, color): self.color = color
    def multiply_scale(self, m): self.scale *= m
    def adjust_value(self, i): self.value += i


class TreeBuilder:
    def __init__(self, drawing, u_scale = 1, u_default = 0.25,
            color = None, hook = None):
        self.drawing = drawing
        self.start_branch(Turtle(origo, color, 1, hook, 0))
        self.stack = []
        self.angle_scale = unit_angle(u_scale)
        self.default_turn = u_default

    def start_branch(self, turtle = None):
        if turtle is None:
            t = self.turtle
            turtle = Turtle(t.position, t.color, t.scale, t.hook, t.value)
            turtle.nose = t.nose
            turtle.foot = t.foot
        self.drawing.paths.append(turtle.path)
        self.turtle = turtle

    def push(self):
        self.stack.append(self.turtle)
        self.start_branch()

    def pop(self):
        self.turtle = self.stack[-1]
        del self.stack[-1]

    def forward(self, step = 1): self.turtle.forward(step)
    def forward_invisible(self, step = 1): self.turtle.forward_invisible(step)

    def dflt(self, u): return self.default_turn if u is None else u
    def pitch_down(self, u = None):  self.turtle.pitch( self.dflt(u) * self.angle_scale)
    def pitch_up(self, u = None):    self.turtle.pitch(-self.dflt(u) * self.angle_scale)
    def roll_clock(self, u = None):  self.turtle.roll(  self.dflt(u) * self.angle_scale)
    def roll_counter(self, u = None):self.turtle.roll( -self.dflt(u) * self.angle_scale)
    def turn_left(self, u = None):   self.turtle.turn(  self.dflt(u) * self.angle_scale)
    def turn_right(self, u = None):  self.turtle.turn( -self.dflt(u) * self.angle_scale)

    def multiply_scale(self, m): self.turtle.multiply_scale(m)
    def adjust_value(self, i): self.turtle.adjust_value(i)




if __name__ == "__main__":
    from rmg.board import Board, Pencil
    from rmg.color import white
    from rmg.math_ import rnd
    from random import seed
    from sys import stderr
    seed(1234)
    turn = 0
    drawing = Drawing()
    t = TreeBuilder(drawing, color = white)
    #note: dont use Turtle directly; just to exercise more stuff

    def the_move(turn):
        t.forward()
        t.turn_left(turn)
        turn += rnd(-0.002, 0.002)
        if abs(turn) > 0.01: turn *= 0.5
        return turn

    n_frames = 2048
    each_n_move = 8
    n = n_frames * each_n_move
    stderr.write("{0...%d}.jpeg:\n" % (n-1,))
    for i in range(n):
        turn = the_move(turn)
        stderr.write("{%d} created\r" % (i+1,))
        if (i+1) % each_n_move == 0:
            board = Board.mono(512, 256, Color(0, 0, 0.2))
            d = drawing.copy()
            d.rescale()
            d.render(Pencil(board))
            board.save("%d.jpeg" % (i/each_n_move,))

