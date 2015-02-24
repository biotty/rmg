CFLAGS=-Wextra -Werror -std=c11 -Irgb
LDFLAGS=-lm
RAY_GUN=$(shell pwd)/ray/gun

.PHONY:
all: ray/gun flow/fluid

media: image1.jpeg

heavy_media: video1.mp4 video2.mp4 fluid.mp4 fractzoom.mp4

ray/gun: rgb/librgb.a
	$(MAKE) -C ray

flow/fluid: rgb/librgb.a
	$(MAKE) -C flow fluid

rgb/librgb.a:
	$(MAKE) -C rgb

%.jpeg: ray/gun py/rmg/script.py py/%.py sky.pnm
	py/$*.py -r 1920x1080 -C $(RAY_GUN) 8 $@

sky.pnm: py/lindo.py
	echo "f x()x()x+y f()y()f x-y" | py/lindo.py -b128 -c12 -o $@

%.mp4: ray/gun movie.sh py/%.py sky.pnm
	./movie.sh $@ "py/$*.py -c 736 -C $(RAY_GUN)"

fractzoom.mp4: fract.sh
	py/border.py | ./fract.sh $@

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
	@rm -f *.png *.jpeg *.avi *.mp4 *.pnm
	$(MAKE) -C ray sweep
	$(MAKE) -C flow sweep
