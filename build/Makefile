# This makefile should be used for building the whole project

# Init global variables

# == Directories ==
export BUILDROOT := $(CURDIR)
export SRCROOT := $(abspath $(BUILDROOT)/..)
export TESTROOT := $(abspath $(BUILDROOT)/../test)
export INCROOT := $(SRCROOT)/src
export MAKE_SUBDIRS := $(BUILDROOT)/subdirs.mk
export MAKE_TARGET := $(BUILDROOT)/target.mk
export PREFIX ?= $(SIMURGA_IMAGE)
export PREFIX ?= /usr/local
FCGIPPINC := $(abspath $(BUILDROOT)/../fastcgipp/include)
TMPROOT := $(abspath $(BUILDROOT)/../tmp)
ifeq ($(PREFIX), /usr)
    export SYSCONFDIR ?= /etc/simurga
else
    export SYSCONFDIR ?= $(PREFIX)/etc
endif

# == Compiler to use ==
CC := gcc
CXX := g++ -std=c++11
AR := ar
STRIP := strip
export CC
export CXX
export AR
export STRIP
export TMPROOT := $(TMPROOT)

# == Compiler flags ==
ifeq ($(DEBUG), yes)
    OPT := -g
    DDEF := -D_DEBUG
else
    OPT := -O2
endif
ifdef EXTRASROOT
    CPPFLAGS += -I$(EXTRASROOT)/include
    LDFLAGS += -L$(EXTRASROOT)/lib -Wl,-rpath=$(EXTRASROOT)/lib
endif
CFLAGS ?= $(OPT) -Wall -fPIC
CXXFLAGS ?= $(OPT) -Wall -fPIC
CXXFLAGS += -fexceptions -frtti
CPPFLAGS += -I$(INCROOT) -I$(FCGIPPINC) -I. $(DDEF) -DSYSCONFDIR=\"$(SYSCONFDIR)\" -DPREFIX=\"$(PREFIX)\"
LDFLAGS ?= -rdynamic -shared-libgcc -Wl,-rpath=$(PREFIX)/lib
export CFLAGS
export CXXFLAGS
export CPPFLAGS
export LDFLAGS

# == Find sysroot dir ==
ifdef PKG_CONFIG_PATH
    PKG_CONFIG_PATH := $(PKG_CONFIG_PATH):
endif
PKG_CONFIG_PATH := $(PKG_CONFIG_PATH)$(SYSROOT)/lib/pkgconfig:$(SYSROOT)/$(ARCH)/lib/pkgconfig
ifdef EXTRASROOT
    PKG_CONFIG_PATH := $(PKG_CONFIG_PATH):$(EXTRASROOT)/lib/pkgconfig
endif
export PKG_CONFIG_PATH
PKGCONFIG ?= $(ARCHP)pkg-config
export PKGCONFIG


.PHONY: all install test runtest clean

all: $(TMPROOT)
	@$(MAKE) -C ../fastcgipp all
	@$(MAKE) -C ../src all

install:
	@$(MAKE) -C ../src install

clean:
	rm -rf $(TMPROOT)/*

$(TMPROOT):
	mkdir -p $(TMPROOT)

