SCIENCES=sound graphics
ACTIONS=all clean media sweep

.PHONY:
$(ACTIONS):
	@$(foreach p,$(SCIENCES),$(MAKE) -C $(p) $@;)
