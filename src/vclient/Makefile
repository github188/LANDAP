
include ../../Makefile.inc

VCLIENT_SRCDIR := $(PWD)
VCLIENT_BUILDDIR := $(VCLIENT_SRCDIR)/../../build
INCLUDEPATH = -I . -I $(PWD)/../../$(INCLUDE_DIRNAME)
CFLAGS := $(CFLAGS) $(CFLAGS_OPT) $(INCLUDEPATH)
SRCEXT := c
SOURCES := $(shell find $(VCLIENT_SRCDIR) -type f -name '*.$(SRCEXT)')
OBJECTS := $(patsubst $(VCLIENT_SRCDIR)/%,$(VCLIENT_SRCDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPS := $(OBJECTS:.o=.deps)
TARGET := $(VCLIENT_BUILDDIR)/$(PLATFORMLIB_FILENAME)


$(TARGET): $(OBJECTS)
	$(AR) -cvq $(TARGET) $(OBJECTS)

$(VCLIENT_SRCDIR)/%.o: $(VCLIENT_SRCDIR)/%.$(SRCEXT)
	$(CC) $(CFLAGS) $(LDFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	-$(RM) -f $(VCLIENT_SRCDIR)/*.o $(VCLIENT_SRCDIR)/*.deps $(TARGET)

-include $(DEPS)

.PHONY: clean
