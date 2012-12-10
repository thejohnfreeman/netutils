INCDIR := include

BINS := \
	ping
INSTALLDIR := bin
BINS := $(addprefix $(INSTALLDIR)/,$(BINS))

CC := clang
CFLAGS += -I$(INCDIR) 

.PHONY : all

all : $(BINS)

$(BINS) : $(INSTALLDIR)/% : %/main.c
	@-mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

