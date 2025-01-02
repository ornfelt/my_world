MAN_DIR = $(BUILDDIR)/usr/share/man

MANPAGES = $(addsuffix .1, $(addprefix $(MAN_DIR)/man1/, $(MAN1))) \
           $(addsuffix .2, $(addprefix $(MAN_DIR)/man2/, $(MAN2))) \
           $(addsuffix .3, $(addprefix $(MAN_DIR)/man3/, $(MAN3))) \
           $(addsuffix .4, $(addprefix $(MAN_DIR)/man4/, $(MAN4))) \
           $(addsuffix .5, $(addprefix $(MAN_DIR)/man5/, $(MAN5))) \
           $(addsuffix .6, $(addprefix $(MAN_DIR)/man6/, $(MAN6))) \
           $(addsuffix .7, $(addprefix $(MAN_DIR)/man7/, $(MAN7))) \
           $(addsuffix .8, $(addprefix $(MAN_DIR)/man8/, $(MAN8))) \
           $(addsuffix .9, $(addprefix $(MAN_DIR)/man9/, $(MAN9))) \

$(MAN_DIR)/man1/%.1: %.1
	@$(MKDIR) -p $(dir $@)
	@$(CP) $< $@

$(MAN_DIR)/man2/%.2: %.2
	@$(MKDIR) -p $(dir $@)
	@$(CP) $< $@

$(MAN_DIR)/man3/%.3: %.3
	@$(MKDIR) -p $(dir $@)
	@$(CP) $< $@

$(MAN_DIR)/man4/%.4: %.4
	@$(MKDIR) -p $(dir $@)
	@$(CP) $< $@

$(MAN_DIR)/man5/%.5: %.5
	@$(MKDIR) -p $(dir $@)
	@$(CP) $< $@

$(MAN_DIR)/man6/%.6: %.6
	@$(MKDIR) -p $(dir $@)
	@$(CP) $< $@

$(MAN_DIR)/man7/%.7: %.7
	@$(MKDIR) -p $(dir $@)
	@$(CP) $< $@

$(MAN_DIR)/man8/%.8: %.8
	@$(MKDIR) -p $(dir $@)
	@$(CP) $< $@

$(MAN_DIR)/man9/%.9: %.9
	@$(MKDIR) -p $(dir $@)
	@$(CP) $< $@

man: $(MANPAGES)

.PHONY: man
