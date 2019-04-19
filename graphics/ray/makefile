CXXFLAGS := -Wall -Wextra -Werror -std=c++17 -pedantic -O3
CFLAGS := -Wall -Wextra -Werror -std=c11 -pedantic -O3
LDFLAGS := -lm -pthread

rayt: rayt.o librayt.a
	$(CXX) -o $@ $^ $(LDFLAGS)

example: example.o librayt.a
	$(CXX) -o $@ $^ $(LDFLAGS)

librayt.a: world_gen.o mapping.o inter.o sphere.o plane.o cylinder.o \
  cone.o parabol.o hyperbol.o saddle.o scene.o render.o sky.o \
  direction.o point.o ray.o color.o photo.o image.o \
  matrix.o trace.o
	ar rc $@ $^

world_gen.o:world_gen.hpp
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
color.o:    color.h
render.o:   render.hpp
matrix.o:   matrix.h
scene.o:    scene.h scene.hpp
trace.o:    trace.hpp
sky.o:      sky.h
photo.o:    photo.h
image.o:    image.h

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f rayt example librayt.a
