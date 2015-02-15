GRAPHICS=..
CFLAGS=-Wextra -Werror -std=c11 -I$(GRAPHICS)/rgb
LDFLAGS=-L$(GRAPHICS)/rgb -lrgb -lm

gun: main.o tracer.a
	$(CC) -o $@ $^ $(LDFLAGS)

tracer.a: map.o inter.o sphere.o plane.o cylinder.o \
	  cone.o direction.o point.o ray.o observer.o \
      matrix.o bitarray.o stack.o scene.o sky.o trace.o
	ar rc $@ $^

map.o:      map.h
inter.o:    inter.h
sphere.o:   sphere.h
plane.o:    plane.h
cylinder.o: cylinder.h
cone.o:     cone.h
direction.o:direction.h
point.o:    point.h
ray.o:      ray.h
observer.o: observer.h
matrix.o:   matrix.h
bitarray.o: bitarray.h
stack.o:    stack.h
scene.o:    scene.h
sky.o:      sky.h
trace.o:    trace.h

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f gun tracer.a
