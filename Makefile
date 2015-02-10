include Makefile.inc

CURDIR := $(PWD)
SRCDIR := $(CURDIR)/$(SRC_DIRNAME)
BUILDDIR := $(CURDIR)/$(BUILD_DIRNAME)
INCLUDEDIR := $(CURDIR)/$(INCLUDE_DIRNAME)

INCLUDEPATH = -I. -I$(INCLUDEDIR)
CFLAGS := $(CFLAGS) $(CFLAGS_OPT) $(INCLUDEPATH)

DDPDIR = $(SRCDIR)/$(DDP_DIRNAME)
DDPLIB = $(BUILDDIR)/$(DDPLIB_FILENAME)

PLATFORMDIR = $(SRCDIR)/$(PLATFORM_DIRNAME)
PLATFORMLIB = $(BUILDDIR)/$(PLATFORMLIB_FILENAME)

CLIDIR = $(SRCDIR)/cmdline
CLIEXE = $(BUILDDIR)/ddp

DIRS = $(DDPDIR) $(PLATFORMDIR) $(CLIDIR)
OBJLIBS = $(DDPLIB) $(PLATFORMLIB)
LD_LIBS = -L$(BUILDDIR) -l$(DDP_NAME) -l$(PLATFORM_NAME) -pthread -lrt -lcrypt

OBJECTS = $(SRCDIR)/main.o
EXE = $(BUILDDIR)/ddpd_$(PLATFORM)

SVN_FLAG := `grep DDP_ENGINE_BUILD_NUMBER $(INCLUDEDIR)/version.h`
SRC_LAUNCH_SH = $(PLATFORMDIR)/$(LAUNCH_SH)
BUILD_LAUNCH_SH = $(BUILDDIR)/$(LAUNCH_SH)
LMT_SRC_LAUNCH_SH = `stat -c %Y $(SRC_LAUNCH_SH)`
LMT_BUILD_LAUNCH_SH = `stat -c %Y $(BUILD_LAUNCH_SH)`


all: $(EXE) $(CLIEXE)
	@if [ -e $(BUILD_LAUNCH_SH) ]; then \
		if [ $(LMT_SRC_LAUNCH_SH) -gt $(LMT_BUILD_LAUNCH_SH) ]; then cp $(SRC_LAUNCH_SH) $(BUILD_LAUNCH_SH); fi \
	else cp $(SRC_LAUNCH_SH) $(BUILD_LAUNCH_SH); fi
	@if [ ! -x $(BUILD_LAUNCH_SH) ]; then chmod +x $(BUILD_LAUNCH_SH); fi

$(EXE): $(OBJECTS) $(OBJLIBS)
	$(LD) $(CFLAGS) $(LDFLAGS) -o $(EXE) $(OBJECTS) $(LD_LIBS)

$(OBJECTS):
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(OBJECTS) -c $(SRCDIR)/main.c

$(PLATFORMLIB):
	@mkdir -p $(BUILDDIR)
	cd $(PLATFORMDIR); $(MAKE)

$(CLIEXE):
	@mkdir -p $(BUILDDIR)
	cd $(CLIDIR); $(MAKE)

$(DDPLIB):
	@if [ -z "$(SVN_FLAG)" ]; then build_num=$(shell svnversion . | sed 's/M//g'); echo "#define DDP_ENGINE_BUILD_NUMBER   $$build_num" >> $(INCLUDEDIR)/version.h; fi
	@mkdir -p $(BUILDDIR)
	cd $(DDPDIR); $(MAKE)


clean:
	-$(RM) -f $(EXE) $(OBJECTS) $(OBJLIBS) $(CLIEXE)
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean); done


# Below is script to make sdk package to partner.
# The ddp engine source files are NOT included in sdk.
# The engine is provided in the format of lipddp.a in build directory.
# The toplevel makefile is copied from the one in sdk directory.
SDK_VER = `grep DDP_ENGINE_MAJOR_VERSION $(SDK_VER_FILE) | awk '{print $$3}'`.`grep DDP_ENGINE_MINOR_VERSION $(INCLUDEDIR)/version.h | awk '{print $$3}'`.`svnversion . | sed 's/M//g'`
SDK_DIRNAME = ../ddp-sdk-$(SDK_VER)
SDK_ROOT_DIR = $(CURDIR)/$(SDK_DIRNAME)
SDK_BUILD_DIR = $(SDK_ROOT_DIR)/build
SDK_DOC_DIR = $(SDK_ROOT_DIR)/doc
SDK_INCLUDE_DIR = $(SDK_ROOT_DIR)/$(INCLUDE_DIRNAME)
SDK_SRC_DIR = $(SDK_ROOT_DIR)/src
SDK_MAIN_FILE = $(SDK_SRC_DIR)/main.c
SDK_UNITTEST_DIR = $(SDK_SRC_DIR)/unittest
SDK_PLATFORM_DIR = $(SDK_SRC_DIR)/$(PLATFORM)
SDK_CMDLINE_DIR = $(SDK_SRC_DIR)/cmdline
SDK_VER_FILE = $(INCLUDEDIR)/version.h
SDK_TAR_FILENAME := ddp-sdk-$(SDK_VER).tar.bz2

sdk_pack:
	@if [ -z "$(SVN_FLAG)" ]; then build_num=$(shell svnversion . | sed 's/M//g'); echo "#define DDP_ENGINE_BUILD_NUMBER   $$build_num" >> $(INCLUDEDIR)/version.h; fi
	@echo "Generating SDK package : $(SDK_TAR_FILENAME)"
	@if [ -d $(SDK_ROOT_DIR) ]; then rm -Rf $(SDK_ROOT_DIR); fi
	@mkdir -p $(SDK_ROOT_DIR)
	@mkdir -p $(SDK_BUILD_DIR)
	@if [ -d $(CURDIR)/doc ]; then mkdir -p $(SDK_DOC_DIR); fi
	@mkdir -p $(SDK_INCLUDE_DIR)
	@mkdir -p $(SDK_SRC_DIR)
	@mkdir -p $(SDK_UNITTEST_DIR)
	@mkdir -p $(SDK_PLATFORM_DIR)
	@mkdir -p $(SDK_CMDLINE_DIR)
	@if [ ! -e $(DDPLIB) ]; then make all; fi
	@if [ -e $(DDPLIB) ]; then cp $(DDPLIB) $(SDK_BUILD_DIR); fi
	@if [ -e $(PLATFORMLIB) ]; then cp $(PLATFORMLIB) $(SDK_BUILD_DIR); fi
	@if [ -e $(CURDIR)/build/ddp_vclient ]; then cp $(CURDIR)/build/ddp_vclient $(SDK_BUILD_DIR); fi
	@if [ -d $(PLATFORMDIR) ]; then cp $(PLATFORMDIR)/*.c $(SDK_PLATFORM_DIR); fi
	@if [ -d $(INCLUDEDIR) ]; then cp $(INCLUDEDIR)/*.h $(SDK_INCLUDE_DIR); fi
	@if [ -e $(SRCDIR)/main.c ]; then cp $(SRCDIR)/main.c $(SDK_SRC_DIR); fi
	@if [ -d $(SRCDIR)/unittest ]; then cp $(SRCDIR)/unittest/*.c $(SDK_UNITTEST_DIR); fi
	@if [ -d $(SRCDIR)/unittest ]; then cp $(SRCDIR)/unittest/Makefile $(SDK_UNITTEST_DIR); fi
	@if [ -e $(PLATFORMDIR)/Makefile ]; then cp $(PLATFORMDIR)/Makefile $(SDK_PLATFORM_DIR); fi
	@if [ -e $(PLATFORMDIR)/ddp.sh ]; then cp $(PLATFORMDIR)/ddp.sh $(SDK_PLATFORM_DIR); fi
	@if [ -e $(CLIDIR)/Makefile ]; then cp $(CLIDIR)/Makefile $(SDK_CMDLINE_DIR); fi
	@if [ -e $(CLIDIR)/ddp_command.c ]; then cp $(CLIDIR)/ddp_command.c $(SDK_CMDLINE_DIR); fi
	@if [ -e $(CURDIR)/sdk/Makefile.sdk ]; then cp $(CURDIR)/sdk/Makefile.sdk $(SDK_ROOT_DIR)/Makefile; fi
	@if [ -d $(CURDIR)/doc ]; then cp $(CURDIR)/doc/* $(SDK_DOC_DIR); fi
	@if [ -e $(CURDIR)/Makefile.inc ]; then cp $(CURDIR)/Makefile.inc $(SDK_ROOT_DIR); fi
	@if [ -e $(CURDIR)/Changelog ]; then cp $(CURDIR)/Changelog $(SDK_ROOT_DIR); fi
	@if [ -e $(SDK_ROOT_DIR)/../$(SDK_TAR_FILENAME) ]; then rm -f $(SDK_ROOT_DIR)/../$(SDK_TAR_FILENAME); fi
	@tar -jcv -f $(SDK_ROOT_DIR)/../$(SDK_TAR_FILENAME) $(SDK_ROOT_DIR)


# script to generate full source package
FULLSRC_VER = `grep DDP_ENGINE_MAJOR_VERSION $(FULLSRC_VER_FILE) | awk '{print $$3}'`.`grep DDP_ENGINE_MINOR_VERSION $(INCLUDEDIR)/version.h | awk '{print $$3}'`.`svnversion . | sed 's/M//g'`
FULLSRC_DIRNAME = ../ddp-$(FULLSRC_VER)
FULLSRC_ROOT_DIR = $(CURDIR)/$(FULLSRC_DIRNAME)
FULLSRC_DOC_DIR = $(FULLSRC_ROOT_DIR)/doc
FULLSRC_INCLUDE_DIR = $(FULLSRC_ROOT_DIR)/$(INCLUDE_DIRNAME)
FULLSRC_SRC_DIR = $(FULLSRC_ROOT_DIR)/src
FULLSRC_MAIN_FILE = $(FULLSRC_SRC_DIR)/main.c
FULLSRC_ENGINE_DIR = $(FULLSRC_SRC_DIR)/$(DDP_DIRNAME)
FULLSRC_UNITTEST_DIR = $(FULLSRC_SRC_DIR)/unittest
FULLSRC_PLATFORM_DIR = $(FULLSRC_SRC_DIR)/$(PLATFORM)
FULLSRC_CMDLINE_DIR = $(FULLSRC_SRC_DIR)/cmdline
FULLSRC_VER_FILE = $(INCLUDEDIR)/version.h
FULLSRC_TAR_FILENAME := ddp-$(FULLSRC_VER).tar.bz2

fullsrc_pack:
	@if [ -z "$(SVN_FLAG)" ]; then build_num=$(shell svnversion . | sed 's/M//g'); echo "#define DDP_ENGINE_BUILD_NUMBER   $$build_num" >> $(INCLUDEDIR)/version.h; fi
	@echo "Generate full source package : $(FULLSRC_TAR_FILENAME)"
	@if [ -d $(FULLSRC_ROOT_DIR) ]; then rm -Rf $(FULLSRC_ROOT_DIR); fi
	@mkdir -p $(FULLSRC_ROOT_DIR)
	@if [ -d $(CURDIR)/doc ]; then mkdir -p $(FULLSRC_DOC_DIR); fi
	@mkdir -p $(FULLSRC_INCLUDE_DIR)
	@mkdir -p $(FULLSRC_SRC_DIR)
	@mkdir -p $(FULLSRC_ENGINE_DIR)
	@mkdir -p $(FULLSRC_UNITTEST_DIR)
	@mkdir -p $(FULLSRC_PLATFORM_DIR)
	@mkdir -p $(FULLSRC_CMDLINE_DIR)
	@if [ -d $(PLATFORMDIR) ]; then cp $(PLATFORMDIR)/*.c $(FULLSRC_PLATFORM_DIR); fi
	@if [ -e $(PLATFORMDIR)/Makefile ]; then cp $(PLATFORMDIR)/Makefile $(FULLSRC_PLATFORM_DIR); fi
	@if [ -e $(PLATFORMDIR)/ddp.sh ]; then cp $(PLATFORMDIR)/ddp.sh $(FULLSRC_PLATFORM_DIR); fi
	@if [ -e $(CLIDIR)/Makefile ]; then cp $(CLIDIR)/Makefile $(FULLSRC_CMDLINE_DIR); fi
	@if [ -e $(CLIDIR)/ddp_command.c ]; then cp $(CLIDIR)/ddp_command.c $(FULLSRC_CMDLINE_DIR); fi
	@if [ -d $(SRCDIR)/unittest ]; then cp $(SRCDIR)/unittest/*.c $(FULLSRC_UNITTEST_DIR); fi
	@if [ -d $(SRCDIR)/unittest ]; then cp $(SRCDIR)/unittest/Makefile $(FULLSRC_UNITTEST_DIR); fi
	@if [ -d $(INCLUDEDIR) ]; then cp $(INCLUDEDIR)/*.h $(FULLSRC_INCLUDE_DIR); fi
	@if [ -e $(SRCDIR)/main.c ]; then cp $(SRCDIR)/main.c $(FULLSRC_SRC_DIR); fi
	@if [ -e $(DDPDIR)/Makefile ]; then cp $(DDPDIR)/Makefile $(FULLSRC_ENGINE_DIR); fi
	@if [ -d $(DDPDIR) ]; then cp $(DDPDIR)/*.c $(FULLSRC_ENGINE_DIR); fi
	@if [ -d $(CURDIR)/doc ]; then cp $(CURDIR)/doc/* $(FULLSRC_DOC_DIR); fi
	@if [ -e $(CURDIR)/Makefile.inc ]; then cp $(CURDIR)/Makefile.inc $(FULLSRC_ROOT_DIR); fi
	@if [ -e $(CURDIR)/Makefile ]; then cp $(CURDIR)/Makefile $(FULLSRC_ROOT_DIR); fi
	@if [ -e $(CURDIR)/Changelog ]; then cp $(CURDIR)/Changelog $(FULLSRC_ROOT_DIR); fi
	@if [ -e $(FULLSRC_ROOT_DIR)/../$(FULLSRC_TAR_FILENAME) ]; then rm -f $(FULLSRC_ROOT_DIR)/../$(FULLSRC_TAR_FILENAME); fi
	@tar -jcv -f $(FULLSRC_ROOT_DIR)/../$(FULLSRC_TAR_FILENAME) $(FULLSRC_ROOT_DIR)
