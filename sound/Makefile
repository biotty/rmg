BINDIR=bin
OBJDIR=obj
LIBDIR=lib

CXXFLAGS=-Wall -Wextra -Werror -std=c++11 -MMD -MP -fPIC `sdl2-config --cflags` -Iinclude
LDFLAGS=`sdl2-config --libs` -lncurses -lm

GTEST_I=-I$(HOME)/gtest/include
GTEST_L=$(HOME)/gtest/lib/gtest_main.a -lpthread

VPATH=src

MRENDER_OBJ=\
	   $(OBJDIR)/mrender.o \
	   $(OBJDIR)/midi.o \
	   $(OBJDIR)/biquad.o

DIGITARP_OBJ=$(OBJDIR)/digitarp.o

AMUSE_OBJ=\
	  $(OBJDIR)/amuse.o \
	  $(OBJDIR)/page.o \
	  $(OBJDIR)/book.o \
	  $(OBJDIR)/screen.o \
	  $(OBJDIR)/orchestra.o \
	  $(OBJDIR)/speaker.o \
	  $(OBJDIR)/recorder.o

SYNTH_OBJ=\
	  $(OBJDIR)/unit.o \
	  $(OBJDIR)/generators.o \
	  $(OBJDIR)/builders.o \
	  $(OBJDIR)/envelopes.o \
	  $(OBJDIR)/filters.o \
	  $(OBJDIR)/musicm.o

ALL_OBJECTS=\
	$(MRENDER_OBJ) \
	$(DIGITARP_OBJ) \
	$(AMUSE_OBJ) \
	$(SYNTH_OBJ) \

MRENDER=$(BINDIR)/mrender
DIGITARP=$(BINDIR)/digitarp
AMUSE=$(BINDIR)/amuse
SYNTH=$(LIBDIR)/synth.a
FUGE=$(LIBDIR)/python/fuge.so

.PHONY:
all: $(AMUSE) $(DIGITARP) $(MRENDER) $(FUGE)

$(ALL_OBJECTS): dirs

$(FUGE): $(SYNTH)
	./setup.py install --home .

.PHONY:
dirs:
	@mkdir -p $(OBJDIR) $(LIBDIR) $(BINDIR)

$(OBJDIR)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

.PHONY:
media: out.mp3 compo.mp3

%.mp3: %.raw
	lame -mm -r -s 44.1 --signed --bitwidth 16 --little-endian $^ $@

out.raw: $(DIGITARP)
	./$(DIGITARP) > $@

.PHONY:
interactive: $(AMUSE)
	./$(AMUSE)

.PHONY: test
test: ringbuf_test
	./ringbuf_test

$(MRENDER): $(MRENDER_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(DIGITARP): $(DIGITARP_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(AMUSE): $(AMUSE_OBJ) $(SYNTH)
	$(CXX) -o $@ $^ $(SYNTH) $(LDFLAGS)

$(SYNTH): $(SYNTH_OBJ)
	$(AR) cr $@ $^

.PHONY:
real_sound: compo.raw
	aplay -c 1 -r 44100 -f s16_le < compo.raw

%.raw: py/%.py
	PYTHONPATH=py:lib/python py/$*.py > $@

compo.raw: $(FUGE)

.PHONY:
clean:
	@rm -rf build
	@rm -rf $(OBJDIR)
	@rm -f *.gch ringbuf_test
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -rf $(LIBDIR) $(BINDIR)
	@rm -f *.raw *.mp3

ringbuf_test: $(OBJDIR)/ringbuf_test.o
	$(CXX) -o $@ $^ $(GTEST_L) $(LDFLAGS)

ringbuf_test.o: ringbuf_test.cpp
	$(CXX) -c $(GTEST_I) $^

-include $(ALL_OBJECTS:.o=.d)
