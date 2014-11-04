CXX=c++ -g --std=c++11 -Wall -Werror -fPIC
#CXX=c++ -O2 --std=c++11 -Wall -Werror -fPIC

GTEST_I=-I$(HOME)/gtest/include
GTEST_L=$(HOME)/gtest/lib/gtest_main.a -lpthread

.PHONY: sw
sw: amuse mrender digitarp test

.PHONY: media
media: digitarp.mp3

%.mp3: %.wav
	lame -mm -r -s 44.1 --signed --bitwidth 16 --little-endian $^ $@

digitarp.wav: digitarp
	./digitarp > $@

.PHONY: interactive
interactive: amuse
	./amuse

.PHONY: test
test: ringbuf_test
	./ringbuf_test

mrender: mrender.o midi.o filter.o
	$(CXX) -o $@ $^ -lm

midi.o: midi.hpp samplerate.hpp
filter.o: filter.hpp samplerate.hpp

amuse: amuse.o page.o book.o screen.o orchestra.o speaker.o recorder.o \
  synth.a
	$(CXX) -o $@ $^ -lncurses -lSDL -lm

book.o: book.hpp page.hpp
page.o: page.hpp screen.hpp builders.hpp filters.hpp musicm.hpp
screen.o: screen.cpp screen.hpp orchestra.hpp page.hpp book.hpp
orchestra.o: orchestra.cpp orchestra.hpp generators.hpp
speaker.o: speaker.cpp speaker.hpp ringbuf.hpp
recorder.o: recorder.cpp recorder.hpp

synth.a: unit.o generators.o builders.o envelopes.o filters.o musicm.o
	$(AR) cr $@ $^

unit.o: unit.cpp unit.hpp
generators.o: generators.cpp generators.hpp unit.hpp filters.hpp
builders.o: builders.cpp builders.hpp generators.hpp filters.hpp
envelopes.o: envelopes.cpp envelopes.hpp math.hpp
filters.o: filters.hpp ringbuf.hpp math.hpp

.PHONY: clean
clean:
	rm -f *.[oa] *.gch ringbuf_test; find py -name "*.pyc" -delete

.PHONY: sweep
sweep: clean
	rm -f amuse mrender digitarp *.wav *.mp3 /tmp/*.wav

ringbuf_test: ringbuf_test.o
	$(CXX) -o $@ $^ $(GTEST_L)

ringbuf_test.o: ringbuf_test.cpp
	$(CXX) -c $(GTEST_I) $^
