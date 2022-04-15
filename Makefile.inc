# This Makefile include is the heart of the PubTools system
# It tries to choose sensible defaults and follow make conventions

# Try to guess the correct number of jobs to launch by counting CPUs or default
# to 2.
MAKEFLAGS+="-j -l $(shell grep -c "^processor[[:space:]]*:[[:space:]]*[[:digit:]]*$" /proc/cpuinfo || echo 2 )"

# Default markdown if not set
# first try pandoc, then multimarkdown fall back to markdown
ifndef MD

ifneq (, $(shell which pandoc))
MD=pandoc
else ifneq (, $(shell which multimarkdown))
MD=multimarkdown
else ifneq (, $(shell which markdown))
MD=markdown
endif

endif

ifeq (, $(shell which $(MD)))
 $(error "Could not find markdown version $(MD), try installing markdown or pandoc, verifying your PATH or updating the Makefile MD processor" )
endif

# If using pandoc without user flags set default flags
ifeq ("pandoc","$(MD)")
 MD_FLAGS?=-f markdown_mmd+task_lists+emoji+autolink_bare_uris -t html --filter pandoc-plantuml
endif

# Sensible defaults for input, output directories, style, and source extensions
OUTDIR?=html
SRC_EXT?= txt md
SRCDIR?=text
STYLE?=defaultStyle

# Search subfolders for resources
SRCDIRS=$(shell find $(SRCDIR) -type d -print)
OUTDIRS=$(patsubst $(SRCDIR)%,$(OUTDIR)%,$(SRCDIRS) )
#SRC=$(foreach EXT,$(SRC_EXT),$(shell find $(SRCDIR) -type f -name "*.$(EXT)" -print))
OUT=$(foreach EXT,$(SRC_EXT),$(patsubst $(SRCDIR)/%.$(EXT),$(OUTDIR)/%.html,$(shell find $(SRCDIR) -type f -name "*.$(EXT)" -print)))


#ifneq ("$(wildcard $(PATH_TO_FILE))","")
STYLEDIR=$(shell find . -maxdepth 1 -type d -name "$(STYLE)" -print| head -n 1)

ifeq (, $(STYLEDIR))
STYLEDIR=$(shell find $(SRCDIR)     -type d -name "$(STYLE)" -print| head -n 1)
endif

ifeq (, $(STYLEDIR))
STYLEDIR=$(shell find . -maxdepth 2 -type d -name "$(STYLE)" -print| head -n 1)
endif

ifeq (, $(STYLEDIR))
STYLEDIR=$(shell find . -maxdepth 3 -type d -name "$(STYLE)" -print| head -n 1)
endif

$(info Using Style Dir at $(STYLEDIR).)

STYLESHEETS=$(shell find $(STYLEDIR) -type f -name "*.css" -print)

OUT+=$(patsubst $(STYLEDIR)/%,$(OUTDIR)/%,$(STYLESHEETS))

$(info build targets $(OUT))

#$(error ALL DONE)


# all is the first and therefore default rule.
all: $(OUT)
	# Done.

.PHONY: all clean clobber install uninstall

$(OUTDIRS):
	mkdir -p $(OUTDIRS)

$(OUTDIR)/%.css: $(STYLEDIR)/%.css |$(OUTDIRS)
	cp $< $@


define convert_rule
$$(OUTDIR)/%.html: $$(SRCDIR)/%.$1 $$(STYLEDIR)/header.html $$(STYLEDIR)/footer.html |$$(OUTDIRS)
	sed "s/MY_TITLE/$$(shell head -n 1 $$<)/" $$(STYLEDIR)/header.html > $$@
	$$(MD) $$(MD_FLAGS) $$< >> $$@
	cat $$(STYLEDIR)/footer.html >> $$@

endef

$(foreach EXT,$(SRC_EXT),$(eval $(call convert_rule,$(EXT))))


clean:
	rm -f $(OUT)
	rm -r $(OUTDIR)

clobber:
	rm -rf $(OUTDIR)

install:
	git config receive.denyCurrentBranch ignore
	ln -sf ../../tools/post-receive .git/hooks/
	.git/hooks/post-receive

uninstall:
	rm .git/hooks/post-receive
