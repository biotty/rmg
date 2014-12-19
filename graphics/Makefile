CFLAGS = -Wall -Wextra -Werror -std=c11 -Iray -Iformat
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -Iformat
LDFLAGS = -lm

.PHONY: sw media interactive

sw: cray cwave cfluid

media: image1.jpeg video1.mp4 fluid.mp4 zoom.mp4

interactive:
	PYTHONPATH=py py/inter.py

cray: ray/main.o ray/tracer.a libformat.a
	$(CC) -o $@ $^ $(LDFLAGS)

cwave: flow/main_wave.o libformat.a
	$(CXX) -o $@ $^ $(LDFLAGS)

cfluid: flow/main_fluid.o libformat.a
	$(CXX) -o $@ $^ $(LDFLAGS)

ray/tracer.a: ray/map.o ray/inter.o ray/sphere.o ray/plane.o ray/cylinder.o ray/cone.o \
        ray/direction.o ray/point.o ray/ray.o ray/observer.o \
        ray/matrix.o ray/bitarray.o ray/stack.o ray/scene.o ray/sky.o ray/trace.o
	ar rc $@ $^

libformat.a: format/color.o format/photo.o format/image.o
	ar rc $@ $^

%.world: py/%.py py/rmg/scene.py
	py/$*.py 16 > $@

%.jpeg: cray %.world sky.pnm
	./cray 1920x1080 $@ < $*.world

sky.pnm: py/lsr.py
	echo "f x()x()x+y f()y()f x-y" | py/lsr.py -b128 -c12 512x512 >$@

%.mp4: cray movie.sh py/%.py sky.pnm
	./movie.sh $@ 'py/$*.py 512 "cray 1280x720"'

zoom.mp4: fract.sh
	py/fz.py | ./fract.sh $@

fluid.mp4: py/globe.py cfluid cmd.sh
	py/globe.py 512x512
	pnmtopng globe.pnm > img.png
	./cfluid -p /tmp/ -i img.png
	./cmd.sh /tmp/ $@

.PHONY:
clean:
	@rm -rf movie.d fract.d
	@rm -f *.o */*.o *.gch
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -f ray/tracer.a cray cwave cfluid *.png *.jpeg *.mp4 *.pnm
