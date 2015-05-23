#
#       © Christian Sommerfeldt Øien
#       All rights reserved

from sys import argv, stderr, stdout
from optparse import OptionParser
from subprocess import Popen, PIPE
from rmg.scene import World


class ParametricWorld:

    def __init__(self, p_objects, p_spots, p_observer, p_sky):
        self.objects = p_objects
        self.spots = p_spots
        self.observer = p_observer
        self.sky = p_sky

    def __call__(self, t):
        return World(self.objects(t),
                self.spots(t), self.observer(t), self.sky(t))


class ScriptInvocation:

    def __init__(self, frame_resolution, frame_count,
            trace_command, path_prefix, positional_args):
        self.frame_resolution = frame_resolution
        self.frame_count = frame_count
        self.trace_command = trace_command
        self.path_prefix = path_prefix
        self.positional_args = positional_args

    @classmethod
    def from_sys(cls):
        opts = OptionParser()
        opts.add_option("-c", "--frame-count", type="int", default=0)
        opts.add_option("-p", "--path-prefix", type="string", default="")
        opts.add_option("-r", "--resolution", type="string", default="1280x720")
        opts.add_option("-C", "--trace-command", type="string", default="gun")
        o, a = opts.parse_args()
        return cls(o.resolution, o.frame_count,
                o.trace_command, o.path_prefix, a)

    def tee(self, world):
        stdout.write("%s\n" % (world,))

    def image(self, world, path):
        data = bytes(str(world), 'ascii')
        comm = "%s %s %s" % (self.trace_command, self.frame_resolution, path)
        p = Popen(comm, stdin=PIPE, shell=True, close_fds=True)
        p.stdin.write(data)
        p.stdin.close()
        if p.wait() != 0:
            raise Exception("Failure on '%s'" % (comm,))

    def sequence(self, parametric_world):
        if not self.frame_count:
            raise Exception("Script not invoked for sequence")
        stderr.write("%d\n+" % (self.frame_count,))
        for i in range(self.frame_count):
            self.image(parametric_world(float(i) / self.frame_count),
                   "%s%d.jpeg" % (self.path_prefix, i,))
            stderr.write("\r%d" % (i,))
