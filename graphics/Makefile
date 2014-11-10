CFLAGS = -Wall -Wextra -Werror -std=c11
CXXFLAGS = -Wall -Wextra -Werror -std=c++11
LDFLAGS = -lm

.PHONY: sw
sw: cray cwave cfluid tests

.PHONY: media
media: random.jpeg dragon.png tree.jpeg fl.mp4 fz.mp4 cloud.avi

cray: cray.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

cwave: cwave.o image.o color.o photo.o
	$(CXX) -o $@ $^ $(LDFLAGS)

cfluid: cfluid.o image.o color.o photo.o
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: tests
tests: direction_test sphere_test plane_test cone_test observer_test matrix_test color_test bitarray_test scene_test trace_test
	./direction_test
	./sphere_test
	./plane_test
	./cone_test
	./observer_test
	./matrix_test
	./color_test
	./bitarray_test
	./scene_test
	./trace_test

raytracer.a: map.o inter.o sphere.o plane.o cylinder.o cone.o direction.o point.o ray.o observer.o color.o image.o photo.o matrix.o bitarray.o stack.o scene.o sky.o trace.o
	ar rc $@ $^

bitarray.o: bitarray.h
color.o:    color.h
cone.o:     cone.h
cylinder.o: cylinder.h
direction.o:direction.h
image.o:    image.h
inter.o:    inter.h
map.o:      map.h
math.o:     math.h
matrix.o:   matrix.h
observer.o: observer.h
photo.o:    photo.h
plane.o:    plane.h
point.o:    point.h
ray.o:      ray.h
real.o:     real.h
scene.o:    scene.h
sky.o:      sky.h
sphere.o:   sphere.h
stack.o:    stack.h
trace.o:    trace.h

direction_test: direction_test.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

sphere_test: sphere_test.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

plane_test: plane_test.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

cone_test: cone_test.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

observer_test: observer_test.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

color_test: color_test.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

matrix_test: matrix_test.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

bitarray_test: bitarray_test.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

scene_test: scene_test.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

trace_test: trace_test.o raytracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

random.world: py/random_.py py/rmg/scene.py
	py/random_.py 16 > random.world

sky.pnm:
	(echo "P6 9 9 255" ; head -c243 /dev/urandom) >$@

random.jpeg: cray random.world sky.pnm
	./cray 1920x1080 $@ < random.world

dragon.png: py/lsr.py
	echo "f x()x()x+y f()y()f x-y" | py/lsr.py -b128 -c12 512x512 | pnmtopng >$@

tree.jpeg: py/lsr.py cray sky.pnm
	PATH=${PWD}:${PATH} py/lsr.py -c4 -tw $@ -r 1024x1024 -e "-:'0.25'/:'0.25'f()f(){/:'rnd(1)'[^:'rnd(.05,.15)'@:'rnd(.82,1)'w f][&:'rnd(.05,.15)'@:'rnd(.82,1)'w f]}{:'0.1'w}"

cloud.avi: cray movie.sh py/cloud.py sky.pnm
	./movie.sh $@ 'py/cloud.py 512 "cray 1280x720"'

fz.mp4: fract.sh
	py/fz.py | ./fract.sh $@

fl.mp4: py/globe.py cfluid cmd.sh
	py/globe.py 512x512
	pnmtopng globe.pnm > img.png
	./cfluid
	mv *.jpeg /tmp/
	./cmd.sh /tmp/ $@

.PHONY:
clean:
	@rm -rf movie.d fract.d
	@rm -f *.[oa] *.gch *_test 
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -f cray cwave cfluid core* \
		*.png *.jpeg *.avi *.mp4 *.pnm
