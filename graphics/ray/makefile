GRAPHICS=..
LDFLAGS=-L$(GRAPHICS)/rgb -lrgb -lm
CXXFLAGS=-Wall -Wextra -Werror -std=c++14 -pedantic -O3 -I$(GRAPHICS)/rgb
CFLAGS=-Wall -Wextra -Werror -std=c11 -pedantic -O3 -I$(GRAPHICS)/rgb

gun: gun.o tracer.a
	$(CXX) -o $@ $^ $(LDFLAGS)

tracer.a: mapping.o inter.o sphere.o plane.o cylinder.o \
	  cone.o parabol.o hyperbol.o saddle.o direction.o point.o ray.o \
	  observer.o matrix.o bitarray.o stack.o scene.o sky.o trace.o
	ar rc $@ $^

mapping.o:  mapping.h
inter.o:    inter.h
sphere.o:   sphere.h
plane.o:    plane.h
cylinder.o: cylinder.h
cone.o:     cone.h
parabol.o:  parabol.h
hyperbol.o: hyperbol.h
saddle.o:   saddle.h
direction.o:direction.h
point.o:    point.h
ray.o:      ray.h
observer.o: observer.h
matrix.o:   matrix.h
bitarray.o: bitarray.hpp
stack.o:    stack.hpp
scene.o:    scene.h
sky.o:      sky.h
trace.o:    trace.hpp

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f gun tracer.a
