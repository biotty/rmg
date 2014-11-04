PRODUCTS=sound graphics artwork
.PHONY: all sound graphics artwork

all: sound graphics artwork

.PHONY:
clean:
	$(foreach product,$(PRODUCTS),make -C $(product) clean;)

$(PRODUCTS):
	$(MAKE) -C $@
