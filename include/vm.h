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
#include <functional>
#include <random>

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

enum class KBState : uint8_t {
    UNBLOCKED = 0,
    RELEASING = 1,
    BLOCKED   = 2
};

class Chip8VM {
public:
    explicit Chip8VM();

    void  cycle();
    void  handleInterrupts();
    void  input(Command, bool);
    bool  isBeeping();
    void  load(const char* filename);
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

    const Instruction   fetch();
    void                decode(const Instruction&);

    void                table0(const Instruction&);
    void                table8(const Instruction&);
    void                tableE(const Instruction&);
    void                tableF(const Instruction&);

    void                no_op(const Instruction&);
    void                cls(const Instruction&);
    void                ret(const Instruction&);
    void                jmp(const Instruction&);
    void                call(const Instruction&);
    void                skip_if_eq_c(const Instruction&);
    void                skip_if_neq_c(const Instruction&);
    void                skip_if_eq_r(const Instruction&);
    void                move_c(const Instruction&);
    void                add_c(const Instruction&);
    void                move_r(const Instruction&);
    void                bitwise_or(const Instruction&);
    void                bitwise_and(const Instruction&);
    void                bitwise_xor(const Instruction&);
    void                add_r(const Instruction&);
    void                sub_r(const Instruction&);
    void                shift_right(const Instruction&);
    void                sub_n(const Instruction&);
    void                shift_left(const Instruction&);
    void                skip_if_neq_r(const Instruction&);
    void                load_i(const Instruction&);
    void                jmp_v0(const Instruction&);
    void                rand(const Instruction&);
    void                draw(const Instruction&);
    void                skip_if_key(const Instruction&);
    void                skip_if_nkey(const Instruction&);
    void                save_delay(const Instruction&);
    void                wait_key(const Instruction&);
    void                load_delay(const Instruction&);
    void                load_sound(const Instruction&);
    void                add_i(const Instruction&);
    void                font(const Instruction&);
    void                bcd(const Instruction&);
    void                save_reg(const Instruction&);
    void                load_reg(const Instruction&);

    using Registers = std::array<uint8_t, 16>;
    using Memory = std::array<uint8_t, MEM_SIZE>;
    using Stack = std::array<uint16_t, STACK_SIZE>;
    using Display = std::array<std::bitset<SCREEN_WIDTH>, SCREEN_HEIGHT>;
    using Keys = std::bitset<16>;
    using Opcode = std::function<void(Chip8VM*, const Instruction&)>;

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
    std::minstd_rand                    rnd_;
    std::uniform_int_distribution<unsigned short> d_;
    KBState                             kbstate_;

    std::array<Opcode, 16>              optable_;
    std::array<Opcode, 16>              optable0_;
    std::array<Opcode, 16>              optable8_;
    std::array<Opcode, 16>              optableE_;
    std::array<Opcode, 256>             optableF_;
};

#endif