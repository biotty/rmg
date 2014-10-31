import pygame
import music
import sys, code, re
from exceptions import Exception
from traceback import format_exc
from numbers import Complex
from functools import wraps


WHITE = 255, 255, 255
GRAY = 50, 50, 50
BLACK = 0, 0, 0
RED = 255, 0, 0
GREEN = 0, 255, 0
BLUE = 0, 0, 255


class ConsoleError(Exception): pass
class EvalError(Exception): pass
class Settings(): pass


settings = Settings()
settings.prompt_pen = GREEN, BLACK
settings.input_pen = WHITE, BLACK
settings.string_pen = BLUE, BLACK
settings.number_pen = WHITE, GRAY
settings.repr_pen = GREEN, BLACK
settings.error_pen = RED, BLACK
settings.pagescroll = 10


def trimmed_exc():
    t = format_exc().split("\n")
    a = []
    for s in t[3:]:
        m = re.match(r'\s+File ".+?",\s*', s)
        a.append(s[len(m.group(0)):] if m else s)
    return "\n".join(a)


class Expression:

    def __init__(self, source):
        self.source = source

    def __call__(self, locals):
        try: return eval(self.source, locals)
        except Exception as e: raise EvalError(trimmed_exc())


class Statement(Expression):

    def __call__(self, locals):
        try: exec self.source in locals
        except Exception as e: raise EvalError(trimmed_exc())


class CommentTracker:

    def __init__(self, notation):
        self.inside = False
        self.enter, self.leave = notation

    def see(self, c):
        if c == self.enter: self.inside = True
        if c == self.leave: self.inside = False

    outside = property(lambda s: not s.inside)


class QuoteTracker:

    def __init__(self, quote):
        self.quote = quote
        self.inside = False

    def see(self, c):
        if c == self.quote:
            self.inside = not self.inside

    outside = property(lambda s: not s.inside)


class LoneTracker:

    def __init__(self, e):
        self.e = e
        self.i = 0
        self.found = 0

    def see(self, c):
        if self.e == c:
            if not self.found: self.found = self.i
            elif self.found == self.i - 1: self.found = 0
        self.i += 1


class ParensTracker:

    def __init__(self, *pairs):
        self.pairs = pairs
        self.stack = []

    def see(self, c):
        for p in self.pairs:
            if c == p[0]: self.stack.append(c)
            elif c == p[1]:
                if not self.stack:
                    raise ConsoleError("- nah %s" % (c,))
                d = self.stack.pop()
                if d != p[0]:
                    raise ConsoleError("%s nah %s" % (d, c))


class Line:

    def __init__(self, s):
        assert not "\n" in s
        self.chars = list(s)

    def __str__(self):
        return "".join(["%s" % (c,) for c in self.chars] + ["\n"])


class Source:

    def __init__(self):
        self.lines = []

    def push(self, s):
        self.lines.append(Line(s))

    def clear(self):
        self.lines = []

    def __str__(self):
        return "".join(["%s" % (l,) for l in self.lines])

    def compile(self):
        s = str(self)
        assert s[-1] == "\n"
        if s.isspace(): raise ConsoleError("- yes ?")
        squotes = QuoteTracker("'")
        dquotes = QuoteTracker('"')
        eqalone = LoneTracker("=")
        parenth = ParensTracker("()", "[]", "{}")
        comment = CommentTracker("#\n")
        for i, c in enumerate(s):
            comment.see(c)
            if comment.outside:
                if not squotes.inside: dquotes.see(c)
                if not dquotes.inside: squotes.see(c)
                if squotes.outside and dquotes.outside:
                    eqalone.see(c)
                    parenth.see(c)
        if parenth.stack:
            return
        if s[-2] == "\n":
            return Statement(s)
        if re.match("(if|for|while|def|class) ", s):
            return
        if re.match("(pass|import|from) ", s):
            return Statement(s)
        return (Statement if eqalone.found else Expression)(s)


repr_writer = None


def media_collect(m):
    if isinstance(m, Icon): c = "\v"
    elif isinstance(m, music.Note): c = "\a"
    else: return m
    repr_writer.avlist.append(m)
    return c


def media(f):
    return wraps(f)(lambda self: media_collect(f(self)))


class List(list):

    separator = classmethod(lambda c: ", ")
    tone = classmethod(lambda c, note: media_collect(note))

    def __repr__(self):
        a = []
        train = False
        for e in self:
            w = Writer()
            w.media_repr(e)
            note = w.compose()
            if note: a.append(self.tone(note))
            icon = w.render()
            if icon:
                if train: a.append(self.separator())
                a.append(media_collect(icon))
                train = True
        return "".join(a)


class HorizontalList(List):

    @classmethod
    def separator(c):
        return media_collect(PIPE)


class VerticalList(List):

    @classmethod
    def separator(c):
        return "\n%c\n" % (media_collect(PIPE),)

    tone = classmethod(lambda c, note: "\n%c"
            % (media_collect(note),))


class Icon(pygame.Surface):
    pass


class Selfrepr:

    @media
    def __repr__(self):
        return self


class PipeFactory(Icon):

    WIDTH = 1
    HEIGHT = 1
    COLOR = (255, 255, 255)

    def __init__(self):
        #note: Icon is just a camouflage
        self.get_size = lambda: (
                PipeFactory.WIDTH, PipeFactory.HEIGHT)

    @staticmethod
    def horizontal(width):
        s = Icon((width, PipeFactory.HEIGHT))
        s.fill(PipeFactory.COLOR)
        return s

    @staticmethod
    def vertical(height):
        s = Icon((PipeFactory.WIDTH, height))
        s.fill(PipeFactory.COLOR)
        return s


PIPE = PipeFactory()


r = pygame.Rect((0, 0), (8, 8))
emptyrow_icon = Icon(r.size)
pygame.draw.rect(emptyrow_icon, GRAY, r, 2)


class Row:

    def __init__(self, y):
        self.x = 0
        self.y = y
        self.icons = []
        self.score = []

    @property
    def width(self):
        return sum(i.get_size()[0] for i in self.icons) \
                if self.icons else 0

    @property
    def height(self):
        return max(i.get_size()[1] for i in self.icons) \
                if self.icons else 0

    def put_note(self, note):
        self.score.append(note)

    def put_icon(self, icon):
        self.icons.append(icon)

    def unput_icon(self):
        icon = self.icons.pop()
        a, b = icon.get_size()
        rubber = Icon((a, b))
        rubber.fill((0, 0, 0))
        return rubber

    def draw(self, surface):
        x = self.x
        for icon in self.icons:
            if icon is PIPE:
                icon = PIPE.vertical(self.height)
            a, _ = icon.get_size()
            surface.blit(icon, (x, self.y))
            x += a

    def displaced(self, m):
        assert self.x == 0, "displaced original"
        a, b = m
        r = Row(self.y + b)
        r.x = a  #note: since original is asserted
        r.icons = self.icons  #note: same one list
        return r


class Writer:

    def __init__(self):
        self.rows = [Row(0)]
        self.ink = (210, 255, 210)
        self.paper = (0, 40, 0)
        self.font = pygame.font.SysFont("monospace", 36, bold = True)
        self.avlist = None

    row = property(lambda s: s.rows[-1])
    put_icon = lambda s, v: s.row.put_icon(v)
    put_note = lambda s, v: s.row.put_note(v)

    def set_color(self, ink, paper):
        self.ink = ink
        self.paper = paper

    def put_text(self, t):
        for u in t:
            if u == "\n": self.new_row()
            else: self.put_icon(self.glyph(u))

    def new_row(self):
        if not self.row.icons:
            self.put_icon(emptyrow_icon)
        b = self.row.height
        y = self.row.y + b
        self.rows.append(Row(y))

    def glyph(self, u):
        s = self.font.render(u, 1, self.ink)
        i = Icon(s.get_size())
        i.fill(self.paper)
        i.blit(s, (0, 0))
        return i

    def media_repr(self, value):
        if str == type(value):  #todo: unicode
            self.set_color(*settings.string_pen)
            self.put_text(value)
        elif isinstance(value, Complex):
            self.set_color(*settings.number_pen)
            self.put_text(repr(value))
        else:
            if type(value) == list: value = HorizontalList(value)
            if type(value) == tuple: value = VerticalList(value)
            self.set_color(*settings.repr_pen)
            global repr_writer
            prev = repr_writer
            repr_writer = self
            self.avlist = []
            self.zip_media(repr(value), self.avlist)
            repr_writer = prev

    def zip_media(self, s, b):
        a = []
        t = []
        z = 0
        for i, c in enumerate(s):
            if c in "\a\v":
                a.append(s[z:i])
                t.append(c)
                z = i + 1
        a.append(s[z:])
        self.put_text(a[0])
        for j, m in enumerate(b):
            if isinstance(m, Icon):
                assert t[j] == "\v"
                self.put_icon(m)
            elif isinstance(m, music.Note):
                assert t[j] == "\a"
                self.put_note(m)
            else: print "%r?" % (m,)
            self.put_text(a[j + 1])

    def render(self):
        width = max(r.width for r in self.rows)
        height = sum(r.height for r in self.rows)
        if not (width and height): return
        icon = Icon((width, height))
        for r in self.rows:
            if r.icons and r.icons[0] is PIPE:
                r.icons[0] = PIPE.horizontal(width)
            r.draw(icon)
        return icon

    def compose(self):
        y = False
        c = music.NoteComposition()
        for r in self.rows:
            if r.score:
                c.add_row(r.score)
                y = True
        if y:
            return c
