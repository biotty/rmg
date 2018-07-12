CXXFLAGS=-Wall -Wextra -Werror -std=c++11 -pedantic -O3
CFLAGS=-Wall -Wextra -Werror -std=c11 -pedantic -O3
LDFLAGS=-lm

bluf: bluf.o librgb.a
	$(CXX) -o $@ $^ $(LDFLAGS)

librgb.a: color.o photo.o image.o
	$(AR) rc $@ $^

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f bluf librgb.a
