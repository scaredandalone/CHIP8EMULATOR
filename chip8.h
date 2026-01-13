#pragma once
#include <cstdint>
#include <fstream>
#include <vector>
#include <iostream>


class Chip8 {
private:
    uint8_t memory[4096];      // 4KB RAM
    uint8_t V[16];              // 16 registers (V0-VF)
    uint16_t I;                 // Index register
    uint16_t pc;                // Program counter (where we are in memory)
    uint8_t display[64][32];    // Screen pixels (on/off)
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint16_t stack[16];         // For subroutines
    uint8_t sp;                 // Stack pointer
    uint8_t keys[16];           // Keypad state

    bool drawFlag;

public:
    void initialize();
    void loadROM(const char* filename);
    void cycle();               // One fetch-decode-execute cycle


    // getters
    uint8_t getPixel(int x, int y) { return display[x][y]; }
    bool shouldDraw() { return drawFlag; }
    void clearDrawFlag() { drawFlag = false; }
};