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
#include <iostream>
#include <map>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_SOUNDWAVE
#include "olcSoundWaveEngine.h"

#include "vm.h"

constexpr static int SCALE = 8;
constexpr static float INTERRUPT_TICK = 1.0f / 60.0f;
constexpr static float CPU_TICK = 1.0f / 240.0f;
constexpr static std::size_t SAMPLE_RATE = 44100;
constexpr static std::size_t SAMPLES = SAMPLE_RATE / 60;
constexpr static float FREQUENCY = 440.0f;
constexpr static float TAU = 2.0f * 3.14159f;

volatile bool endflag = false;

void system_end(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            endflag = true;
            break;
        case SIGABRT:
#ifndef _WIN32
        case SIGHUP:
#endif
            exit(EXIT_FAILURE);
            break;
        default:
            break;
    }
}

class View : public olc::PixelGameEngine
{
public:
    View(Chip8VM&);
    ~View()=default;

    bool OnUserCreate() override;
    bool OnUserDestroy() override;
    bool OnUserUpdate(float) override;

private:
    void draw();
    void handleInput();

    using Keymap = std::map<Command, const olc::Key>;

    float cpuLag_;
    float interruptLag_;
    Keymap keys_;

    Chip8VM& vm_;

	olc::sound::WaveEngine soundengine_;
	olc::sound::Wave beep_;

};

View::View(Chip8VM& vm) : cpuLag_{ 0.0f }, interruptLag_ { 0.0f }, keys_{
        { Command::KEY_0, olc::Key::X },
        { Command::KEY_1, olc::Key::K1 },
        { Command::KEY_2, olc::Key::K2 },
        { Command::KEY_3, olc::Key::K3 },
        { Command::KEY_4, olc::Key::Q },
        { Command::KEY_5, olc::Key::W },
        { Command::KEY_6, olc::Key::E },
        { Command::KEY_7, olc::Key::A },
        { Command::KEY_8, olc::Key::S },
        { Command::KEY_9, olc::Key::D },
        { Command::KEY_A, olc::Key::Z },
        { Command::KEY_B, olc::Key::C },
        { Command::KEY_C, olc::Key::K4 },
        { Command::KEY_D, olc::Key::R },
        { Command::KEY_E, olc::Key::F },
        { Command::KEY_F, olc::Key::V },
    }, vm_{vm}, soundengine_{}, beep_{} {
    sAppName = "CHIP-8";
}

bool View::OnUserCreate() {
    soundengine_.InitialiseAudio(SAMPLE_RATE, 1);

    beep_ = olc::sound::Wave(1, sizeof(uint8_t), SAMPLE_RATE, SAMPLES);

    double dt = 1.0 / SAMPLE_RATE;
    for (size_t i = 0; i < SAMPLES; i++) {
        double t = double(i) * dt;
        beep_.file.data()[i] = float(0.5 * sin(TAU * FREQUENCY * t));
    }

    return true;
}

bool View::OnUserDestroy() {

    return true;
}

bool View::OnUserUpdate(float elapsed) {
    if (endflag) {
        return false;
    }

    // fixed time step
    cpuLag_ += elapsed;
    interruptLag_ += elapsed;

    if (cpuLag_ >= CPU_TICK) {
        cpuLag_ -= CPU_TICK;

        handleInput();
        vm_.cycle();
        draw();
    }

    if (interruptLag_ >= INTERRUPT_TICK) {
        interruptLag_ -= INTERRUPT_TICK;

        if (vm_.isBeeping()) {
            soundengine_.PlayWaveform(&beep_);
        }
        vm_.handleInterrupts();
    }

    return true;
}

void View::draw() {
    for (auto row = 0; row < SCREEN_HEIGHT; row++) {
        for (auto col = 0; col < SCREEN_WIDTH; col++) {
            Draw(col, row, vm_.pixelAt(row, col) ? olc::WHITE : olc::BLACK);
        }
    }
}

void View::handleInput() {
    for (auto& key : keys_) {
        if (GetKey(key.second).bPressed || GetKey(key.second).bHeld) {
            vm_.input(key.first, true);
        } else {
            vm_.input(key.first, false);
        }
    }
}

int main(int argc, const char* argv[]) {
    setlocale(LC_ALL, "POSIX");

#ifndef _WIN32
    struct sigaction act;
    act.sa_handler = system_end;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGABRT, &act, NULL);
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
#else
    signal(SIGABRT, system_end);
    signal(SIGINT, system_end);
    signal(SIGTERM, system_end);
#endif

    Chip8VM vm;

    if (argc > 1) {
        try {
            vm.load(argv[1]);
        } catch (...) {
            std::cerr << "Could not load " << argv[1] << '\n';
            return EXIT_FAILURE;
        }
    }

    View view(vm);

    if (view.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, SCALE, SCALE)) {
        view.Start();
    }

	return EXIT_SUCCESS;
}