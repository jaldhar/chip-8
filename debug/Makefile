#
# CHIP-8 emulator
#
# By Jaldhar H. Vyas <jaldhar@braincells.com>
# Copyright (C) 2021, Consolidated Braincells Inc.  All rights reserved.
# "Do what thou wilt" shall be the whole of the license.
#

CPPFLAGS += -DDEBUG
CXXFLAGS += -g3 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
VPATH = ../src:../include

include ../Makefile

STRIP=
