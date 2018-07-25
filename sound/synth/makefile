CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3 -fPIC

GTEST_DIR := $(HOME)/gtest
GTEST_LIB := $(GTEST_DIR)/src
GTEST_CXXFLAGS := -I$(GTEST_DIR)/include
GTEST_LDFLAGS := -pthread

sizr.a: builders.o envelopes.o filters.o generators.o unit.o musicm.o
	$(AR) cr $@ $^

builders.cpp:  builders.hpp
envelopes.cpp: envelopes.hpp
filters.cpp:   filters.hpp
generators.cpp:generators.hpp
musicm.cpp:    musicm.hpp
unit.cpp:      unit.hpp

.PHONY:
clean:
	@rm -f *.o test.a

.PHONY:
sweep: clean
	@rm -f *.a *_test

test.a: $(GTEST_LIB)/gtest-all.o $(GTEST_LIB)/gtest_main.o
	$(AR) cr $@ $^

.PHONY: test
test: ringbuf_test
	./ringbuf_test

ringbuf_test: ringbuf_test.o test.a
	$(CXX) -o $@ $^ $(GTEST_LDFLAGS)

ringbuf_test.o: ringbuf_test.cpp
	$(CXX) -c $(GTEST_CXXFLAGS) $^
