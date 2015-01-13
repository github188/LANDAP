
include ../../Makefile.inc

ENGINE_SRCDIR := $(PWD)
ENGINE_BUILDDIR := $(ENGINE_SRCDIR)/../../build
INCLUDEPATH = -I . -I $(PWD)/../../$(INCLUDE_DIRNAME)
CFLAGS := $(CFLAGS) $(CFLAGS_OPT) $(INCLUDEPATH) -D_GNU_SOURCE
SRCEXT := c
SOURCES := $(shell find $(ENGINE_SRCDIR) -type f -name '*.$(SRCEXT)')
OBJECTS := $(patsubst $(ENGINE_SRCDIR)/%,$(ENGINE_SRCDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPS := $(OBJECTS:.o=.deps)
TARGET := $(ENGINE_BUILDDIR)/$(DDPLIB_FILENAME)


$(TARGET): $(OBJECTS)
	$(AR) -cvq $(TARGET) $(OBJECTS)

$(ENGINE_SRCDIR)/%.o: $(ENGINE_SRCDIR)/%.$(SRCEXT)
	$(CC) $(CFLAGS) $(LDFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	-$(RM) -f $(ENGINE_SRCDIR)/*.o $(ENGINE_SRCDIR)/*.deps $(TARGET)

-include $(DEPS)

.PHONY: clean
