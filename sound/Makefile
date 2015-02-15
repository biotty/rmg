CXXFLAGS=-Wextra -Werror -std=c++11

.PHONY:
all: digitarp mrender/mrender amuse/amuse py/fuge.so

.PHONY:
media: digitarp.mp3 compo.mp3

digitarp: digitarp.o
	$(CXX) -o $@ $^ $(LDFLAGS)

mrender/mrender:
	$(MAKE) -C mrender

synth/sizr.a:
	$(MAKE) -C synth

amuse/amuse: synth/sizr.a
	$(MAKE) -C amuse

py/fuge.so: synth/sizr.a
	./setup.py install --home .
	ln lib/python/fuge.so py

%.mp3: %.raw
	lame -mm -r -s 44.1 --signed --bitwidth 16 --little-endian $^ $@

digitarp.raw: digitarp
	./digitarp > $@

%.raw: py/%.py py/fuge.so
	PYTHONPATH=py py/$*.py > $@

.PHONY:
clean:
	@rm -f *.o
	@rm -fr build lib
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -f digitarp
	@rm -f py/fuge.so
	@rm -f *.raw *.mp3
	$(MAKE) -C mrender $@
	$(MAKE) -C synth $@
	$(MAKE) -C amuse $@
