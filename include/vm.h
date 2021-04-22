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
#include <bitset>
#include <cstdint>

constexpr static int MEM_SIZE =   0x1000;
constexpr static int STACK_SIZE = 0x0010;
constexpr static int SCREEN_WIDTH  = 0x40;
constexpr static int SCREEN_HEIGHT = 0x20;

enum class Command : uint8_t {
    KEY_0 = 0x0,
    KEY_1 = 0x1,
    KEY_2 = 0x2,
    KEY_3 = 0x3,
    KEY_4 = 0x4,
    KEY_5 = 0x5,
    KEY_6 = 0x6,
    KEY_7 = 0x7,
    KEY_8 = 0x8,
    KEY_9 = 0x9,
    KEY_A = 0xa,
    KEY_B = 0xb,
    KEY_C = 0xc,
    KEY_D = 0xd,
    KEY_E = 0xe,
    KEY_F = 0xf,
};

class Chip8VM {
public:
    explicit Chip8VM();

    void  cycle();
    void  decode();
    void  input(Command, bool);
    bool  pixelAt(int, int) const;

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
    using Display = std::array<std::bitset<SCREEN_WIDTH>, SCREEN_HEIGHT>;
    using Keys = std::bitset<16>;

    Registers                           V_;     // general-purpose registers
    uint16_t                            I_;     // memory address register
    uint16_t                            PC_;    // program counter register
    uint8_t                             SP_;    // stack pointer register
    uint8_t                             DT_;    // delay timer register
    uint8_t                             ST_;    // sound timer register

    Memory                              memory_;
    Stack                               stack_;
    Display                             display_;
    Keys                                keys_;
    bool                                blocking_;

};

#endif