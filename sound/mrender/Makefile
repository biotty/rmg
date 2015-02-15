CXXFLAGS=-std=c++11

mrender: mrender.o midi.o biquad.o
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f mrender
