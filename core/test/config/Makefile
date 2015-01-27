NAME = TestSystem
LIBDIR = /var/testfarm/lib

CONFIG = $(NAME).xml $(NAME).pm
TOOLS = echo.pl Echo.pm

SRCDIR = $(PWD)

all: $(NAME).pm

install: $(CONFIG) $(TOOLS)
	ln -sf $(^:%=$(SRCDIR)/%) $(LIBDIR)

clean:
	$(RM) $(NAME).pm *~ *.bak

$(NAME).pm: $(NAME).xml
	twiz-conf $<
