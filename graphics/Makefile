RAY_GUN=$(CURDIR)/ray/gun
FLOW_FLUID=$(CURDIR)/flow/fluid

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

image.jpeg: py/image1.py ray/gun
	$< 12 |$(RAY_GUN) 1920x1080 |pnmtojpeg >$@

example.avi: py/video1.py ray/gun
	./movie_from_exec.sh $@ "$< -c 736 -C $(RAY_GUN)"

fluid.mp4: img.png flow/fluid
	./movie_from_exec.sh $@ "$(FLOW_FLUID)"

mandelbrot.mp4: py/follow.py
	$< |./movie_from_pipe.sh $@

img.png: py/globe.py
	$< 192x108
	pnmtopng globe.pnm >$@

.PHONY:
clean:
	@find py -name "*.pyc" -delete

.PHONY:
sweep: clean
	@rm -f *.png *.jpeg *.avi *.mp4 *.pnm
	@ls -d *.movie 2>/dev/null |xargs -i ./movie_directory.sh -D {}
	@$(MAKE) -C rgb sweep
	@$(MAKE) -C ray sweep
	@$(MAKE) -C flow sweep
