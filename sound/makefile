CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3 -Isynth
LAME_S16 := -mm -r -s 44.1 --signed --bitwidth 16 --little-endian

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
	@ln -f $< $@

%.raw: py/%.py py/fuge.so
	py/$*.py > $@

digitarp.raw: digitarp
	./digitarp > $@

%.mp3: %.raw
	lame $(LAME_S16) $< $@

clean:
	@$(foreach d,synth fuge amuse mrender,$(MAKE) $@ -C $(d);)
	@rm -f *.o
	@py3clean .

sweep: clean
	@$(foreach d,synth fuge amuse mrender,$(MAKE) $@ -C $(d);)
	@rm -f digitarp py/fuge.so *.raw *.mp3
