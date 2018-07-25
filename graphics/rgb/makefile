RAY := ../ray
CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3 -I$(RAY)
LDFLAGS := -L$(RAY) -lrayt -lm

bluf: bluf.o
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f bluf
