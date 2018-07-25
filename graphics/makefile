RAYT := $(CURDIR)/ray/rayt
CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3

.PHONY: all media heavy_media clean sweep

all: ray/rayt flow/fluid flow/wave

media: image.jpeg

heavy_media: video.mp4 fluid.mp4 mandelbrot.mp4

flow/fluid flow/wave: ray/librayt.a
	@$(MAKE) -C flow $(notdir $@)

ray/rayt ray/librayt.a:
	@$(MAKE) -C ray

image.jpeg: py/image1.py ray/rayt map.jpeg
	$< |$(RAYT) 1920x1080 |pnmtojpeg >$@

video.mp4: py/image1.py ray/gun map.jpeg map.movie
	./movie_exec.sh $@ "$< -n 500 -j 2 -C $(RAYT) -o {}/"

fluidwave.mp4: flow/sh flow/fluid flow/wave map.jpeg
	$< map.jpeg $@

follow: follow.cpp
	$(CXX) $(CXXFLAGS) -o $@ -Iray -Iflow $<

mandelbrot.mp4: follow
	./movie_exec.sh $@ "./follow |./movie_pipe.sh {}/"

map.jpeg: py/globe.py
	$< 320x200 $@

map.movie: py/atomic.py
	./movie_exec.sh $@/ "$< -n 500 -o {}/"

clean:
	@$(foreach d,ray flow,$(MAKE) $@ -C $(d);)
	@py3clean py

sweep: clean
	@$(foreach d,ray flow,$(MAKE) $@ -C $(d);)
	@rm -f follow *.pnm *.png *.jpeg *.mp4
	@ls -d *.movie 2>/dev/null |xargs -I{} ./storage.sh -D {}
