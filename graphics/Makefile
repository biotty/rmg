CFLAGS = -Wall -Wextra -Werror -std=c99 -Iray -I.
CXXFLAGS = -Wall -Wextra -Werror -std=c++0x -I.
LDFLAGS = -lm

.PHONY: sw media interactive

sw: cray cwave cfluid

media: random.jpeg fluid.mp4 zoom.mp4 cloud.mp4

interactive:
	PYTHONPATH=py py/inter.py

cray: ray/main.o ray/tracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

cwave: flow/main_wave.o color.o image.o photo.o
	$(CXX) -o $@ $^ $(LDFLAGS)

cfluid: flow/main_fluid.o color.o image.o photo.o
	$(CXX) -o $@ $^ $(LDFLAGS)

ray/tracer.a: ray/map.o ray/inter.o ray/sphere.o ray/plane.o ray/cylinder.o ray/cone.o \
        ray/direction.o ray/point.o ray/ray.o ray/observer.o color.o image.o photo.o \
        ray/matrix.o ray/bitarray.o ray/stack.o ray/scene.o ray/sky.o ray/trace.o
	ar rc $@ $^

random.world: py/random_.py py/rmg/scene.py
	py/random_.py 16 > random.world

random.jpeg: cray random.world sky.pnm
	./cray 1920x1080 $@ < random.world

sky.pnm: py/lsr.py
	echo "f x()x()x+y f()y()f x-y" | py/lsr.py -b128 -c12 512x512 >$@

cloud.mp4: cray movie.sh py/cloud.py dragon.pnm
	./movie.sh $@ 'py/cloud.py 512 "cray 1280x720"'

zoom.mp4: fract.sh
	py/fz.py | ./fract.sh $@

fluid.mp4: py/globe.py cfluid cmd.sh
	py/globe.py 512x512
	pnmtopng globe.pnm > img.png
	./cfluid
	mv *.jpeg /tmp/
	./cmd.sh /tmp/ $@

.PHONY:
clean:
	@rm -rf movie.d fract.d
	@rm -f *.o */*.o *.gch
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -f *.world ray/tracer.a cray cwave cfluid core* *.png *.jpeg *.mp4 *.pnm
