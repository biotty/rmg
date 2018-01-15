#!/usr/bin/env python2
import math
import numpy
import pygame
from ctypes import c_void_p

from OpenGL.GL import *
from OpenGL.GL.shaders import *

from rmg.space import Direction, Point, origo
from rmg.math_ import rnd

def make_shader():
    return compileProgram(
    compileShader("""
    #version 300 es
    in vec4 a;
    in vec4 b;  // note: b.w unused (could pass vec3)
    uniform mat4 view;
    uniform mat4 proj;
    uniform float start;
    uniform float step;
    out vec3 color;
    out vec2 edged;
    vec3 hue_to_rgb(float h)
    {
       h = mod((h * 6.0), 6.0);
       float x = 1.0 - abs(mod(h, 2.0) - 1.0);
       if (0.0 <= h && h < 1.0) return vec3(1.0, x, 0.0);
       if (1.0 <= h && h < 2.0) return vec3(x, 1.0, 0.0);
       if (2.0 <= h && h < 3.0) return vec3(0.0, 1.0, x);
       if (3.0 <= h && h < 4.0) return vec3(0.0, x, 1.0);
       if (4.0 <= h && h < 5.0) return vec3(x, 0.0, 1.0);
       /*else */ return vec3(1.0, 0.0, x);
    }
    void main()
    {
        vec4 pa = view * vec4(a.xyz, 1.0);
        vec4 pb = view * vec4(b.xyz, 1.0);
        vec2 dd = pb.xy - pa.xy;
        vec2 dq = normalize(vec2(-dd.y, dd.x));
        if (gl_VertexID == 0 || b.w < 0.0)
            edged = vec2(1.0, 0.0);
        else
            edged = vec2(0.0, float((gl_VertexID % 2) * 2 - 1));
        pa.xy += edged.y * dq * a.w;
        gl_Position = proj * pa;
        float hue = start + step * (2.0 * float((gl_VertexID - 1) / 2));
        color = hue_to_rgb(hue);
    }
    """, GL_VERTEX_SHADER),
    compileShader("""
    #version 300 es
    precision mediump float;
    in vec3 color;
    in vec2 edged;
    out vec4 fragcolor;
    float manhattan(vec2 v)
    {
        return abs(v.x) + abs(v.y);
    }
    void main()
    {
        fragcolor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0 - manhattan(edged));
    }
    """, GL_FRAGMENT_SHADER))

def mvp_scene(h_w):
    h_radians = math.radians(10)
    near, far = .1, 100

    f = 1 / math.tan(h_radians / 2.0)
    proj = numpy.array([
        f * h_w, 0.0, 0.0, 0.0,
        0.0, f, 0.0, 0.0,
        0.0, 0.0, (far + near) / (near - far), -1.0,
        0.0, 0.0, far * near * 2.0 / (near - far), 0.0],
        numpy.float32)

    z = 10

    view = numpy.array([
        [1.0, 0.0, 0.0, 0.0],
        [0.0, 1.0, 0.0, 0.0],
        [0.0, 0.0, 1.0, 0.0],
        [0.0, 0.0, -z, 1.0]],
        numpy.float32)
    return view, proj

class Stroke():
    def __init__(self, shader, v, a, i):
        self.vao = glGenVertexArrays(1)
        self.shader = shader
        self.n = n = len(v) / 4
        self.start = numpy.float32(a)
        self.step = numpy.float32(i) * 1.0 / (self.n - 1)  # investigate: - 1
        avg = origo
        for i in range(0, len(v), 4):
            avg += Point(*v[i:i+3])
        self.avg = avg * (1.0 / n)

        a_attrloc = glGetAttribLocation(shader, "a")
        b_attrloc = glGetAttribLocation(shader, "b")
        glBindVertexArray(self.vao)
        vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, vbo)
        data = numpy.array(v, dtype=numpy.float32)
        glBufferData(GL_ARRAY_BUFFER, 4 * len(data), data, GL_STATIC_DRAW)
        glEnableVertexAttribArray(a_attrloc);
        glEnableVertexAttribArray(b_attrloc);
        glVertexAttribPointer(a_attrloc, 4, GL_FLOAT, False, 0, c_void_p(0))
        glVertexAttribPointer(b_attrloc, 4, GL_FLOAT, False, 0, c_void_p(16))
        glBindVertexArray(0)
        self.view_uloc = glGetUniformLocation(shader, "view")
        self.proj_uloc = glGetUniformLocation(shader, "proj")
        self.start_uloc = glGetUniformLocation(shader, "start")
        self.step_uloc = glGetUniformLocation(shader, "step")

    def draw(self, view, proj):
        glUseProgram(self.shader)
        glUniformMatrix4fv(self.view_uloc, 1, GL_FALSE, view)
        glUniformMatrix4fv(self.proj_uloc, 1, GL_FALSE, proj)
        glUniform1f(self.start_uloc, self.start)
        glUniform1f(self.step_uloc, self.step)
        glBindVertexArray(self.vao)
        glDrawArrays(GL_TRIANGLE_STRIP, 0, self.n - 1)
        glBindVertexArray(0)
        glUseProgram(0)

def rotation_matrix(axis, theta):
    axis = numpy.asarray(axis)
    length = numpy.linalg.norm(axis)
    axis /= length
    a = math.cos(.5 * theta)
    b, c, d = -axis * math.sin(.5 * theta)
    aa, bb, cc, dd = a * a, b * b, c * c, d * d
    bc, ad, ac, ab, bd, cd = b * c, a * d, a * c, a * b, b * d, c * d
    return numpy.array([[aa + bb - cc - dd, 2 * (bc + ad), 2 * (bd - ac), 0],
                     [2 * (bc - ad), aa + cc - bb - dd, 2 * (cd + ab), 0],
                     [2 * (bd + ac), 2 * (cd - ab), aa + dd - bb - cc, 0],
                     [0, 0, 0, 1]], numpy.float32)


class Path:
    def __init__(self, p=None, color=None):
        self.hue_a = None
        self.hue_i = 0
        self.points = [p] if p else []
        if color: self.hue_a = color.hsv()[0]

    def hue_range(self):
        return self.hue_a, self.hue_i * len(self.points)

    def sawtooth(self, r=None):
        # return: flattened xyzr...

        a = self.points
        if len(a) == 1:
            a.append(a[0].copy())
            a[0].x -= r
            a[1].x += r

        n_interp = int(len(a) == 2)
        m = .5

        if n_interp:
            n = 1 + n_interp
            m *= n
            w = []
            for i in range(len(a) - 1):
                o = a[i]
                p = a[i + 1]
                for j in range(n):
                    mp = j / float(n)
                    mo = 1 - mp
                    w.append(o * mo + p * mp)
            w.append(a[-1])
            a = w
        assert len(a) > 2

        breadth = lambda d: r or abs(d) * m

        e = .001
        o, p = a[:2]
        d = p - o
        b = list(o.xyz())
        b.append(0)  # note: dummy
        for i in range(1, len(a) - 1):
            o = a[i - 1]
            p = a[i]
            q = a[i + 1]
            d = ((p - o) + (q - p)) * .5
            b.extend(p.xyz())
            b.append(breadth(d))
            b.extend((p + d * e).xyz())
            b.append(breadth(d))
        p, q = a[-2:]
        d = q - p
        b.extend(q.xyz())
        b.append(breadth(d))
        b.extend((q + d * e).xyz())
        b.append(-e)  # note: last-mark for vertex shader
        return b


def test_path(n):
    m = .01
    xx = rnd(3, 5) * m
    yy = rnd(3, 5) * m
    zz = rnd(3, 5) * m
    path = Path()
    a = path.points
    for i in range(n):
        x = math.sin(i * xx) + math.sin(i * yy * 5) * .2
        y = math.sin(i * yy) + math.sin(i * zz * 5) * .2
        z = math.sin(i * zz) + math.sin(i * xx * 5) * .2
        a.append(Point(x, y, z))
    path.hue_a = 0.00
    path.hue_i = 0.01
    return path


class Display:

    class Pencil:
        def __init__(self, display):
            self.display = display
            self.path = None

        def draw(self, p, color):
            if not color:
                self.path = Path(p)
                self.display.paths.append(self.path)
            else:
                if not self.path.hue_a:
                    self.path.hue_a = color.hsv()[0]
                elif not self.path.hue_i:
                    self.path.hue_i = color.hsv()[0] - self.path.hue_a
                self.path.points.append(p)
        #$class

    def __init__(self, order_independent):
        self.order_independent = order_independent
        self.paths = []

    def pencil(self):
        return Display.Pencil(self)

    def run(self):
        pygame.init()
        width = 1280
        height = 720
        flags = pygame.OPENGL | pygame.DOUBLEBUF
        screen = pygame.display.set_mode((width, height), flags)
        glShadeModel(GL_SMOOTH)
        glClearColor(0, 0, 0, 0)
        glEnable(GL_BLEND);
        if self.order_independent: glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA)
        else: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)
        shader = make_shader()
        strokes = []
        for path in self.paths:
            strokes.append(Stroke(shader, path.sawtooth(.02), *path.hue_range()))
        r_a = Direction.random().xyz()
        r_b = Direction.random().xyz()
        r_c = Direction.random().xyz()
        m = .00005
        s_a = rnd(3, 5) * m
        s_b = rnd(3, 5) * m
        s_c = rnd(3, 5) * m
        view, proj = mvp_scene(height / float(width))
        while True:
            event = pygame.event.poll()
            if event.type == pygame.QUIT: break
            glClear(GL_COLOR_BUFFER_BIT)
            t = pygame.time.get_ticks()
            view_rot = numpy.dot(rotation_matrix(r_b, t * s_c),
                    numpy.dot(rotation_matrix(r_a, t * s_b),
                    numpy.dot(rotation_matrix(r_a, t * s_a), view)))
            if self.order_independent: dist = id
            else: dist = lambda s: numpy.dot(view_rot, list(s.avg.xyz())+[1])[3]
            for stroke in sorted(strokes, key=dist):
                stroke.draw(view_rot.flatten(), proj)
                #$for
            pygame.display.flip()
            #$while
        pygame.quit()

if __name__ == "__main__":
    d = Display(True)
    d.paths.append(test_path(7000))
    d.run()
