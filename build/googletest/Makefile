
ROOT=../..

include $(ROOT)/deps/readies/mk/main

define HELPTEXT
make build    # configure and compile
make clean    # clean generated sbinaries
  ALL=1       # remote entire binary directory
endef

MK_ALL_TARGETS=build

#----------------------------------------------------------------------------------------------

BINDIR=$(BINROOT)/googletest
SRCDIR=$(ROOT)/deps/googletest/googletest/src
SRCBASEDIR=$(ROOT)/deps/googletest/googletest

TARGET=$(BINDIR)/libgtest.a

#----------------------------------------------------------------------------------------------

MK_CUSTOM_CLEAN=1

include $(MK)/defs

_SOURCES=gtest-all.cc gtest_main.cc

SOURCES=$(addprefix $(SRCDIR)/,$(_SOURCES))
OBJECTS=$(patsubst $(SRCDIR)/%.cc,$(BINDIR)/%.o,$(SOURCES))

CC_DEPS = $(patsubst $(SRCDIR)/%.cc, $(BINDIR)/%.d, $(SOURCES))

CC_FLAGS += \
	-g \
	-fPIC \
	-fvisibility=hidden \
	-MMD -MF $(@:.o=.d) \
	-I$(SRCDIR) \
	-I$(SRCBASEDIR)/include \
	-I$(SRCBASEDIR) \
	$(CC_FLAGS.coverage)

CXX_FLAGS += \
	-std=c++11

ifeq ($(DEBUG),1)
CC_FLAGS += -O0
LD_FLAGS += -g
else
CC_FLAGS += -O3 -Wno-unused-result
LD_FLAGS += -g -lto
endif

ifeq ($(OS),macos)
LD_FLAGS += -undefined dynamic_lookup
endif

LD_FLAGS += $(LD_FLAGS.coverage)

#----------------------------------------------------------------------------------------------

include $(MK)/rules

-include $(CC_DEPS)

$(BINDIR)/%.o: $(SRCDIR)/%.cc
	@echo Compiling $<...
	$(SHOW)$(CXX) $(CC_FLAGS) $(CXX_FLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	@echo Creating $@...
	$(SHOW)$(AR) rcs $@ $(OBJECTS)

clean:
ifeq ($(ALL),1)
	$(SHOW)rm -rf $(BINDIR) $(TARGET)
else
	-$(SHOW)find $(BINDIR) -name '*.[oadh]' -type f -delete
	$(SHOW)rm -f $(TARGET)
endif
