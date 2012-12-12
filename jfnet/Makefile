SRCDIR := src
SOURCES := \
	io.c \
	ip.c \
	sock.c \
	icmp.c \
	inet.c \

OBJDIR := build
OBJECTS := $(SOURCES:%.c=$(OBJDIR)/$(SRCDIR)/%.o)

INSTALLDIR := lib

LIB := jfnet
LIB := $(INSTALLDIR)/lib$(LIB).a

## default target

.PHONY : all

all : $(LIB)


## dependencies

include $(shell find $(OBJDIR) -name '*.make' 2>/dev/null)

# $1 = target ($@)
# $2 = source ($<)
# $3 = extra flags
dep_c = \
	DEP=$1.make; \
	  $(CC) $(CFLAGS) $3 -o $$DEP -MM -MT '$1' $2; \
	  cat $$DEP | sed -e 's!/usr[^[:space:]]*!!g' \
	    -e '/^[:space:]*\\$$/ d' \
	    > $$DEP.tmp && mv $$DEP.tmp $$DEP


## compiler

CC  := clang

FLAGS += -I./include
#FLAGS += -O2 -DNDEBUG
FLAGS += -O0 -g3

CFLAGS += $(FLAGS)
CFLAGS += -std=c11


## library

$(OBJECTS) : $(OBJDIR)/$(SRCDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	@$(call dep_c,$@,$<)
	$(CC) $(CFLAGS) -o $@ -c $<

$(LIB) : $(OBJECTS)
	@-mkdir -p $(dir $@)
	ar -rcs $@ $^


## cleaning

.PHONY : clean

clean :
	rm -rf $(OBJDIR)
	rm -rf $(INSTALLDIR)

