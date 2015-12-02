CXXFLAGS=-std=c++11 -I..

mrender: mrender.o midi.o biquad.o musicm.o
	$(CXX) $(CXXFLAGS) -o $@ $^

musicm.o: ../musicm.cpp
	$(CXX) $(CXXFLAGS) -c $^

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f mrender
