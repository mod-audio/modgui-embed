#!/usr/bin/make -f
# Makefile for modgui-embed #
# ------------------------- #
# Created by falkTX
#

CC  ?= gcc
CXX ?= g++

# --------------------------------------------------------------
# Set build and link flags

BASE_FLAGS = -Wall -Wextra -pipe -MD -MP $(shell pkg-config --cflags carla-utils)
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

OBJS   = $(OBJDIR)/lv2_ui.cpp.o
TARGET = $(CURDIR)/modgui-x11ui.lv2/modgui-x11.so

# --------------------------------------------------------------
# Common

all: $(TARGET)

$(OBJDIR)/%.c.o: src/%.c
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $<"
	$(CC) $< $(BUILD_C_FLAGS) -c -o $@

$(OBJDIR)/%.cpp.o: %.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $<"
	$(CXX) $< $(BUILD_CXX_FLAGS) -c -o $@

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

# --------------------------------------------------------------
# Objects

$(CURDIR)/modgui-x11ui.lv2/modgui-x11.so: $(OBJS)
	@echo "Linking modgui-x11"
	$(CXX) $^ $(LINK_FLAGS) -lpthread -shared -o $@

# --------------------------------------------------------------

-include $(OBJS:%.o=%.d)

# --------------------------------------------------------------
