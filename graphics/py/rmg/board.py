#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved
"""PNM (or JPEG) filter facility; read, buffer and write a set of files"""
#note:  pure black-white not supported (P4); only gray (P5) and color (P6)


from rmg.plane import XY
from rmg.color import Color
from copy import deepcopy
from array import array
import sys, os, re


class Quality:
    def __init__(self, width, height, gray):
        self.width = width
        self.height = height
        self.gray = gray

    def __repr__(self):
        return "Quality(%d, %d, %d)" % (self.width, self.height, self.gray)

    def __eq__(self, other):
        return (self.width, self.height, self.gray) \
                == (other.width, other.height, other.gray)

    def __ne__(self, other):
        return not (self == other)


class Photo:
    """Reads a PNM (JPEG if such path) and provides access to RGB tripples"""

    def __init__(self, width, height, data, gray = False):
        self.quality = Quality(width, height, gray)
        self.data = data
        if gray and len(data) < width * height:
            sys.stderr.write("PNM data (gray) short\n")
        if not gray and len(data) < 3 * width * height:
            sys.stderr.write("PNM data (rgb) short\n")

    @staticmethod
    def from_stream(ins = sys.stdin):
        buf = ins.buffer.read(128)  #note: will block on tiny photos
        if len(buf) == 0:
            return None #eof
        tokens = buf.split(None, 4)
        some_data = tokens[4]
        if tokens[0] not in (b"P5", b"P6") or tokens[3] != b"255":
            sys.stderr.write("stdin PNM not okay; P5 or P6, with max 255\n")
            return Nono
        gray = tokens[0] == b"P5"
        width = int(tokens[1])
        height = int(tokens[2])
        size = width * height * (1 if gray else 3) - len(some_data)
        data = some_data + ins.buffer.read(size)
        return Photo(width, height, data, gray)

    @staticmethod
    def from_file(path):
        if re.match(".*\.jpe?g", path, re.IGNORECASE):
            st = os.popen("jpegtopnm " + path, "r")
        else:
            st = open(path, "r")
        ph = Photo.from_stream(st)
        st.close()
        return ph

    def binary_rgb_at(self, column, row):
        i = row * self.quality.width + column
        if self.quality.gray: return self.data[i] * 3
        else: return self.data[i*3 : (i+1)*3]

    def color_at(self, column, row):
        return Color.from_binary_rgb(self.binary_rgb_at(column, row))

    def colors(self):
        for row in range(self.quality.height):
            for column in range(self.quality.width):
                yield self.color_at(column, row)


class MonoPhoto(Photo):
    def __init__(self, width, height, color):
        self.binary_rgb_ = color.binary_rgb()
        self.quality = Quality(width, height, False)

    def binary_rgb_at(self, column, row):
        return self.binary_rgb_


class Image:
    """Writes a PNM (JPEG if such path) pixel by pixel"""

    def __init__(self, width, height, path = None, gray = False):
        self.quality = Quality(width, height, gray)
        if not path:
            self.outs = sys.stdout
        elif re.match(".*\.jpe?g", path, re.IGNORECASE):
            self.outs = os.popen("pnmtojpeg >" + path, "w")
        else:
            self.outs = open(path, "w")
        s = "%s\n%d %d 255\n" % ("P5" if gray else "P6", width, height)
        b = bytes(s, 'ascii')
        self.outs.buffer.write(b)

    def put(self, color):
        if self.quality.gray:
            b = bytes([int(round(color.intensity() * 255))])
            self.outs.buffer.write(b)
        else:
            self.outs.buffer.write(color.binary_rgb())

    def close(self):
        if self.outs != sys.stdout: self.outs.close()
        else: self.outs.flush()


class Board:
    def __init__(self, matrix):
        self.n_rows = len(matrix)
        self.n_columns = len(matrix[0])
        data = array("L")
        for row in matrix: data.extend(row)
        self.data = data

    def copy(self):
        return deepcopy(self)

    def contains(self, x, y):
        return x >= 0 and x < self.n_columns and \
                y >= 0 and y < self.n_rows

    @classmethod
    def mono(cls, width, height, color):
        return cls.from_photo(MonoPhoto(width, height, color))

    @classmethod
    def from_photo(cls, photo):
        matrix = []
        for row in range(photo.quality.height):
            r = []
            for column in range(photo.quality.width):
                r.append(photo.color_at(column, row).int_rgb())
            matrix.append(r)
        return cls(matrix)

    def int_at(self, column, row):
        return self.data[self.n_columns * row + column]

    def set_int(self, column, row, i):
        self.data[self.n_columns * row + column] = i

    def color_at(self, column, row):
        return Color.from_int_rgb(self.int_at(column, row))

    def set_color(self, column, row, color):
        self.data[self.n_columns * row + column] = color.int_rgb()

    def save(self, path, gray = False):
        image = Image(self.n_columns, self.n_rows, path, gray)
        for i in self.data: image.put(Color.from_int_rgb(i))
        image.close()

    def to_stream(self, gray = False):
        self.save(None, gray)


def mostly_vertically(at, to):
    x_distance = abs(to.x - at.x)
    y_distance = abs(to.y - at.y)
    return x_distance < y_distance


class Pencil:
    def __init__(self, board, projection = None):
        self.board = board
        self.projection = projection or (lambda p: (p.x, 1-p.y))
        self.at = None
        self.last_touched_xy = None

    def touch_pixel(self, xy, color):
        x, y = xy
        if self.board.contains(x, y):
            self.board.set_color(x, y, color)

    def touch_pixel_careful(self, xy, color):
        if xy != self.last_touched_xy:
            self.touch_pixel(xy, color)

    def move_(self, xy):
        self.at = xy.copy()
        self.last_touched_xy = None

    def draw(self, p, color):
        x, y = self.projection(p)
        p = XY(x * self.board.n_columns, y * self.board.n_rows)
        if color: self.draw_line(p, color)
        else: self.move_(p)

    def draw_line(self, to, color):
        if to == self.at:
            sys.stderr.write("draw_line to same point\n")
            return
        at_ = self.at.copy()
        to_ = to.copy()
        self.at = to.copy()
        yx = mostly_vertically(at_, to_)
        if yx:
            at_.x, at_.y = at_.y, at_.x
            to_.x, to_.y = to_.y, to_.x
        if to_.x < at_.x:
            at_, to_ = to_, at_
        ratio = (to_.y - at_.y) / (to_.x - at_.x)
        stepping_x, last_x = int(at_.x), int(to_.x)
        floating_y = at_.y - ratio * (at_.x - stepping_x)
        x, y = stepping_x, int(floating_y)
        if yx:
            x, y = y, x
        self.touch_pixel_careful((x, y), color)
        while stepping_x != last_x:
            stepping_x += 1
            floating_y += ratio
            x, y = stepping_x, int(floating_y)
            if yx:
                x, y = y, x
            self.touch_pixel((x, y), color)
        self.last_touched_xy = x, y

