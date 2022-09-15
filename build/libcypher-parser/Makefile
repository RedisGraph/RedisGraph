
ROOT=../..
MK.pyver:=3
include $(ROOT)/deps/readies/mk/main

MK.configure=1

define HELPTEXT
make build    # configure and compile
make clean    # clean generated sbinaries
  ALL=1       # remote entire binary directory
make source   # fetch sources and generate configure script
endef

#----------------------------------------------------------------------------------------------

BINDIR=$(BINROOT)/libcypher-parser
SRCDIR=$(ROOT)/deps/libcypher-parser

TARGET=$(BINDIR)/lib/src/.libs/libcypher-parser.a

export CFLAGS += -fPIC -O3 -DYY_BUFFER_SIZE=1048576 -I$(BINDIR)/lib/src

#----------------------------------------------------------------------------------------------

include $(MK)/defs
include $(MK)/rules

#----------------------------------------------------------------------------------------------

source: $(SRCDIR)/configure

.PHONY: source

$(SRCDIR)/configure: $(SRCDIR)/configure.ac
	$(SHOW)cd $(SRCDIR); ./autogen.sh
