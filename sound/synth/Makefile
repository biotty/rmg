CXXFLAGS=-Wextra -Werror -std=c++11 -fPIC

GTEST_I=-I$(HOME)/gtest/include
GTEST_L=$(HOME)/gtest/lib/gtest_main.a -lpthread

sizr.a: builders.o envelopes.o filters.o generators.o musicm.o unit.o
	$(AR) cr $@ $^

builders.cpp:  builders.hpp
envelopes.cpp: envelopes.hpp
filters.cpp:   filters.hpp
generators.cpp:generators.hpp
musicm.cpp:    musicm.hpp
unit.cpp:      unit.hpp

.PHONY:
clean:
	@rm -f *.o

.PHONY:
sweep: clean
	@rm -f sizr.a

.PHONY: test
test: ringbuf_test
	./ringbuf_test

ringbuf_test: ringbuf_test.o
	$(CXX) -o $@ $^ $(GTEST_L) $(LDFLAGS)

ringbuf_test.o: ringbuf_test.cpp
	$(CXX) -c $(GTEST_I) $^

