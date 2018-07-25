GRAPHICS := ..
CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3 -I$(GRAPHICS)/ray
LDFLAGS := -L$(GRAPHICS)/ray -lrayt -lm -pthread

FLOW := flow.hpp cellular.hpp planar.hpp linear.hpp

.PHONY:
all: wave fluid

wave: wave.o
	$(CXX) -o $@ $< $(LDFLAGS)

wave.o: wave.cpp wave.hpp $(FLOW)
	$(CXX) $(CXXFLAGS) -c $<

fluid: fluid.o
	$(CXX) -o $@ $< $(LDFLAGS)

fluid.o: fluid.cpp fluid.hpp $(FLOW) tracer.hpp palette.hpp
	$(CXX) $(CXXFLAGS) -c $<

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f wave fluid
