
ROOT=../..
MK.pyver:=3
include $(ROOT)/deps/readies/mk/main

define HELPTEXT
make build    # configure and compile
make clean    # clean generated sbinaries
  ALL=1       # remote entire binary directory
endef

MK_ALL_TARGETS=build

#----------------------------------------------------------------------------------------------

BINDIR=$(BINROOT)/xxHash
SRCDIR=$(ROOT)/deps/xxHash

TARGET=$(BINDIR)/libxxhash.a

#----------------------------------------------------------------------------------------------

MK_CUSTOM_CLEAN=1

include $(MK)/defs

CC=gcc

_SOURCES=xxhash.c

SOURCES=$(addprefix $(SRCDIR)/,$(_SOURCES))
OBJECTS=$(patsubst $(SRCDIR)/%.c,$(BINDIR)/%.o,$(SOURCES))

CC_DEPS = $(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.d, $(SOURCES))

CC_FLAGS += \
	-fPIC \
	-std=gnu99 \
	-MMD -MF $(@:.o=.d) \
	-I$(SRCDIR) \
	-I$(BINDIR) \
	-I$(LIBSSL_PREFIX)/include \
	$(CC_FLAGS.coverage)

ifeq ($(DEBUG),1)
CC_FLAGS += -g -O0 \
	-Wall -Wextra -Wconversion -Wcast-qual -Wcast-align -Wshadow \
	-Wstrict-aliasing=1 -Wswitch-enum -Wdeclaration-after-statement \
	-Wstrict-prototypes -Wundef -Wpointer-arith -Wformat-security \
	-Wvla -Wformat=2 -Winit-self -Wfloat-equal -Wwrite-strings \
	-Wredundant-decls -Wstrict-overflow=5
LD_FLAGS += -g
else
CC_FLAGS += -O3 -Wno-unused-result
LD_FLAGS += -lto
endif

ifeq ($(OS),macos)
LD_FLAGS += -undefined dynamic_lookup
endif

LD_FLAGS += $(LD_FLAGS.coverage)

#----------------------------------------------------------------------------------------------

include $(MK)/rules

-include $(CC_DEPS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling $<...
	$(SHOW)$(CC) $(CC_FLAGS) -fvisibility=hidden -c $< -o $@

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
