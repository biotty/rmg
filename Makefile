.PHONY: all sound graphics artwork

all: sound graphics artwork

sound graphics artwork:
	$(MAKE) -C $@
