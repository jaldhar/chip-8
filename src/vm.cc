//
// CHIP-8 emulator
//
// By Jaldhar H. Vyas <jaldhar@braincells.com>
// Copyright (C) 2021, Consolidated Braincells Inc.  All rights reserved.
// "Do what thou wilt" shall be the whole of the license.
//

#include "vm.h"

constexpr static int PROGRAM_START = 0x0200;

Chip8VM::Chip8VM() : V_{}, I_{}, PC_{PROGRAM_START}, SP_{}, DT_{}, ST_{},
memory_{}, stack_{} {
}
