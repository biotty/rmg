CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3

.PHONY: all media clean sweep

all: feigen

media: feigen.pnm

# todo: a script generating just a
#       PNM header of a certain kind
#       and parameters.  even tho
#       it's merely an echo, yes
#       or rather, the tool can extract from
#       various raw formats depending on
#       file extension, and also from a
#       grid of images

feigen-ref.pnm: feigen.fs
	gforth > $@ $<

feigen-femto: feigen-femto.o
	ld $^ -o $@

.S.o:
	nasm -f elf64 $^

feigen: feigen.cpp feigen-proc.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lpthread

clean:
	@rm *.o

sweep: clean
	@rm -f feigen *.pnm

sweep_heavy: sweep
	@rm -rf grid
