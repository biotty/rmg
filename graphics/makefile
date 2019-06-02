RAYT := $(CURDIR)/ray/rayt
CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3

.PHONY: all media heavy_media clean sweep

all: ray/rayt flow/fluid flow/wave

media: image.jpeg

heavy_media: video.mp4 fluidwave.mp4 mandelbrot.mp4

flow/fluid flow/wave: ray/librayt.a
	@$(MAKE) -C flow $(notdir $@)

ray/rayt ray/demo ray/librayt.a:
	@$(MAKE) -C ray $(notdir $@)

image.jpeg: ray/demo
	$< $@

video.mp4: ray/demo a.movie
	./movie_exec.sh "$< -n $$(ls a.movie/*txt|wc -l) {}/"

fluidwave.mp4: flow/sh flow/fluid flow/wave map.jpeg
	$< map.jpeg $@

follow: follow.cpp
	$(CXX) $(CXXFLAGS) -o $@ -Iray -Iflow $<

mandelbrot.mp4: follow
	./movie_exec.sh "./follow |./movie_pipe.sh {}/" $@

map.jpeg: py/globe.py
	$< 320x200 $@

a.movie: py/atomic.py
	./movie_exec.sh "$< -r 640x360 -x -n 2400 -o {}/" $@/

clean:
	@$(foreach d,ray flow,$(MAKE) $@ -C $(d);)
	@for d in $$(ls -d *.movie);do [ ! -e "$d" ] && rm $d;done
	@py3clean py

sweep: clean
	@$(foreach d,ray flow,$(MAKE) $@ -C $(d);)
	@rm -f follow *.pnm *.png *.jpeg *.mp4
	@ls -d *.movie 2>/dev/null |xargs -I{} ./storage.sh -D {}
