GRAPHICS=..
CXXFLAGS=-Wall -Wextra -Werror -std=c++11 -I$(GRAPHICS)/rgb
LDFLAGS=-L$(GRAPHICS)/rgb -lrgb -lm

.PHONY:
all: wave fluid

wave: main_wave.o
	$(CXX) -o $@ $^ $(LDFLAGS)

fluid: main_fluid.o
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY:
clean:
	@rm -f *.o *.gch

.PHONY:
sweep: clean
	@rm -f wave fluid
