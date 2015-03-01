CFLAGS=-Wextra -Werror -std=c11 -Irgb
LDFLAGS=-lm
RAY_GUN=$(shell pwd)/ray/gun
FLOW_FLUID=$(shell pwd)/flow/fluid

.PHONY:
all: ray/gun flow/fluid

media: image.jpeg

heavy_media: example.avi fluid.mp4 mandelbrot.mp4

ray/gun: rgb/librgb.a
	@$(MAKE) -C ray

flow/fluid: rgb/librgb.a
	@$(MAKE) -C flow fluid

rgb/librgb.a:
	@$(MAKE) -C rgb

image.jpeg: py/image1.py sky.pnm ray/gun
	$< -r 1920x1080 -C $(RAY_GUN) 12 $@

example.avi: py/video1.py sky.pnm ray/gun
	./movie_from_exec.sh $@ "$< -c 736 -C $(RAY_GUN)"

fluid.mp4: img.png flow/fluid
	./movie_from_exec.sh $@ "$(FLOW_FLUID)"

mandelbrot.mp4: py/follow.py
	$< | ./movie_from_pipe.sh $@

sky.pnm: py/lindo.py py/rmg/ls.py
	$< -b128 -c12 -e "f x()x()x+y f()y()f x-y" -o $@

img.png: py/globe.py
	$< 1920x1080
	pnmtopng globe.pnm > $@

.PHONY:
clean:
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -f *.png *.jpeg *.avi *.mp4 *.pnm
	@ls -d *.movie 2>/dev/null | xargs -i ./movie_directory.sh -D {}
	@$(MAKE) -C ray sweep
	@$(MAKE) -C flow sweep
