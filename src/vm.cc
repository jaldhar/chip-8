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
d_{0, 255}, blocking_{false} {

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
}

void Chip8VM::cycle() {
    if (!blocking_) {
        decode();
    }
}

void Chip8VM::decode() {
    uint16_t fetched = (memory_[PC_] << 8) | memory_[PC_ + 1];
    Instruction instruction{
        static_cast<uint16_t>((fetched & 0xF000) >> 12),
        {{ static_cast<uint16_t>(fetched & 0x0FFF) }}
    };

    PC_ += 2;

    switch(instruction.opcode_) {
        case 0x0:
            switch(instruction.args_.one.NNN_) {
                case 0xE0:
                    // 00E0 -   Clear the screen
                    std::fill(display_.begin(), display_.end(), 0x00);
                    break;
                case 0xEE:
                    // 00EE -   Return from a subroutine
                    SP_--;
                    PC_ = stack_[SP_];
                    break;
                default:
                    break;
            }
            break;
        case 0x1:
            // 1NNN -   Jump to address NNN
            PC_ = instruction.args_.one.NNN_;
            break;
        case 0x2:
            // 2NNN -   Execute subroutine starting at address NNN
            stack_[SP_] = PC_;
            SP_++;
            PC_ = instruction.args_.one.NNN_;
            break;
        case 0x3:
            // 3XNN -   Skip the following instruction if the value of register
            //          VX equals NN
            if (V_[instruction.args_.two.X_] == instruction.args_.two.NN_) {
                PC_ += 2;
            }
            break;
        case 0x4:
            // 4XNN -   Skip the following instruction if the value of register
            //          VX is not equal to NN
            if (V_[instruction.args_.two.X_] != instruction.args_.two.NN_) {
                PC_ += 2;
            }
            break;
        case 0x5:
            switch(instruction.args_.three.N_) {
                case 0x0:
                    // 5XY0 -   Skip the following instruction if the value of
                    //          register VX is equal to the value of register VY
                    if (V_[instruction.args_.three.X_] ==
                    V_[instruction.args_.three.Y_]) {
                        PC_ += 2;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 0x6:
            // 6XNN -   Store number NN in register VX
            V_[instruction.args_.two.X_] = instruction.args_.two.NN_;
            break;
        case 0x7:
            // 7XNN -   Add the value NN to register VX
            //          (Carry flag is not changed)
            V_[instruction.args_.two.X_] += instruction.args_.two.NN_;
            break;
        case 0x8:
            switch(instruction.args_.three.N_) {
                case 0x0:
                    // 8XY0 -   Store the value of register VY in register VX
                    V_[instruction.args_.three.X_] =
                        V_[instruction.args_.three.Y_];
                    break;
                case 0x1:
                    // 8XY1 -   Set VX to VX OR VY
                    V_[instruction.args_.three.X_] |=
                        V_[instruction.args_.three.Y_];
                    break;
                case 0x2:
                    // 8XY2 -   Set VX to VX AND VY
                    V_[instruction.args_.three.X_] &=
                        V_[instruction.args_.three.Y_];
                    break;
                case 0x3:
                    // 8XY3 -   Set VX to VX XOR VY
                    V_[instruction.args_.three.X_] ^=
                        V_[instruction.args_.three.Y_];
                    break;
                case 0x4:
                    // 8XY4 -   Add the value of register VY to register VX
                    //          Set VF to 01 if a carry occurs
                    //          Set VF to 00 if a carry does not occur
                    {
                        uint16_t result = V_[instruction.args_.three.X_] + 
                            V_[instruction.args_.three.Y_];
                        V_[0xF] = (result > 255) ? 1 : 0;
                        V_[instruction.args_.three.X_] = (result & 0x00FF);
                    }
                    break;
                case 0x5:
                    // 8XY5 - Subtract the value of register VY from register VX
                    //        Set VF to 00 if a borrow occurs
                    //        Set VF to 01 if a borrow does not occur
                    V_[0xF] = (V_[instruction.args_.three.X_] >
                        V_[instruction.args_.three.Y_]) ? 1 : 0;
                    V_[instruction.args_.three.X_] -=
                        V_[instruction.args_.three.Y_];
                    break;
                case 0x6:
                    // 8XY6 - Store the value of register VY shifted right 1 bit
                    //        in register VX
                    //        Set register VF to the least significant bit prior
                    //        to the shift
                    V_[0xF] = V_[instruction.args_.three.Y_] & 0x01;
                    V_[instruction.args_.three.X_] =
                        V_[instruction.args_.three.Y_] >> 1;
                    break;
                case 0x7:
                    // 8XY7 -   Set register VX to the value of VY minus VX
                    //          Set VF to 00 if a borrow occurs
                    //          Set VF to 01 if a borrow does not occur
                    V_[0xF] = (V_[instruction.args_.three.Y_] >
                        V_[instruction.args_.three.X_]) ? 1 : 0;
                    V_[instruction.args_.three.X_] =
                        V_[instruction.args_.three.Y_] -
                        V_[instruction.args_.three.X_];
                    break;
                case 0xE:
                    // 8XYE     Store the value of register VY shifted left one
                    //          bit in register VX
                    //          Set register VF to the most significant bit
                    //          prior to the shift
                    V_[0xF] = V_[instruction.args_.three.Y_] & 0x80;
                    V_[instruction.args_.three.X_] =
                        V_[instruction.args_.three.Y_] << 1;
                    break;
                default:
                    break;
            }
            break;
        case 0x9:
            switch(instruction.args_.three.N_) {
                case 0x0:
                    // 9XY0 -   Skip the following instruction if the value of
                    //          register VX is not equal to value of register VY
                    if (V_[instruction.args_.three.X_] !=
                    V_[instruction.args_.three.Y_]) {
                        PC_ += 2;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 0xA:
            // ANNN -   Store memory address NNN in register I
            I_ = instruction.args_.one.NNN_;
            break;
        case 0xB:
            // BNNN -   Jump to address NNN + V0
            PC_ = instruction.args_.one.NNN_ + V_[0];
            break;
        case 0xC:
            // CXNN -   Set VX to a random number with a mask of NN
            V_[instruction.args_.two.X_] = d_(rnd_) & instruction.args_.two.NN_;
            break;
        case 0xD:
            // DXYN - Draw a sprite at position VX, VY with N bytes of sprite
            //        data starting at the address stored in I.  Set VF to 01 if
            //        any set pixels are changed to unset, and 00 otherwise
            {
                auto originX = V_[instruction.args_.three.X_] % SCREEN_WIDTH;
                auto originY = V_[instruction.args_.three.Y_] % SCREEN_HEIGHT;
                V_[0xF] = 0;

                for (auto row = 0; row < instruction.args_.three.N_; row++) {
                    auto posY = originY + row;

                    if (posY < 0 || posY >= SCREEN_HEIGHT) {
                        break;
                    }

                    auto data = memory_[I_ + row];

                    for (uint8_t bit = 0x80,col = 0; bit > 0; bit >>= 1,col++) {
                        auto posX = originX + col;

                        if (posX < 0 || posX >= SCREEN_WIDTH) {
                            break;
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
            break;
        case 0xE:
            switch(instruction.args_.two.NN_) {
                case 0x9E:
                    // EX9E - Skip the following instruction if the key
                    //        corresponding to the hex value currently stored
                    //        in register VX is pressed
                    if (keys_[V_[instruction.args_.two.X_]]) {
                        PC_ += 2;
                    }
                    break;
                case 0xA1:
                    // EXA1 - Skip the following instruction if the key
                    //        corresponding to the hex value currently stored
                    //        in register VX is not pressed
                    if (!keys_[V_[instruction.args_.two.X_]]) {
                        PC_ += 2;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 0xF:
            switch(instruction.args_.two.NN_) {
                case 0x07:
                    // FX07 - Store the current value of the delay timer in
                    //        register VX
                    V_[instruction.args_.two.X_] = DT_;
                    break;
                case 0x0A:
                    // FX0A - Wait for a keypress and store the result in
                    //        register VX
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
                    break;
                case 0x15:
                    // FX15 -   Set the delay timer to the value of register VX
                    DT_ = V_[instruction.args_.two.X_];
                    break;
                case 0x18:
                    // FX18 -   Set the sound timer to the value of register VX
                    ST_ = V_[instruction.args_.two.X_];
                    break;
                case 0x1E:
                    // FX1E -  Add the value stored in register VX to register I
                    {
                        uint16_t result = I_ + V_[instruction.args_.two.X_];
                        V_[0xF] = (result > 0xFFF) ? 1 : 0;
                        I_ = result;
                    }
                    break;
                case 0x29:
                    // FX29 -  Set I to the memory address of the sprite data
                    //         corresponding to the hexadecimal digit stored in
                    //         register VX
                    I_ = FONT_START + (5 * instruction.args_.two.X_);
                    break;
                case 0x33:
                    // FX33 - Store the binary-coded decimal equivalent of the
                    //        value stored in register VX at addresses I, I+1,
                    //        and I+2
                    {
                        auto temp = V_[instruction.args_.two.X_];
                        for (auto i = 0, power = 100; i < 3; i++, power /= 10) {
                            memory_[I_ + i] = temp / power;
                            temp = temp % power;
                        }
                    }
                    break;
                case 0x55:
                    // FX55 - Store the values of registers V0 to VX inclusive
                    //        in memory starting at address I
                    //        I is set to I + X + 1 after operation
                    for (auto i = 0; i <= instruction.args_.two.X_; i++) {
                        memory_[I_ + i] = V_[i];
                    }
                    I_ += (instruction.args_.two.X_ + 1);
                    break;
                case 0x65:
                    // FX65 -  Fill registers V0 to VX inclusive with the values
                    //         stored in memory starting at address I
                    //         I is set to I + X + 1 after operation
                    for (auto i = 0; i <= instruction.args_.two.X_; i++) {
                        V_[i] = memory_[I_ + i];
                    }
                    I_ += (instruction.args_.two.X_ + 1);
                    break;
                default:
                    break;
            }
            break;
    }

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