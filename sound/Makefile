BINDIR=bin
OBJDIR=obj
LIBDIR=lib

CXXFLAGS=-Wall -Wextra -std=c++11 -MMD -MP -fPIC `sdl2-config --cflags` -I ./include
LDFLAGS=`sdl2-config --libs` -lncurses -lm

GTEST_I=-I$(HOME)/gtest/include
GTEST_L=$(HOME)/gtest/lib/gtest_main.a -lpthread

VPATH = src

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

.PHONY:
all: $(AMUSE) $(DIGITARP) $(MRENDER)

$(OBJDIR) $(LIBDIR) $(BINDIR):
	@mkdir -p $@

$(OBJDIR)/%.o: %.cpp Makefile $(OBJDIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

.PHONY:
media: out.mp3

%.mp3: %.wav
	lame -mm -r -s 44.1 --signed --bitwidth 16 --little-endian $^ $@

out.wav: $(DIGITARP)
	./$(DIGITARP) > $@

.PHONY:
interactive: $(AMUSE)
	./$(AMUSE)

.PHONY: test
test: ringbuf_test
	./ringbuf_test

$(MRENDER): $(BINDIR) $(MRENDER_OBJ)
	$(CXX) -o $@ $(MRENDER_OBJ) $(LDFLAGS)

$(DIGITARP): $(BINDIR) $(DIGITARP_OBJ)
	$(CXX) -o $@ $(DIGITARP_OBJ) $(LDFLAGS)

$(AMUSE): $(BINDIR) $(AMUSE_OBJ) $(SYNTH)
	$(CXX) -o $@ $(AMUSE_OBJ) $(SYNTH) $(LDFLAGS)

$(SYNTH): $(LIBDIR) $(SYNTH_OBJ)
	$(AR) cr $@ $(SYNTH_OBJ)

.PHONY:
clean:
	@rm -rf $(OBJDIR)
	@rm -f *.gch ringbuf_test
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -rf $(LIBDIR) $(BINDIR)
	@rm -f *.wav *.mp3

ringbuf_test: $(OBJDIR)/ringbuf_test.o
	$(CXX) -o $@ $^ $(GTEST_L) $(LDFLAGS)

ringbuf_test.o: ringbuf_test.cpp
	$(CXX) -c $(GTEST_I) $^

-include $(ALL_OBJECTS:.o=.d)
