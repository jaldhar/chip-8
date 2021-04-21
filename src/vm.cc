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

void Chip8VM::cycle() {
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
                    // TODO
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
            // TODO
            break;
        case 0xD:
            // DXYN - Draw a sprite at position VX, VY with N bytes of sprite
            //        data starting at the address stored in I.  Set VF to 01 if
            //        any set pixels are changed to unset, and 00 otherwise
            // TODO
            break;
        case 0xE:
            switch(instruction.args_.two.NN_) {
                case 0x9E:
                    // EX9E - Skip the following instruction if the key
                    //        corresponding to the hex value currently stored
                    //        in register VX is pressed
                    // TODO
                    break;
                case 0xA1:
                    // EXA1 - Skip the following instruction if the key
                    //        corresponding to the hex value currently stored
                    //        in register VX is not pressed
                    // TODO
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
                    // TODO
                    break;
                case 0x0A:
                    // FX0A - Wait for a keypress and store the result in
                    //        register VX
                    // TODO
                    break;
                case 0x15:
                    // FX15 -   Set the delay timer to the value of register VX
                    // TODO
                    break;
                case 0x18:
                    // FX18 -   Set the sound timer to the value of register VX
                    // TODO
                    break;
                case 0x1E:
                    // FX1E -  Add the value stored in register VX to register I
                    // TODO
                    break;
                case 0x29:
                    // FX29 -  Set I to the memory address of the sprite data
                    //         corresponding to the hexadecimal digit stored in
                    //         register VX
                    // TODO
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
                        I_ += 3;
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
