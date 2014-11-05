PRODUCTS=sound graphics artwork
.PHONY: all sound graphics artwork

all: sound graphics artwork

.PHONY:
clean sweep:
	@$(foreach product,$(PRODUCTS),make -C $(product) $@;)

$(PRODUCTS):
	@$(MAKE) -C $@
