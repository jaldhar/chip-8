//
// CHIP-8 emulator
//
// By Jaldhar H. Vyas <jaldhar@braincells.com>
// Copyright (C) 2021, Consolidated Braincells Inc.  All rights reserved.
// "Do what thou wilt" shall be the whole of the license.
//

#ifndef VM_H
#define VM_H

#include <array>
#include <cstdint>

constexpr static int MEM_SIZE =   0x1000;
constexpr static int STACK_SIZE = 0x0010;

struct Chip8VM {
    explicit Chip8VM();

    using Registers = std::array<uint8_t, 16>;
    using Memory = std::array<uint8_t, MEM_SIZE>;
    using Stack = std::array<uint16_t, STACK_SIZE>;

    Registers                           V_;     // general-purpose registers
    uint16_t                            I_;     // memory address register
    uint16_t                            PC_;    // program counter register
    uint8_t                             SP_;    // stack pointer register
    uint8_t                             DT_;    // delay timer register
    uint8_t                             ST_;    // sound timer register

    Memory                              memory_;
    Stack                               stack_;
};

#endif