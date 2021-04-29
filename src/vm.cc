//
// CHIP-8 emulator
//
// By Jaldhar H. Vyas <jaldhar@braincells.com>
// Copyright (C) 2021, Consolidated Braincells Inc.  All rights reserved.
// "Do what thou wilt" shall be the whole of the license.
//

#include <algorithm>
#include <fstream>
#include "vm.h"

constexpr static int PROGRAM_START = 0x0200;
constexpr static int FONT_START = 0x0050;

Chip8VM::Chip8VM() : V_{}, I_{}, PC_{PROGRAM_START}, SP_{}, DT_{}, ST_{},
memory_{}, stack_{}, display_{}, keys_{},  rnd_{std::random_device{}()},
d_{0, 255}, blocking_{false}, optable_{}, optable0_{}, optable8_{}, optableE_{},
optableF_{} {
    std::array<uint8_t, 80> font {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    std::copy(font.begin(), font.end(), &memory_[FONT_START]);

    optable_[0x0] = &Chip8VM::table0;
    optable_[0x1] = &Chip8VM::jmp;
    optable_[0x2] = &Chip8VM::call;
    optable_[0x3] = &Chip8VM::skip_if_eq_c;
    optable_[0x4] = &Chip8VM::skip_if_neq_c;
    optable_[0x5] = &Chip8VM::skip_if_eq_r;
    optable_[0x6] = &Chip8VM::move_c;
    optable_[0x7] = &Chip8VM::add_c;
    optable_[0x8] = &Chip8VM::table8;
    optable_[0x9] = &Chip8VM::skip_if_neq_r;
    optable_[0xA] = &Chip8VM::load_i;
    optable_[0xB] = &Chip8VM::jmp_v0;
    optable_[0xC] = &Chip8VM::rand;
    optable_[0xD] = &Chip8VM::draw;
    optable_[0xE] = &Chip8VM::tableE;
    optable_[0xF] = &Chip8VM::tableF;

    optable0_.fill(&Chip8VM::no_op);
    optable0_[0x0] = &Chip8VM::cls;
    optable0_[0xE] = &Chip8VM::ret;

    optable8_.fill(&Chip8VM::no_op);
    optable8_[0x0] = &Chip8VM::move_r;
    optable8_[0x1] = &Chip8VM::bitwise_or;
    optable8_[0x2] = &Chip8VM::bitwise_and;
    optable8_[0x3] = &Chip8VM::bitwise_xor;
    optable8_[0x4] = &Chip8VM::add_r;
    optable8_[0x5] = &Chip8VM::sub_r;
    optable8_[0x6] = &Chip8VM::shift_right;
    optable8_[0x7] = &Chip8VM::sub_n;
    optable8_[0xE] = &Chip8VM::shift_left;

    optableE_.fill(&Chip8VM::no_op);
    optableE_[0x1] = &Chip8VM::skip_if_nkey;
    optableE_[0xE] = &Chip8VM::skip_if_key;

    optableF_.fill(&Chip8VM::no_op);
    optableF_[0x07] = &Chip8VM::save_delay;
    optableF_[0x0A] = &Chip8VM::wait_key;
    optableF_[0x15] = &Chip8VM::load_delay;
    optableF_[0x18] = &Chip8VM::load_sound;
    optableF_[0x1E] = &Chip8VM::add_i;
    optableF_[0x29] = &Chip8VM::font;
    optableF_[0x33] = &Chip8VM::bcd;
    optableF_[0x55] = &Chip8VM::save_reg;
    optableF_[0x65] = &Chip8VM::load_reg;

    cls(Instruction{});
}

void Chip8VM::cycle() {
    if (!blocking_) {
        auto instruction = fetch();
        decode(instruction);
    }
}

const Chip8VM::Instruction Chip8VM::fetch() {
    uint16_t fetched = (memory_[PC_] << 8) | memory_[PC_ + 1];
    PC_ += 2;

    return Instruction {
        static_cast<uint16_t>((fetched & 0xF000) >> 12),
        {{ static_cast<uint16_t>(fetched & 0x0FFF) }}
    };
}

void Chip8VM::decode(const Instruction& instruction) {
    optable_[instruction.opcode_](this, instruction);
}

void Chip8VM::table0(const Instruction& instruction) {
    optable0_[instruction.args_.three.N_](this, instruction);
}

void Chip8VM::table8(const Instruction& instruction) {
    optable8_[instruction.args_.three.N_](this, instruction);
}

void Chip8VM::tableE(const Instruction& instruction) {
    optableE_[instruction.args_.three.N_](this, instruction);
}

void Chip8VM::tableF(const Instruction& instruction) {
    optableF_[instruction.args_.two.NN_](this, instruction);
}

void Chip8VM::handleInterrupts() {
    if (DT_) {
        DT_--;
    }

    if (ST_) {
        ST_--;
    }
}

void Chip8VM::input(Command command, bool up) {
    keys_[static_cast<uint8_t>(command)] = up;
}

bool Chip8VM::isBeeping() {
    return ST_ != 0;
}

void Chip8VM::load(const char* filename) {
    std::ifstream input(filename, std::ios::in | std::ios::binary);
    input.exceptions(std::ifstream::failbit);
    input.seekg(0, std::ios::end);
    auto sz = input.tellg();
    input.seekg(0, std::ios::beg);
    auto contents = new char[sz];
    input.read(contents, sz);
    std::copy_n(contents, sz, &memory_[PROGRAM_START]);
    delete[] contents;
}

bool Chip8VM::pixelAt(int height, int width) const {
    return display_[height].test(width);
}

void Chip8VM::no_op(const Instruction&) {
}

// 00E0 -   Clear the screen
void Chip8VM::cls(const Instruction&) {
    std::fill(display_.begin(), display_.end(), 0x00);
}

// 00EE -   Return from a subroutine
void Chip8VM::ret(const Instruction&) {
    SP_--;
    PC_ = stack_[SP_];
}

// 1NNN -   Jump to address NNN
void Chip8VM::jmp(const Instruction& instruction) {
    PC_ = instruction.args_.one.NNN_;
}

// 2NNN -   Execute subroutine starting at address NNN
void Chip8VM::call(const Instruction& instruction) {
    stack_[SP_] = PC_;
    SP_++;
    PC_ = instruction.args_.one.NNN_;
}

// 3XNN -   Skip the following instruction if the value of register
//          VX equals NN
void Chip8VM::skip_if_eq_c(const Instruction& instruction) {
    if (V_[instruction.args_.two.X_] == instruction.args_.two.NN_) {
        PC_ += 2;
    }
}

// 4XNN -   Skip the following instruction if the value of register
//          VX is not equal to NN
void Chip8VM::skip_if_neq_c(const Instruction& instruction) {
    if (V_[instruction.args_.two.X_] != instruction.args_.two.NN_) {
        PC_ += 2;
    }
}

// 5XY0 -   Skip the following instruction if the value of
//          register VX is equal to the value of register VY
void Chip8VM::skip_if_eq_r(const Instruction& instruction) {
    if (V_[instruction.args_.three.X_] == V_[instruction.args_.three.Y_]) {
        PC_ += 2;
    }
}

// 6XNN -   Store number NN in register VX
void Chip8VM::move_c(const Instruction& instruction) {
    V_[instruction.args_.two.X_] = instruction.args_.two.NN_;
}

// 7XNN -   Add the value NN to register VX
//          (Carry flag is not changed)
void Chip8VM::add_c(const Instruction& instruction) {
    V_[instruction.args_.two.X_] += instruction.args_.two.NN_;
}

// 8XY0 -   Store the value of register VY in register VX
void Chip8VM::move_r(const Instruction& instruction) {
    V_[instruction.args_.three.X_] = V_[instruction.args_.three.Y_];
}

// 8XY1 -   Set VX to VX OR VY
void Chip8VM::bitwise_or(const Instruction& instruction) {
    V_[instruction.args_.three.X_] |= V_[instruction.args_.three.Y_];
}

// 8XY2 -   Set VX to VX AND VY
void Chip8VM::bitwise_and(const Instruction& instruction) {
    V_[instruction.args_.three.X_] &= V_[instruction.args_.three.Y_];
}

// 8XY3 -   Set VX to VX XOR VY
void Chip8VM::bitwise_xor(const Instruction& instruction) {
    V_[instruction.args_.three.X_] ^= V_[instruction.args_.three.Y_];
}

// 8XY4 -   Add the value of register VY to register VX
//          Set VF to 01 if a carry occurs
//          Set VF to 00 if a carry does not occur
void Chip8VM::add_r(const Instruction& instruction) {
    uint16_t result = V_[instruction.args_.three.X_] +
        V_[instruction.args_.three.Y_];
    V_[0xF] = (result > 255) ? 1 : 0;
    V_[instruction.args_.three.X_] = (result & 0x00FF);
}

// 8XY5 - Subtract the value of register VY from register VX
//        Set VF to 00 if a borrow occurs
//        Set VF to 01 if a borrow does not occur
void Chip8VM::sub_r(const Instruction& instruction) {
    V_[0xF] = (V_[instruction.args_.three.X_] >
        V_[instruction.args_.three.Y_]) ? 1 : 0;
    V_[instruction.args_.three.X_] -=
        V_[instruction.args_.three.Y_];
}

// 8XY6 - Store the value of register VY shifted right 1 bit
//        in register VX
//        Set register VF to the least significant bit prior
//        to the shift
void Chip8VM::shift_right(const Instruction& instruction) {
    V_[0xF] = V_[instruction.args_.three.Y_] & 0x01;
    V_[instruction.args_.three.X_] = V_[instruction.args_.three.Y_] >> 1;
}

// 8XY7 -   Set register VX to the value of VY minus VX
//          Set VF to 00 if a borrow occurs
//          Set VF to 01 if a borrow does not occur
void Chip8VM::sub_n(const Instruction& instruction) {
    V_[0xF] = (V_[instruction.args_.three.Y_] >
        V_[instruction.args_.three.X_]) ? 1 : 0;
    V_[instruction.args_.three.X_] = V_[instruction.args_.three.Y_] -
        V_[instruction.args_.three.X_];
}

// 8XYE     Store the value of register VY shifted left one
//          bit in register VX
//          Set register VF to the most significant bit
//          prior to the shift
void Chip8VM::shift_left(const Instruction& instruction) {
    V_[0xF] = V_[instruction.args_.three.Y_] & 0x80;
    V_[instruction.args_.three.X_] = V_[instruction.args_.three.Y_] << 1;
}

// 9XY0 -   Skip the following instruction if the value of
//          register VX is not equal to value of register VY
void Chip8VM::skip_if_neq_r(const Instruction& instruction) {
    if (V_[instruction.args_.three.X_] != V_[instruction.args_.three.Y_]) {
        PC_ += 2;
    }
 }

// ANNN -   Store memory address NNN in register I
void Chip8VM::load_i(const Instruction& instruction) {
    I_ = instruction.args_.one.NNN_;
}

void Chip8VM::jmp_v0(const Instruction& instruction) {
// BNNN -   Jump to address NNN + V0
PC_ = instruction.args_.one.NNN_ + V_[0];
}

// CXNN -   Set VX to a random number with a mask of NN
void Chip8VM::rand(const Instruction& instruction) {
    V_[instruction.args_.two.X_] = d_(rnd_) & instruction.args_.two.NN_;
}

// DXYN - Draw a sprite at position VX, VY with N bytes of sprite
//        data starting at the address stored in I.  Set VF to 01 if
//        any set pixels are changed to unset, and 00 otherwise
void Chip8VM::draw(const Instruction& instruction) {
    auto originX = V_[instruction.args_.three.X_] & (SCREEN_WIDTH - 1);
    auto originY = V_[instruction.args_.three.Y_] & (SCREEN_HEIGHT - 1);
    V_[0xF] = 0;

    for (auto row = 0; row < instruction.args_.three.N_; row++) {
        auto posY = originY + row;

        if (posY < 0 || posY >= SCREEN_HEIGHT) {
            continue;
        }

        auto data = memory_[I_ + row];

        for (uint8_t bit = 0x80,col = 0; bit > 0; bit >>= 1,col++) {
            auto posX = originX + col;

            if (posX < 0 || posX >= SCREEN_WIDTH) {
                continue;
            }

            auto previous = display_[posY].test(posX);
            auto current = data & bit ? true : false;

            display_[posY].set(posX, previous ^ current);

            if  (previous && !current) {
                V_[0xF] = 1;
            }
        }
    }
}

// EX9E - Skip the following instruction if the key
//        corresponding to the hex value currently stored
//        in register VX is pressed
void Chip8VM::skip_if_key(const Instruction& instruction) {
    if (keys_[V_[instruction.args_.two.X_]]) {
        PC_ += 2;
    }
}

// EXA1 - Skip the following instruction if the key
//        corresponding to the hex value currently stored
//        in register VX is not pressed
void Chip8VM::skip_if_nkey(const Instruction& instruction) {
    if (!keys_[V_[instruction.args_.two.X_]]) {
        PC_ += 2;
    }
}


// FX07 - Store the current value of the delay timer in
//        register VX
void Chip8VM::save_delay(const Instruction& instruction) {
    V_[instruction.args_.two.X_] = DT_;
}

// FX0A - Wait for a keypress and store the result in
//        register VX
void Chip8VM::wait_key(const Instruction& instruction) {
    if (keys_.any()) {
        blocking_ = false;
        for (size_t i = 0; i < keys_.size(); i++) {
            if (keys_[i]) {
                V_[instruction.args_.two.X_] = i;
                break;
            }
        }
    } else {
        blocking_ = true;
    }
}

// FX15 -   Set the delay timer to the value of register VX
void Chip8VM::load_delay(const Instruction& instruction) {
    DT_ = V_[instruction.args_.two.X_];
}

// FX18 -   Set the sound timer to the value of register VX
void Chip8VM::load_sound(const Instruction& instruction) {
    ST_ = V_[instruction.args_.two.X_];
}

// FX1E -  Add the value stored in register VX to register I
void Chip8VM::add_i(const Instruction& instruction) {
    uint16_t result = I_ + V_[instruction.args_.two.X_];
    V_[0xF] = (result > 0xFFF) ? 1 : 0;
    I_ = result;
}

// FX29 -  Set I to the memory address of the sprite data
//         corresponding to the hexadecimal digit stored in
//         register VX
void Chip8VM::font(const Instruction& instruction) {
    I_ = FONT_START + (5 * instruction.args_.two.X_);
}

// FX33 - Store the binary-coded decimal equivalent of the
//        value stored in register VX at addresses I, I+1,
//        and I+2
void Chip8VM::bcd(const Instruction& instruction) {
    auto temp = V_[instruction.args_.two.X_];

    for (auto i = 0, power = 100; i < 3; i++, power /= 10) {
        memory_[I_ + i] = temp / power;
        temp = temp % power;
    }
}

// FX55 - Store the values of registers V0 to VX inclusive
//        in memory starting at address I
//        I is set to I + X + 1 after operation
void Chip8VM::save_reg(const Instruction& instruction) {
    for (auto i = 0; i <= instruction.args_.two.X_; i++) {
        memory_[I_ + i] = V_[i];
    }
    I_ += (instruction.args_.two.X_ + 1);
}

// FX65 -  Fill registers V0 to VX inclusive with the values
//         stored in memory starting at address I
//         I is set to I + X + 1 after operation
void Chip8VM::load_reg(const Instruction& instruction) {
    for (auto i = 0; i <= instruction.args_.two.X_; i++) {
        V_[i] = memory_[I_ + i];
    }
    I_ += (instruction.args_.two.X_ + 1);
}