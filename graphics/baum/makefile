CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3

.PHONY: all media heavy_media clean sweep sweep_heavy

all: feigen feigen-femto

media: feigen.pnm

heavy_media: genfei.d

genfei.d: feigen genfei.py
	./genfei.py

feigen-ref.jpeg: feigen.fs
	gforth $< |pnmtojpeg >$@

feigen-femto: feigen-femto.o
	ld $^ -o $@

.S.o:
	nasm -f elf64 $^

feigen: feigen.cpp feigen-proc.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lm # -lpthread

clean:
	@rm -f *.o a.out core*

sweep: clean
	@rm -f feigen feigen-femto *.jpeg

heavy_sweep: sweep
	@rm -rf genfei.d
