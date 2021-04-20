//
// CHIP-8 emulator
//
// By Jaldhar H. Vyas <jaldhar@braincells.com>
// Copyright (C) 2021, Consolidated Braincells Inc.  All rights reserved.
// "Do what thou wilt" shall be the whole of the license.
//

#include <chrono>
#include <clocale>
#include <csignal>
#include <cstdlib>
#include "vm.h"

constexpr double TICK = 1E9 / 60.0;

volatile bool endflag = false;

void system_end(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            endflag = true;
            break;
        case SIGHUP:
            exit(EXIT_FAILURE);
            break;
        default:
            break;
    }
}

int main() {
    setlocale(LC_ALL, "POSIX");

    struct sigaction act;
    act.sa_handler = system_end;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    Chip8VM vm;

    std::chrono::steady_clock clock;
    auto previous = clock.now();
    auto lag = 0.0;

    while (!endflag) {
        auto current = clock.now();
        auto elapsed = current - previous;
        previous = current;
        lag += elapsed.count();

        while (lag >= TICK) {
            lag -= TICK;
            vm.cycle();
        }
    }

    return EXIT_SUCCESS;
}