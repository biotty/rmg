.PHONY:
sw: lib

.PHONY:
media: compo.mp3

compo.mp3: compo.raw
	lame -mm -r -s 44.1 --signed --bitwidth 16 --little-endian $^ $@

.PHONY:
real_sound: compo.raw
	aplay -c 1 -r 44100 -f s16_le < compo.raw

compo.raw: lib standcompo.py
	PYTHONPATH=lib/python ./standcompo.py > $@

lib: setup.py sound_lib
	./setup.py install --home .

.PHONY:
sound_lib:
	$(MAKE) -C ../sound lib/synth.a

.PHONY:
clean:
	@rm -rf build
	@rm -f *.pyc

.PHONY:
sweep: clean
	@rm -rf lib
	@rm -f compo.raw compo.mp3
