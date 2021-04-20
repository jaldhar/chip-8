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

class Chip8VM {
public:
    explicit Chip8VM();

    void  cycle();

private:
    struct OneArg {
        uint16_t NNN_:12;
    };

    struct TwoArgs {
        uint16_t NN_:8;
        uint16_t X_:4;
    };

    struct ThreeArgs {
        uint16_t N_:4;
        uint16_t Y_:4;
        uint16_t X_:4;
    };

    struct Instruction {
        uint16_t opcode_:4;
        union {
            OneArg one;
            TwoArgs two;
            ThreeArgs three;
        } args_;
    };

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