TEX=latexmk -silent
# on-need: when generating figures or tables to be included,
#          -use-make and have rules for those files here

.PHONY: all clean sweep degarb

all: optic.pdf sound.pdf fluid.pdf

%.pdf: %.tex
	$(TEX) -pdf $<
	@touch $@  # note: remember latex-make was satisfied

# seems general rule, appending %.bib above is not effective
optic.pdf: optic.bib
sound.pdf: sound.bib
fluid.pdf: fluid.bib

clean: degarb
	@$(TEX) -c

sweep: degarb
	@$(TEX) -C

degarb:
	@rm -f *.bbl *.bak
