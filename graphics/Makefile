CFLAGS=-Wextra -Werror -std=c11 -Irgb
LDFLAGS=-lm

.PHONY:
all: ray/gun flow/fluid

media: image1.jpeg video1.mp4 video2.mp4 fluid.mp4 fractzoom.mp4

ray/gun: rgb/librgb.a
	$(MAKE) -C ray

flow/fluid: rgb/librgb.a
	$(MAKE) -C flow fluid

rgb/librgb.a:
	$(MAKE) -C rgb

%.world: py/%.py py/rmg/scene.py
	py/$*.py 16 > $@

%.jpeg: ray/gun %.world sky.pnm
	ray/gun 1920x1080 $@ < $*.world

sky.pnm: py/lsr.py
	echo "f x()x()x+y f()y()f x-y" | py/lsr.py -b128 -c12 512x512 > $@

%.mp4: ray/gun movie.sh py/%.py sky.pnm
	./movie.sh $@ 'py/$*.py 736 "$(PWD)/ray/gun 1280x720"'

fractzoom.mp4: fract.sh
	py/fz.py | ./fract.sh $@

fluid.mp4: img.png fluid cmd.sh
	flow/fluid -p /tmp/ -i img.png
	./cmd.sh /tmp/ $@

img.png: py/globe.py
	py/globe.py 512x512
	pnmtopng globe.pnm > $@

.PHONY:
clean:
	@rm -rf movie.d fract.d *.o
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -f *.world *.png *.jpeg *.avi *.mp4 *.pnm
	$(MAKE) -C ray sweep
	$(MAKE) -C flow sweep
