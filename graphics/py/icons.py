#!/usr/bin/env python3
#
#       © Christian Sommerfeldt Øien
#       All rights reserved
import pygame, os
from rmg import avrepr
from exceptions import Exception, EOFError, SystemExit
from random import randrange


PS1 = ">>>"
PS2 = "..."


class Recorder:

    frames_per_second = 10

    def __init__(self, screen):
        self.screen = screen
        self.frames_saved = 0
        self.tmp = "/tmp/R%d" % (randrange(0, 10**9),)
        os.mkdir(self.tmp)
        print(self.tmp)  #hack: practical data to user

    def savename(self):
        n = "%s/%d.jpeg" % (self.tmp, self.frames_saved)
        self.frames_saved += 1
        return n

    def pressed_key(self):
        self.advance(randrange(1, 4))

    def pressed_enter(self):
        self.advance(randrange(4, 9))

    def got_prompt(self):
        self.advance(randrange(4, 9))

    def advance(self, instants):
        self.dump(instants)

    def dump(self, copies):
        if not copies: return
        name = self.savename()
        pygame.image.save(self.screen, name)
        for _ in range(1, copies):
            os.link(name, self.savename())


class EditorError(Exception): pass


class Console(avrepr.Writer):


    def __init__(self, screen, recorder):
        avrepr.Writer.__init__(self)
        self.vertical_scroll = 0
        self.screen = screen
        self.recorder = recorder
        self.clock = pygame.time.Clock()
        self.locals = {
                "__name__": "__workspace__",
                "__doc__": "Multimedia Console" }

    position = property(lambda s: (
        s.row.width, s.row.y - s.vertical_scroll))

    def invite(self, prompt):
        self.set_color(*avrepr.settings.prompt_pen)
        self.put_text(prompt)
        self.set_color(*avrepr.settings.input_pen)
    
    def put_icon(self, i):
        w, h = self.screen.get_size()
        a, b = i.get_size()
        if self.row.width + a > w:
            self.new_row()
        y = self.row.y - self.vertical_scroll
        d = (y + b) - h
        if d > 0:
            self.vertical_scroll += d
            self.refresh()
        self.screen.blit(i, self.position)
        self.row.put_icon(i)

    def refresh(self):
        self.screen.blit(self.spunge(), (0, 0))
        self.draw(self.screen)

    def spunge(self):
        w = pygame.Surface(self.screen.get_size())
        w.fill((0, 0, 0))
        return w

    def draw(self, surface):
        d = 0, -self.vertical_scroll
        for r in self.rows:
            r.displaced(d).draw(surface)

    def backspace(self):
        rubber = self.row.unput_icon()
        self.screen.blit(rubber, self.position)

    def input(self, prompt):
        self.invite(prompt)
        if self.recorder: self.recorder.got_prompt()
        dirty = True
        scrolled = False
        a = []
        while 1:
            for e in pygame.event.get():
                if e.type == pygame.KEYDOWN:
                    if e.key in (pygame.K_PAGEUP, pygame.K_PAGEDOWN):
                        self.vertical_scroll += avrepr.settings.pagescroll \
                                * (1 - 2 * (e.key == pygame.K_PAGEUP))
                        scrolled = dirty = True
                        continue
                    u = e.unicode
                    if not u: continue
                    c = ord(u)
                    if c >= 0x20:
                        a.append(u)
                        self.put_text(u)
                        dirty = True
                    if c == 0x03: raise EditorError("^C")
                    if c == 0x04 and not a: raise EOFError()
                    if c == 0x08 and a:
                        a.pop()
                        self.backspace()
                        dirty = True
                    if c == 0x0d:
                        if self.recorder:
                            self.recorder.pressed_enter()
                        self.new_row()
                        return "".join(a)
            if dirty:
                if scrolled: self.refresh()
                elif self.recorder: self.recorder.pressed_key()
                pygame.display.flip()
                scrolled = dirty = False
            self.clock.tick(20)
    
    def execute(self, expr):
        try:
            value = expr(self.locals)
            if value is not None:
                self.rprint(value)
        except SystemExit as x: sys.exit(x.code)
        except avrepr.EvalError as x: self.error(str(x))
        except Exception as x:
            self.error(avrepr.trimmed_exc())

    def rprint(self, *args):
        self.media_repr(list(args))
        self.new_row()

    def error(self, s):
        self.set_color(*avrepr.settings.error_pen)
        self.put_text(s)

    def interact(self):
        while 1:
            p = PS1
            e = None
            t = avrepr.Source()
            while not e:
                try: s = self.input(p)
                except EOFError: return
                except EditorError as x:
                    self.error("%s\n" % (x,))
                    break
                t.push(s)
                try: e = t.compile()
                except avrepr.ConsoleError as x:
                    self.error("%s\n" % (x,))
                    t.clear()
                    continue
                p = PS2
            else:
                self.execute(e)


pygame.init()
pygame.display.set_caption("Workspace")
screen = pygame.display.set_mode((1024, 600))
Console(screen, Recorder(screen)).interact()
