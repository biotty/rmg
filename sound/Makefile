MOD_INCS=`python-config --includes` -Isynth
CXXFLAGS=-Wextra -Werror -std=c++11
LAME_S16=-mm -r -s 44.1 --signed --bitwidth 16 --little-endian

.PHONY:
all: digitarp mrender/mrender amuse/amuse py/fuge.so

.PHONY:
media: digitarp.mp3 compo.mp3

mrender/mrender:
	$(MAKE) -C mrender

synth/sizr.a:
	$(MAKE) -C synth

amuse/amuse: synth/sizr.a
	$(MAKE) -C amuse

py/fuge.so: fugemodule.o synth/sizr.a
	$(CXX) -o $@ $^ -shared

fugemodule.o: fugemodule.cpp
	$(CXX) -c $^ $(CXXFLAGS) $(MOD_INCS) -fPIC

%.mp3: %.raw
	lame $(LAME_S16) $^ $@

digitarp.raw: digitarp
	./digitarp > $@

%.raw: py/%.py py/fuge.so
	py/$*.py > $@

.PHONY:
clean:
	@rm -f *.o
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -f digitarp
	@rm -f py/fuge.so
	@rm -f *.raw *.mp3
	$(MAKE) -C mrender $@
	$(MAKE) -C synth $@
	$(MAKE) -C amuse $@
