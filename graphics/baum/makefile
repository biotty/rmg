.PHONY: all media clean sweep

all: feigen

media: feigen.pnm
	# todo: a script generating just a
	#       PNM header of a certain kind
	#       and parameters.  even tho
	#       it's merely an echo, yes

feigen: feigen.o
	ld $^ -o $@

feigen.o: feigen.S
	nasm -f elf64 $^

feigen.pnm:
	gforth > $@ feigen.fs

clean:
	@rm *.o

sweep: clean
	@rm -f feigen *.pnm
