CXXFLAGS=-Wall -Wextra -Werror -std=c++11 -O2
CFLAGS=-Wall -Wextra -Werror -std=c99 -O2
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
