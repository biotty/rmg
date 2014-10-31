import orchestra
import avrepr


class Playrepr:

    def __init__(self, *args):
        self.args = args

    @avrepr.media
    def __repr__(self):
        return self.player.play(*self.args)


class Pause(Playrepr): player = orchestra.Pause()
class Drum(Playrepr): player = orchestra.Drum()
class TenseString(Playrepr): player = orchestra.TenseString()


class HelloWorld:

    def __repr__(self):
        w = avrepr.Writer()
        w.set_color((255,0,255), (0,0,0))
        w.put_text("hello world")
        avrepr.media_collect(w.render())
        #example: would normally as above used @avrepr.media
        #return: \v for video (icons) and \a for audio (notes)
        #        and each respective collected media is zipped
        #        at callers writer (repr thus recursive thru
        #        repr-collections unaware of any media, because
        #        the \a and \v are seen in the composed
        #        repr-string from their repr(parts).
        return "\v"


P = Pause
X = Drum
def n(p): return lambda d: TenseString(d, p)
C, D_, D, E_, E, F, G_, G, A_, A, B_, B = [n(p) for p in range(60, 72)]

