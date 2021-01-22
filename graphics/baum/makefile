CXXFLAGS := -Wall -Wextra -Werror -std=c++14 -pedantic -O3

.PHONY: all media heavy_media clean sweep sweep_heavy

all: feigen

media: feigen.pnm

heavy_media: feigen
	./script.py

feigen-ref.jpeg: feigen.fs
	gforth $< |pnmtojpeg >$@

feigen-femto: feigen-femto.o
	ld $^ -o $@

.S.o:
	nasm -f elf64 $^

feigen: feigen.cpp feigen-proc.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lm # -lpthread

extract: extract.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	@rm -f *.o a.out core*

sweep: clean
	@rm -f feigen feigen-femto *.jpeg

heavy_sweep: sweep
	@rm -rf y[0-9]*
