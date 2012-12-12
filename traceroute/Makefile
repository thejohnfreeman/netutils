SRCDIR := src
SOURCES := \
	main.c

OBJDIR := build
OBJECTS := $(SOURCES:%.c=$(OBJDIR)/$(SRCDIR)/%.o)

INSTALLDIR := bin
INSTALLDIR := $(abspath $(INSTALLDIR))

EXE := traceroute
EXE := $(INSTALLDIR)/lib$(EXE).a

INCPATHS := ../jfnet/include

LIBPATHS := ../jfnet/lib
LIBS := jfnet


## default target

.PHONY : all

all : $(EXE)


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

FLAGS += $(foreach INCPATH, $(INCPATHS), -I$(INCPATH))
#FLAGS += -O2 -DNDEBUG
FLAGS += -O0 -g3

CFLAGS += $(FLAGS)
CFLAGS += -std=c11

LFLAGS += $(foreach LIBPATH, $(LIBPATHS), -L$(LIBPATH))
LFLAGS += $(foreach LIB, $(LIBS), -l$(LIB))


## components

$(OBJECTS) : $(OBJDIR)/$(SRCDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	@$(call dep_c,$@,$<)
	$(CC) $(CFLAGS) -o $@ -c $<


## target

$(EXE) : $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^


## testing

.PHONY : test

test :
	$(abspath $(EXE)) google.com


## cleaning

.PHONY : clean

clean :
	rm -rf $(OBJDIR)
	rm -rf $(INSTALLDIR)
