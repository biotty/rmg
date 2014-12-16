SUBDIRS=sound graphics
.PHONY: all $(SUBDIRS)
all: $(SUBDIRS)

.PHONY:
clean sweep:
	@$(foreach product,$(SUBDIRS),make -C $(product) $@;)

$(SUBDIRS):
	@$(MAKE) -C $@
