SOUND := ..
CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3 \
 `python3-config --includes` -I$(SOUND)/synth

fuge.so: fuge.o lib.o $(SOUND)/synth/sizr.a
	$(CXX) -o $@ $^ -shared

fuge.o: fuge.cpp
	$(CXX) -c $^ $(CXXFLAGS) -fPIC

lib.o: lib.cpp
	$(CXX) -c $^ $(CXXFLAGS) -fPIC

fuge.cpp lib.cpp: fugelib.hpp

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f fuge.so
