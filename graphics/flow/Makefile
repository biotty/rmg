GRAPHICS=..
CXXFLAGS=-Wall -Wextra -Werror -std=c++11 -O2 -I$(GRAPHICS)/rgb
LDFLAGS=-L$(GRAPHICS)/rgb -lrgb

.PHONY:
all: wave fluid

wave: wave.o wave.hpp
	$(CXX) -o $@ $< $(LDFLAGS)

fluid: fluid.o fluid.hpp
	$(CXX) -o $@ $< $(LDFLAGS)

.PHONY:
clean:
	@rm -f *.o *.gch

.PHONY:
sweep: clean
	@rm -f wave fluid
