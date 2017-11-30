RAY_GUN=$(CURDIR)/ray/gun
CXXFLAGS=-Wall -Wextra -Werror -std=c++11 -O2

.PHONY: all media heavy_media clean sweep

all: ray/gun flow/fluid flow/wave

media: image.jpeg

heavy_media: video.mp4 fluid.mp4 mandelbrot.mp4

ray/gun: rgb/librgb.a
	@$(MAKE) -C ray

flow/fluid flow/wave: rgb/librgb.a
	@$(MAKE) -C flow $(notdir $@)

rgb/librgb.a:
	@$(MAKE) -C rgb

image.jpeg: py/image1.py ray/gun map.jpeg
	$< 12 |$(RAY_GUN) 1920x1080 |pnmtojpeg >$@

video.mp4: py/image1.py ray/gun map.jpeg
	./movie_from_exec.sh $@ "$< -n 736 -C $(RAY_GUN)"

fluidwave.mp4: flow/fluid flow/wave
	flow/sh

follow: follow.cpp
	$(CXX) $(CXXFLAGS) -o $@ -Irgb -Iflow $<

mandelbrot.mp4: follow
	./follow |./movie_from_pipe.sh $@

map.jpeg: py/globe.py
	$< 512x512 && pnmtojpeg globe.pnm > $@

clean:
	@find py -name "*.pyc" -delete

sweep: clean
	@rm -f follow globe.pnm *.png *.jpeg *.mp4
	@ls -d *.movie 2>/dev/null |xargs -i ./movie_directory.sh -D {}
	@$(MAKE) -C rgb sweep
	@$(MAKE) -C ray sweep
	@$(MAKE) -C flow sweep
