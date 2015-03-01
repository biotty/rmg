GRAPHICS=..
LDFLAGS=-L$(GRAPHICS)/rgb -lrgb -lm
CXXFLAGS=-g -Wall -Wextra -Werror -std=c++11 -I$(GRAPHICS)/rgb
CFLAGS=-g -Wextra -Werror -std=c11 -I$(GRAPHICS)/rgb

gun: gun.o tracer.a
	$(CXX) -o $@ $^ $(LDFLAGS)

tracer.a: mapping.o inter.o sphere.o plane.o cylinder.o \
	  cone.o direction.o point.o ray.o observer.o \
      matrix.o bitarray.o stack.o scene.o sky.o trace.o
	ar rc $@ $^

mapping.o:  mapping.h
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
