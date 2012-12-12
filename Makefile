PROJECTS := \
	jfnet \
	ping \
	traceroute

# $1 = target
defer = \
	@for proj in $(PROJECTS); do \
		$(MAKE) -C $$proj $1; \
	done

## targets

TARGETS := all test clean

.PHONY : $(TARGETS)

$(TARGETS) : % :
	$(call defer, $@)

