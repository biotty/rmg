BINDIR=bin
OBJDIR=obj
LIBDIR=lib

CFLAGS = -Wall -std=c99 -MMD -MP -fPIC `sdl2-config --cflags` -I ./include
CXXFLAGS = -Wall -std=c++11 -MMD -MP -fPIC `sdl2-config --cflags` -I ./include
LDFLAGS=`sdl2-config --libs` -lncurses -lm

GTEST_I=-I$(HOME)/gtest/include
GTEST_L=$(HOME)/gtest/lib/gtest_main.a -lpthread

VPATH = src

RENDER_OBJ=\
	   $(OBJDIR)/mrender.o \
	   $(OBJDIR)/midi.o \
	   $(OBJDIR)/filter.o

AMUSE_OBJ=\
	  $(OBJDIR)/amuse.o \
	  $(OBJDIR)/page.o \
	  $(OBJDIR)/book.o \
	  $(OBJDIR)/screen.o \
	  $(OBJDIR)/orchestra.o \
	  $(OBJDIR)/speaker.o \
	  $(OBJDIR)/speaker.o \
	  $(OBJDIR)/recorder.o

SYNTH_OBJ=\
	  $(OBJDIR)/unit.o \
	  $(OBJDIR)/generators.o \
	  $(OBJDIR)/builders.o \
	  $(OBJDIR)/envelopes.o \
	  $(OBJDIR)/filter.o \
	  $(OBJDIR)/filters.o \
	  $(OBJDIR)/musicm.o

ALL_OBJECTS=\
	$(RENDER_OBJ) \
	$(AMUSE_OBJ) \
	$(SYNTH_OBJ) \

MRENDER=$(BINDIR)/mrender
AMUSE=$(BINDIR)/amuse
DIGITARP=$(BINDIR)/digitarp

all: dirs $(AMUSE) $(MRENDER)

$(OBJDIR)/%.o: %.c Makefile $(AMUSE) render
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o: %.cpp Makefile
	$(CXX) -c -o $@ $< $(CXXFLAGS)


.PHONY:
dirs:
	@mkdir -p $(BINDIR) $(OBJDIR) $(LIBDIR)

.PHONY: sw
sw: $(AMUSE) $(MRENDER) $(DIGITARP)

.PHONY: media
media: digitarp.mp3

%.mp3: %.wav
	lame -mm -r -s 44.1 --signed --bitwidth 16 --little-endian $^ $@

digitarp.wav: $(DIGITARP)
	./$(DIGITARP) > $@

.PHONY: interactive
interactive: $(AMUSE)
	./$(AMUSE)

.PHONY: test
test: ringbuf_test
	./ringbuf_test

$(MRENDER): $(RENDER_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(AMUSE): $(AMUSE_OBJ) $(LIBDIR)/synth.a
	$(CXX) -o $@ $^ $(LDFLAGS)

$(LIBDIR)/synth.a: $(SYNTH_OBJ)
	$(AR) cr $@ $^

.PHONY: clean
clean:
	@rm -vf \
	    $(OBJDIR)/* \
	    $(LIBDIR)/* \
	    $(BINDIR)/* \
	      *.gch ringbuf_test
	find py -name "*.pyc" -delete

.PHONY: sweep
sweep: clean
	@rm -vf \
	    $(BINDIR)/*
	    *.wav *.mp3 /tmp/*.wav

ringbuf_test: $(OBJDIR)/ringbuf_test.o
	$(CXX) -o $@ $^ $(GTEST_L) $(LDFLAGS)

ringbuf_test.o: ringbuf_test.cpp
	$(CXX) -c $(GTEST_I) $^

-include $(ALL_OBJECTS:.o=.d)
