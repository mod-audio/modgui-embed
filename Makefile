#!/usr/bin/make -f
# Makefile for modgui-embed #
# ------------------------- #
# Created by falkTX
#

CC  ?= gcc
CXX ?= g++

# --------------------------------------------------------------
# Set build and link flags

BASE_FLAGS = -Wall -Wextra -pipe -DREAL_BUILD -MD -MP
BASE_OPTS  = -O2 -mtune=generic -msse -msse2 -fdata-sections -ffunction-sections -fPIC -DPIC
LINK_OPTS  = -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-O1 -Wl,--as-needed -Wl,--strip-all

ifeq ($(DEBUG),true)
BASE_FLAGS += -DDEBUG -O0 -g
LINK_OPTS   =
else
BASE_FLAGS += -DNDEBUG $(BASE_OPTS) -fvisibility=hidden
CXXFLAGS   += -fvisibility-inlines-hidden
endif

BUILD_C_FLAGS   = $(BASE_FLAGS) -std=c99 -std=gnu99 $(CFLAGS)
BUILD_CXX_FLAGS = $(BASE_FLAGS) -std=c++0x -std=gnu++0x $(CXXFLAGS) $(CPPFLAGS)
LINK_FLAGS      = $(LINK_OPTS) -Wl,--no-undefined $(LDFLAGS)

# --------------------------------------------------------------
# Basic setup

ifeq ($(DEBUG),true)
OBJDIR = $(CURDIR)/build/Debug
else
OBJDIR = $(CURDIR)/build/Release
endif

TARGETS = \
	$(CURDIR)/modgui-x11ui.lv2/modgui-x11.so \
	$(CURDIR)/modgui-x11ui.lv2/modgui-utils.so

# --------------------------------------------------------------
# Common

all: $(TARGETS)

$(OBJDIR)/%.c.o: src/%.c
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $<"
	$(CC) $< $(BUILD_C_FLAGS) -c -o $@

$(OBJDIR)/%.cpp.o: %.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $<"
	$(CXX) $< $(BUILD_CXX_FLAGS) -Icarla-common -c -o $@

clean:
	rm -f $(OBJS)
	rm -f $(TARGETS)

# --------------------------------------------------------------
# Objects

$(CURDIR)/modgui-x11ui.lv2/modgui-x11.so: $(OBJDIR)/lv2_ui.cpp.o
	@echo "Linking modgui-x11"
	$(CXX) $^ $(LINK_FLAGS) -lpthread -shared -o $@

$(CURDIR)/modgui-x11ui.lv2/modgui-utils.so: $(OBJDIR)/lv2_ui-utils.cpp.o
	@echo "Linking libutils.so"
	$(CXX) $^ $(LINK_FLAGS) -lpthread -shared -o $@

# --------------------------------------------------------------

-include $(OBJDIR)/lv2_ui.cpp.d
-include $(OBJDIR)/lv2_ui-utils.cpp.d

# --------------------------------------------------------------
