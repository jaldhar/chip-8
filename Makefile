#
# CHIP-8 emulator
#
# By Jaldhar H. Vyas <jaldhar@braincells.com>
# Copyright (C) 2021, Consolidated Braincells Inc.  All rights reserved.
# "Do what thou wilt" shall be the whole of the license.
#

PROGRAM=chip8
SRCDIR:=../src
INCDIR:=../include
PREFIX?=/usr/local
BINDIR?=bin

SRC:=$(wildcard $(SRCDIR)/*.cc)
OBJECTS:=$(patsubst $(SRCDIR)/%.cc,./%.o,$(SRC))
DEPFILES:=$(patsubst $(SRCDIR)/%.cc,./%.d,$(SRC))

CXX?=/usr/bin/g++
STRIP?=/usr/bin/strip --strip-all  -R .comment -R .note $(PROGRAM)
INSTALL?=/usr/bin/install

DEPFLAGS=-MT $@ -MMD -MP -MF $*.d
CPPFLAGS+=$(DEPFLAGS) -I$(INCDIR)
CXXFLAGS+=-std=c++17 -Wall -Wextra -Wpedantic -Weffc++ -flto
LDFLAGS+=-ffunction-sections -fdata-sections -Wl,-gc-sections
LIBS=-lX11 -lGL -lpthread -lpng -lstdc++fs

get_builddir = '$(findstring '$(notdir $(CURDIR))', 'debug' 'release')'

.cc.o:

$(PROGRAM): $(OBJECTS) | checkinbuilddir
	$(LINK.cc) $(OUTPUT_OPTION) $^ $(LIBS)
	$(STRIP)

$(DEPFILES):

checkinbuilddir:
ifeq ($(call get_builddir), '')
	$(error 'Change to the debug or release directories and run make from there.')
endif

checkintopdir:
ifneq ($(call get_builddir), '')
	$(error 'Make this target from the top-level directory.')
endif

install:
	@cd release && $(MAKE) install-$(PROGRAM)

clean:
	-$(RM) *.o *.d $(PROGRAM)

distclean: | checkintopdir
	cd debug && $(MAKE) clean
	cd release && $(MAKE) clean

.PHONY: checkinbuilddir checkintopdir install clean distclean

.DELETE_ON_ERROR:

-include $(wildcard $(DEPFILES))
