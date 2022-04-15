MAKEFLAGS+="-j -l $(shell grep -c ^processor /proc/cpuinfo)"
#MD=pandoc
MD=multimarkdown
#MD=markdown
ifeq ("pandoc","$(MD)")
# MD_FLAGS=-f markdown_mmd -t html
# MD_FLAGS=-f gfm -t html
 MD_FLAGS=-f markdown -t html
else
 MD_FLAGS=
endif
HDIR=html
SRC_EXT= txt md
SRCDIR=src
SRCDIRS=$(shell find $(SRCDIR) -type d -print)
OUTDIRS=$(patsubst $(SRCDIR)%,$(HDIR)%,$(SRCDIRS) )
#SRC=$(foreach EXT,$(SRC_EXT),$(shell find $(SRCDIR) -type f -name "*.$(EXT)" -print))
STYLE=$(shell find $(SRCDIR) -type f -name "*.css" -print)
OUT=$(foreach EXT,$(SRC_EXT),$(patsubst $(SRCDIR)/%.$(EXT),$(HDIR)/%.html,$(shell find $(SRCDIR) -type f -name "*.$(EXT)" -print)))
OUT+=$(patsubst $(SRCDIR)/%,$(HDIR)/%,$(STYLE))


ifeq (, $(shell which $(MD)))
 $(error "Could not find $(MD), try installing $(MD), verifying your PATH or updating the Makefile markdown processor" )
endif

all: $(OUT)
	# Done.

.PHONY: all clean clobber install uninstall

$(OUTDIRS):
	mkdir -p $(OUTDIRS)

$(HDIR)/%.css: $(SRCDIR)/%.css |$(OUTDIRS)
	cp $< $@


define convert_rule
$$(HDIR)/%.html: $$(SRCDIR)/%.$1 $$(SRCDIR)/header.html $$(SRCDIR)/footer.html |$$(OUTDIRS)
	sed "s/MY_TITLE/$$(shell head -n 1 $$<)/" $$(SRCDIR)/header.html > $$@
	$$(MD) $$(MD_FLAGS) $$< >> $$@
	cat $$(SRCDIR)/footer.html >> $$@

endef

$(foreach EXT,$(SRC_EXT),$(eval $(call convert_rule,$(EXT))))


clean:
	rm -f $(OUT)
	rm -r $(HDIR)

clobber:
	rm -rf $(HDIR)

install:
	git config receive.denyCurrentBranch ignore
	ln -sf ../../tools/post-receive .git/hooks/
	.git/hooks/post-receive

uninstall:
	rm .git/hooks/post-receive
