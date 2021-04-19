#
# CHIP-8 emulator
#
# By Jaldhar H. Vyas <jaldhar@braincells.com>
# Copyright (C) 2021, Consolidated Braincells Inc.  All rights reserved.
# "Do what thou wilt" shall be the whole of the license.
#

CXXFLAGS += -O2
VPATH = ../src:../include

include ../Makefile

install-$(PROGRAM): $(PROGRAM)
	$(INSTALL) -m755 -D -d $(DESTDIR)$(PREFIX)/$(BINDIR)
	$(INSTALL) -m755 $< $(DESTDIR)$(PREFIX)/$(BINDIR/$(PROGRAM)

