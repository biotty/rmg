PRODUCTS=sound graphics
.PHONY: all sound graphics

all: sound graphics

.PHONY:
clean sweep:
	@$(foreach product,$(PRODUCTS),make -C $(product) $@;)

$(PRODUCTS):
	@$(MAKE) -C $@
