
include ../../Makefile.inc

CMDLINE_SRCDIR := $(PWD)
CMDLINE_BUILDDIR := $(CMDLINE_SRCDIR)/../../build
INCLUDEPATH = -I . -I $(PWD)/../../$(INCLUDE_DIRNAME)
CFLAGS := $(CFLAGS) $(CFLAGS_OPT) $(INCLUDEPATH)
SRCEXT := c
SOURCES := $(shell find $(CMDLINE_SRCDIR) -type f -name '*.$(SRCEXT)')
OBJECTS := $(patsubst $(CMDLINE_SRCDIR)/%,$(CMDLINE_SRCDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPS := $(OBJECTS:.o=.deps)
TARGET := $(CMDLINE_BUILDDIR)/ddp

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LD_LIBS)

$(CMDLINE_SRCDIR)/%.o: $(CMDLINE_SRCDIR)/%.$(SRCEXT)
	$(CC) $(CFLAGS) $(LDFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	-$(RM) -f $(CMDLINE_SRCDIR)/*.o $(CMDLINE_SRCDIR)/*.deps $(TARGET)

-include $(DEPS)

.PHONY: clean
