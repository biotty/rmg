#
#       (c) Christian Sommerfeldt OEien
#       All rights reserved

from sys import argv, stderr, stdout
from argparse import ArgumentParser
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

    def __init__(self, frame_resolution, frame_count, parallel,
            trace_command, output_path, time_offset, world_tee, args):
        self.frame_resolution = frame_resolution
        self.frame_count = frame_count
        self.parallel = parallel
        self.trace_command = trace_command
        self.output_path = output_path
        self.time_offset = time_offset
        self.world_tee = world_tee
        self.args = dict(enumerate(args))  # purpose: get with default

    @classmethod
    def from_sys(cls):
        ap = ArgumentParser()
        ap.add_argument("-n", "--frame-count", type=int, default=0)
        ap.add_argument("-j", "--parallel", type=int, default=1)
        ap.add_argument("-o", "--output-path", type=str, default="")
        ap.add_argument("-r", "--resolution", type=str, default="1280x720")
        ap.add_argument("-C", "--trace-command", type=str, default="rayt")
        ap.add_argument("-t", "--time-offset", type=float, default=0)
        ap.add_argument("-w", "--world-tee", action="store_true")
        ap.add_argument("user_defined_args", nargs="*", type=str)
        o = ap.parse_args()
        return cls(o.resolution, o.frame_count, o.parallel,
                o.trace_command, o.output_path, o.time_offset, o.world_tee,
                o.user_defined_args)

    def pipe(self, world, path):
        data = bytes(str(world), 'ascii')
        comm = (self.trace_command, self.frame_resolution, path)
        p = Popen(comm, stdin=PIPE)
        p.stdin.write(data)
        p.stdin.close()
        return p

    def image(self, world, path):
        if self.pipe(world, path).wait() != 0:
            sys.stderr.write("trace command failed for %s" % path)

    def images(self, task_descrs):
        for path, pipe in [(path, self.pipe(world, path))
                for world, path in task_descrs]:
            if pipe.wait() != 0:
                raise Exception("trace command failed for %s" % path)

    def run(self, parametric_world):
        if not self.frame_count:
            world = parametric_world(self.time_offset)
            if self.output_path: self.image(world, self.output_path)
            else: stdout.write("%s\n" % (world,))
        else:
            stderr.write("%d\n+" % (self.frame_count,))
            for j in range(0, self.frame_count, self.parallel):
                task_descrs = []
                for i in range(j, j + self.parallel):
                    world = parametric_world(
                        float(i) / self.frame_count + self.time_offset)
                    if self.world_tee:
                        with open("%s%d.world" % (self.output_path, i,), "w") as f:
                            f.write("%s\n" % (world,))
                    task_descrs.append(
                            (world, "%s%d.jpeg" % (self.output_path, i,)))
                self.images(task_descrs)
                stderr.write("\r%d" % (j,))
