#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from math import pi
from rmg.math_ import degrees_unit, rnd
from rmg.board import Board, Pencil
from rmg.draw import Drawing, TreeBuilder
from rmg.ls import Settings, System, OperationsVisitor
from rmg.color import Optics, Color, white, black
from rmg.scene import World, WorldPencil, SceneObject, LightSpot, RgbSky
from rmg.bodies import Sphere, Cylinder, Plane, Intersection
from rmg.space import Point, Direction
from optparse import OptionParser
from subprocess import Popen, PIPE
from sys import stdout, stdin, stderr, exit
from os import environ


def hue_palette(hue_count, s = 1, v = 1):
    if hue_count == 0: return [Color(.8, .8, .8)]
    palette = []
    scale = 2 * pi / hue_count
    for i in range(hue_count):
        h = i * scale
        palette.append(Color.from_hsv(h, s, v))
    return palette


class Hook:
    def __init__(self, count, offset = 0):
        self.palette = hue_palette(count)
        self.offset = offset
        self.counter = 0

    def __call__(self, turtle):
        hue = (turtle.value + self.offset) % len(self.palette)
        turtle.set_color(self.palette[hue])
        turtle.value += 1
        stderr.write(str(self.counter) + "\r")
        self.counter += 1


def operating_visitor(t):
    return OperationsVisitor({
        "[": t.push,
        "]": t.pop,
        "+": t.turn_left,
        "-": t.turn_right,
        "&": t.pitch_down,
        "^": t.pitch_up,
        "/": t.roll_clock,
        "\\": t.roll_counter,
        "@": t.multiply_scale,
        "#": t.adjust_value,
        "~": t.forward,
        "?": t.forward_invisible},
        t.forward,
        t.forward_invisible)


def examples():
    dragon = System(r"f x()x()x+y f  ()y()f x-y")
    dragon.derive(2)
    assert str(dragon.axiom) == "f x + y f + f x - y f"
    triangle = System(r"a  ()a(){:'1'b-a-b}{:'0'.}  ()b()a+b+a")  #stochastic
    triangle.derive(2)
    assert str(triangle.axiom) == "a + b + a - b - a - b - a + b + a"
    snowflake = System("f--f--f ()f() f+f--f+f")
    snowflake.derive()
    fass = System(r"L ()L()L+R++R-L--L L-R+()R()-L+R R++R+L--L -R")
    fass.derive()
    plant = System(r"f ()f() f f-[-f+f+f]+[+f-f-f]")
    plant.derive(2)
    assert str(plant.axiom) == "f f - [- f + f + f] + [+ f - f - f] f f - " \
            "[- f + f + f] + [+ f - f - f] - [- f f - [- f + f + f] + [+ f" \
            "- f - f] + f f - [- f + f + f] + [+ f - f - f] + f f - [- f +" \
            "f + f] + [+ f - f - f]] + [+ f f - [- f + f + f] + [+ f - f -" \
            "f] - f f - [- f + f + f] + [+ f - f - f] - f f - [- f + f + f" \
            "] + [+ f - f - f]]"
    hesper = System(r"""~0~1~1
             (0)0(0) 1  (0)1(0) 0  (0)1(1) 1~1  (1)0(1) 1[+~1~1]  (1)1(1) 0
            (0[)0(0) 1 (0[)1(0) 0 (0[)1(1) 1~1 (1[)0(1) 1[+~1~1] (1[)1(1) 0
            ()+()-()-()+""")
    hesper.derive(11)
    assert str(hesper.axiom) == "~ 0 ~ 1 ~ 1 ~ 1 ~ 1 [+ ~ 1 ~ 1] ~ 1 ~ 1 ~" \
            "0 [- ~ 0 ~ 1] ~ 0 ~ 1 ~ 0 [+ ~ 1 ~ 1 [- ~ 0 ~ 1] ~ 1] ~ 1 ~ 1" \
            "~ 1 [- ~ 1 ~ 0 [- ~ 1 ~ 1 [- ~ 0 ~ 1] ~ 1] ~ 1] ~ 0 [+ ~ 1 ~ " \
            "1 [- ~ 0 ~ 1] ~ 1] ~ 1 ~ 1 ~ 0 [- ~ 0 ~ 1] [- ~ 0 ~ 1 ~ 1 [+ " \
            "~ 1 ~ 1] [- ~ 1 ~ 1 [- ~ 0 ~ 1] ~ 1] ~ 1] [+ ~ 1 [+ ~ 1 ~ 1] " \
            "[+ ~ 0 ~ 1] ~ 1] ~ 1"
    pplant = System(r"x:'1'"
            "()x:'j'()f[@:'.6'+:'j'x:'j']f[@:'.4'-:'j'x:'j']@:'.9'x:'j*-1'")
    pplant.derive(2)
    Settings.param_globals({"P": 0.3, "Q": 0.7})
    parametric = System(r"""
    f:'1,0'
    ()f:'x,t'()  :'t==0' f:'x*P,2'
                               + f:'x*(P*Q)**0.5,1'
                             - - f:'x*(P*Q)**0.5,1'
                               + f:'x*Q,0'
    ()f:'x,t'()  :'t >0' f:'x,t-1'
    """, symbols_used = "f+-")
    parametric.derive(2)

opts = OptionParser()
opts.add_option("-a", "--auto-test", action="store_true", default=False)
opts.add_option("-b", "--palette-breadth", type="int", default=32)
opts.add_option("-c", "--derivations-count", type="int", default=6)
opts.add_option("-d", "--default-turn", type="float", default=90)
opts.add_option("-e", "--l-system-expression", type="string", default="")
opts.add_option("-g", "--eval-globals", type="string", default="")
opts.add_option("-i", "--palette-index", type="int", default=0)
opts.add_option("-n", "--only-last-nodes", type="int")
opts.add_option("-o", "--image-path", type="string")
opts.add_option("-p", "--print-axiom", action="store_true", default=False)
opts.add_option("-r", "--resolution", type="string", default="512x512")
opts.add_option("-s", "--random-seed", type="int")
opts.add_option("-t", "--ray-trace-mode", action="store_true", default=False)
opts.add_option("-u", "--unit-turn-degrees", type="float", default=360)
opts.add_option("-C", "--trace-command", type="string", default="gun")
opts.add_option("-H", "--expression-help", action="store_true", default=False)
(options, args) = opts.parse_args()
if options.expression_help:
    stderr.write("See examples() in this file\n")
    exit()
if not options.l_system_expression:
    stderr.write("Please specify expression with -e.  For help use -h\n")
    exit()
if len(args):
    stderr.write("Positional arguments triggers self-test mode\n")
    examples()
    stderr.write("Done\n")
    exit()

param_globals = {"rnd": rnd}
exec(options.eval_globals, {}, param_globals)
Settings.param_globals(param_globals)

if options.only_last_nodes: Settings.only_last_nodes(options.only_last_nodes)
if options.random_seed is not None: Settings.random_seed(options.random_seed)

ls = System(options.l_system_expression or stdin.read())

ls.derive(options.derivations_count, lambda n: stderr.write(str(n) + "\r"))
stderr.write("\r")
if options.print_axiom: stderr.write("%s\n" % (ls.axiom))
if not options.image_path: exit()

stderr.write("Drawing resulting axiom\n")
blockf = (Direction(-0.5, -0.5, -0.5), 1) if options.ray_trace_mode else None
drawing = Drawing(adjust_factor = blockf)
tree_builder = TreeBuilder(drawing,
        u_scale = degrees_unit(options.unit_turn_degrees),
        u_default = degrees_unit(options.default_turn),
        hook = Hook(options.palette_breadth, options.palette_index))
ls.axiom.visit_by(operating_visitor(tree_builder))
stderr.write("")

if not options.ray_trace_mode:
    stderr.write("Rendering drawing at %s\n" % (options.resolution,))
    width, height = [int(s) for s in options.resolution.split("x")]
    board = Board.mono(width, height, white)
    drawing.rescale()
    drawing.render(Pencil(board))
    stderr.write("Writing board to %s\n" % (options.image_path,))
    board.save(options.image_path, gray = (options.palette_breadth == 0))
else:
    world = World([], [
        LightSpot(Point(-8,-8, 2), Color(.8, .2, .2)),
        LightSpot(Point( 8,-8, 2), Color(.8, .8, .2)),
        LightSpot(Point(-8, 8, 2), Color(.2, .8, .2)),
        LightSpot(Point( 8, 8, 2), Color(.2, .2, .8)),
    ], sky = RgbSky())
    def factory(p, q, c):
        if p == q: return []
        r = abs(q - p) * .65
        color = c.mix(white, .4)
        reflection = color * .2
        absorption = color * .6
        optics = Optics(reflection, absorption)
        ball = Sphere(q, r)
        so_ball = SceneObject(optics, ball)
        return [so_ball]
    pencil = WorldPencil(world, factory)
    drawing.rescale()
    drawing.render(pencil)

    comm = "%s %s %s" % (options.trace_command,
            options.resolution, options.image_path)
    stderr.write("Piping world to '%s'\n" % (comm,))
    environ["GUN_RS"] = "Yes"
    p = Popen(comm, stdin=PIPE, shell=True, close_fds=True)
    p.stdin.write(str(world))
    p.stdin.close()
    exit(p.wait())
