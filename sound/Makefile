CXXFLAGS=-Wextra -Werror -std=c++11
LAME_S16=-mm -r -s 44.1 --signed --bitwidth 16 --little-endian

.PHONY:
all: digitarp mrender/mrender amuse/amuse py/fuge.so

.PHONY:
media: digitarp.mp3 compo.mp3

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

.PHONY:
clean:
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -f digitarp
	@rm -f py/fuge.so
	@rm -f *.raw *.mp3
	@$(MAKE) -C mrender $@
	@$(MAKE) -C synth $@
	@$(MAKE) -C amuse $@
	@$(MAKE) -C fuge $@
