SOUND := ..
CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3 -I$(SOUND)/synth

mrender: mrender.o midi.o biquad.o musicm.o
	$(CXX) $(CXXFLAGS) -o $@ $^

mrender.o: samplerate.hpp
biquad.o:  samplerate.hpp

musicm.o: $(SOUND)/synth/musicm.cpp
	$(CXX) $(CXXFLAGS) -c $^

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f mrender
