
.NOTPARALLEL:

ROOT=../..
MK.pyver:=3
include $(ROOT)/deps/readies/mk/main

MK.configure=1

define HELPTEXT
make build    # configure and compile
make clean    # clean generated sbinaries
  ALL=1         # remote entire binary directory
  FORCE=1       # clean non-git files from source dir (use with caution!)
make source   # fetch sources and generate configure script
endef

#----------------------------------------------------------------------------------------------

MK_ALL_TARGETS=bindirs build

BINDIR=$(BINROOT)/libcypher-parser
SRCDIR=$(ROOT)/deps/libcypher-parser

TARGET=$(BINDIR)/lib/src/.libs/libcypher-parser.a

export CFLAGS += \
	-fPIC \
	-O3 \
	-DYY_BUFFER_SIZE=1048576 \
	-I$(BINDIR)/lib/src \
	$(CFLAGS.$(OS))

CFLAGS.macos += -mmacosx-version-min=10.15

#----------------------------------------------------------------------------------------------

MK_CUSTOM_CLEAN=1

include $(MK)/defs
include $(MK)/rules

#----------------------------------------------------------------------------------------------

source: $(SRCDIR)/configure

.PHONY: source

$(SRCDIR)/configure: $(SRCDIR)/configure.ac
	$(SHOW)cd $(SRCDIR); ./autogen.sh

clean:
if ($(ALL),1)
	$(SHOW)-rm -rf $(CONFIGURE_BUILD_DIR)
else
	$(SHOW)$(MAKE) -C $(CONFIGURE_BUILD_DIR) clean
endif
if ($(FORCE),1)
	$(SHOW)cd $(SRCDIR); git ls-files -o | xargs rm
endif
