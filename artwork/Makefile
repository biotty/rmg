.PHONY: sw
sw: lib

.PHONY: media
media: compo.mp3

compo.mp3: compo.raw
	lame -mm -r -s 44.1 --signed --bitwidth 16 --little-endian $^ $@

.PHONY: real_sound
real_sound: compo.raw
	aplay -c 1 -r 44100 -f s16_le < compo.raw

compo.raw: lib standcompo.py
	PYTHONPATH=lib/python python standcompo.py > $@

lib: setup.py sound_sw
	python setup.py install --home .

.PHONY: sound_sw
sound_sw:
	$(MAKE) -C ../sound sw

.PHONY: clean sweep
clean:
	rm -rf build
	rm -f *.pyc

sweep: clean
	rm -rf lib
	rm -f compo.raw compo.mp3
