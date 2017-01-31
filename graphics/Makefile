RAY_GUN=$(CURDIR)/ray/gun
FLOW_FLUID=$(CURDIR)/flow/fluid
CXXFLAGS=-Wall -Wextra -Werror -std=c++11 -O2

.PHONY:
all: ray/gun flow/fluid

media: image.jpeg

heavy_media: video.mp4 fluid.mp4 mandelbrot.mp4

ray/gun: rgb/librgb.a
	@$(MAKE) -C ray

flow/fluid: rgb/librgb.a
	@$(MAKE) -C flow fluid

follow: follow.cpp
	$(CXX) $(CXXFLAGS) -o $@ -Irgb -Iflow $<

rgb/librgb.a:
	@$(MAKE) -C rgb

image.jpeg: py/image1.py ray/gun
	$< 12 |$(RAY_GUN) 1920x1080 |pnmtojpeg >$@

video.mp4: py/image1.py ray/gun
	./movie_from_exec.sh $@ "$< -n 736 -C $(RAY_GUN)"

fluid.mp4: flow/fluid img.png
	./movie_from_exec.sh $@ "$(FLOW_FLUID) -n 2048 1920x1080"

mandelbrot.mp4: follow
	./follow |./movie_from_pipe.sh $@

img.png: py/globe.py
	$< 1280x720
	pnmtopng globe.pnm >$@

.PHONY:
clean:
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -f follow globe.pnm *.png *.jpeg *.mp4
	@ls -d *.movie 2>/dev/null |xargs -i ./movie_directory.sh -D {}
	@$(MAKE) -C rgb sweep
	@$(MAKE) -C ray sweep
	@$(MAKE) -C flow sweep
