CXXFLAGS=-Wall -Wextra -Werror -O2 -std=c++11 -Isynth
LAME_S16=-mm -r -s 44.1 --signed --bitwidth 16 --little-endian

.PHONY: all media clean sweep

all: digitarp mrender/mrender amuse/amuse py/fuge.so

media: digitarp.mp3 demo.mp3

digitarp: digitarp.o musicm.o
	$(CXX) -o $@ $^

musicm.o: synth/musicm.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

mrender/mrender:
	@$(MAKE) -C mrender

synth/sizr.a:
	@$(MAKE) -C synth

amuse/amuse: synth/sizr.a
	@$(MAKE) -C amuse

fuge/fuge.so: synth/sizr.a
	@$(MAKE) -C fuge

py/fuge.so: fuge/fuge.so
	@ln -f $^ $@

%.raw: py/%.py py/fuge.so
	py/$*.py > $@

digitarp.raw: digitarp
	./digitarp > $@

%.mp3: %.raw
	lame $(LAME_S16) $^ $@

clean:
	@find py -name "*.pyc" -delete

sweep: clean
	@rm -f digitarp
	@rm -f py/fuge.so
	@rm -f *.raw *.mp3
	@$(MAKE) -C mrender $@
	@$(MAKE) -C synth $@
	@$(MAKE) -C amuse $@
	@$(MAKE) -C fuge $@
