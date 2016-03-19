
SOUND=..
CXXFLAGS=-std=c++11 -Wextra -Werror -I$(SOUND)/synth

mrender: mrender.o midi.o biquad.o musicm.o
	$(CXX) $(CXXFLAGS) -o $@ $^

musicm.o: $(SOUND)/synth/musicm.cpp
	$(CXX) $(CXXFLAGS) -c $^

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f mrender
