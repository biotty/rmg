RAY_GUN=$(CURDIR)/ray/gun
CXXFLAGS=-Wall -Wextra -Werror -std=c++14 -pedantic -O2

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
	$< |$(RAY_GUN) 1920x1080 |pnmtojpeg >$@

video.mp4: py/image1.py ray/gun map.jpeg map.movie
	./movie_exec.sh $@ "$< -n 500 -j 2 -C $(RAY_GUN) -o {}/"

fluidwave.mp4: flow/sh flow/fluid flow/wave map.jpeg
	$< map.jpeg $@

follow: follow.cpp
	$(CXX) $(CXXFLAGS) -o $@ -Irgb -Iflow $<

mandelbrot.mp4: follow
	./movie_exec.sh $@ "./follow |./movie_pipe.sh {}/"

map.jpeg: py/globe.py
	$< 32x32 $@

map.movie: py/atomic.py
	./movie_exec.sh $@/ "$< -n 500 -o {}/"

clean:
	@$(foreach d,ray flow rgb,$(MAKE) $@ -C $(d);)
	@py3clean py

sweep: clean
	@$(foreach d,ray flow rgb,$(MAKE) $@ -C $(d);)
	@rm -f follow *.pnm *.png *.jpeg *.mp4
	@ls -d *.movie 2>/dev/null |xargs -I{} ./storage.sh -D {}
