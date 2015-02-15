SCIENCES=graphics sound
ACTIONS=all clean media sweep

.PHONY: $(ACTIONS) $(SCIENCES)

$(ACTIONS):
	@$(foreach p,$(SCIENCES),$(MAKE) -C $(p) $@;)
