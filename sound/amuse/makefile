SOUND := ..
CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3 \
 `sdl2-config --cflags` -I$(SOUND)/synth
LDFLAGS := `sdl2-config --libs` $(SOUND)/synth/sizr.a -lncurses -lm

amuse: amuse.o orchestra.o page.o book.o recorder.o screen.o speaker.o
	$(CXX) -o $@ $^ $(SYNTH) $(LDFLAGS)

orchestra.cpp:orchestra.hpp
page.cpp:     page.hpp
book.cpp:     book.hpp
recorder.cpp: recorder.hpp
screen.cpp:   screen.hpp
speaker.cpp:  speaker.hpp

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f amuse
