#include "chip8.h"

void Chip8::initialize() {
    pc = 0x200;  // start reading from 0x200
    I = 0;
    sp = 0;
    delayTimer = 0;
    soundTimer = 0;
    drawFlag = false;

    // Clear everything
    for (int i = 0; i < 4096; i++) memory[i] = 0;
    for (int i = 0; i < 16; i++) V[i] = 0;
    for (int i = 0; i < 16; i++) stack[i] = 0;
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            display[x][y] = 0;
        }
    }
}

void Chip8::loadROM(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Failed to open ROM" << std::endl;
        return;
    }


    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        for (long i = 0; i < size; i++) {
            memory[0x200 + i] = buffer[i];
        }
    }

    file.close();
}

void Chip8::cycle()
{
    // FETCH: Read instruction (2 bytes)
    uint16_t opcode = memory[pc] << 8 | memory[pc + 1];
    pc += 2;
    printf("PC: 0x%03X, Opcode: 0x%04X\n", pc - 0x200, opcode);

    switch (opcode & 0xF000)
{

/* =========================================================
   SYSTEM / FLOW CONTROL
   ========================================================= */

/* 00E0 / 00EE */
case 0x0000:
    switch (opcode & 0x00FF)
    {
    case 0x00E0:  // CLS
        for (int x = 0; x < 64; x++)
            for (int y = 0; y < 32; y++)
                display[x][y] = 0;

        drawFlag = true;
        break;

    case 0x00EE:  // RET
        pc = stack[--sp];
        break;
    }
    break;

/* 1NNN — Jump */
case 0x1000:
    pc = opcode & 0x0FFF;
    break;

/* 2NNN — Call subroutine */
case 0x2000:
    stack[sp++] = pc;
    pc = opcode & 0x0FFF;
    break;

/* 3XNN - Skip if VX == NN */
case 0x3000:
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;

    if (V[x] == nn)
        pc += 2;

    break;
}
/* 4XNN - Skip if VX != NN */
case 0x4000:
{
    {
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t nn = opcode & 0x00FF;

        if (V[x] != nn)
            pc += 2;

        break;
    }
}
/* 5XY0 - Skip if VX == VY */
case 0x5000:
{
    {
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t y = (opcode & 0x00F0) >> 4;

        if (V[x] == V[y])
            pc += 2;

        break;
    }
}
/* 9XY0 - Skip if VX != VY */
case 0x9000:
{
    {
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t y = (opcode & 0x00F0) >> 4;

        if (V[x] != V[y])
            pc += 2;


        break;
    }
}

case 0xE000:
{
    {
        uint8_t x = (opcode & 0X0F00) >> 8;

        switch (opcode & 0x00FF) {

        /* EX9E - Skip if key VX is pressed */
        case(0x9E):
            {
            if (keys[V[x]]) {
                pc += 2;
            }
            break;

            }
        /* EXA1 - Skip if key VX is not pressed */
        case(0xA1):
        {
            if (!keys[V[x]]) {
                pc += 2;
            }
            break;

            }
        }
        break;

    }
}

/* =========================================================
   REGISTER / STATE SETUP
   ========================================================= */

/* 6XNN — Set VX = NN */
case 0x6000:
{
    V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
    break;
}
/* 7XNN - Add NN to VX */
case 0x7000:
{
    uint8_t x = (opcode & 0X0F00) >> 8;

    V[x] += opcode & 0x00FF;
    break;
}
/* 8XY0 - 8XYE  */
case 0x8000:
{
    {
        uint8_t x = (opcode & 0X0F00) >> 8;
        uint8_t y = (opcode & 0X00F0) >> 4;

        switch (opcode & 0x000F) {
        /* 8XY0 - Set VX = VY */
        case(0x0):
        {
            V[x] = V[y];
            break;

        }
        /* 8XY1 - Set VX = VX OR VY */
        case(0x1):
        {
            V[x] = V[x] | V[y];
            break;

        }
        /* 8XY2 - Set VX = VX AND VY */
        case(0x2):
        {
            V[x] = V[x] & V[y];
            break;

        }
        /* 8XY3 - Set VX = VX XOR VY */
        case(0x3):
        {
            V[x] = V[x] ^ V[y];
            break;

        }
        /* 8XY4 - Add VY to VX (set VF = carry) */
        case(0x4):
        {
          
            uint16_t temp = V[x] + V[y];

            V[0xF] = (temp & 0x100) != 0; // set carry flag if 9th bit is set
            V[x] = temp & 0xFF;           // set V[x] to bottom 8 bits
            break;

        }
        /* 8XY5 - Subtract VY from VX (set VF = NOT borrow) */
        case(0x5):
        {
            V[0xF] = (V[x] >= V[y]);
            V[x] = (V[x] - V[y]) & 0xFF;           // set V[x] to bottom 8 bits
            break;

        }
        /* 8XY6 - Shift VX right by 1 (VF = LSB before shift) */
        case(0x6):
        {
            V[0xF] = (V[x] & 1);
            V[x] >>= 1;
            break;
        }
        /* 8XY7 - Set VX = VY - VX (set VF = NOT borrow) */
        case(0x7): 
        {
            V[0xF] = (V[x] <= V[y]);
            V[x] = (V[y] - V[x]) & 0xFF;           // set V[x] to bottom 8 bits
            break;
        }
        /* 8XYE - Shift VX left by 1 (VF = MSB before shift) */
        case(0xE):
        {
            V[0xF] = (V[x] & 0x80) >> 7;
            V[x] <<= 1;
            break;
        }
        }
        break;

    }

}

/* CXNN (0xC000) - Set VX = random byte AND NN */
case 0xC000:
{
    uint8_t x = (opcode & 0x0F00) >> 8;  
    uint8_t nn = opcode & 0x00FF;        
    V[x] = (rand() % 256) & nn;          
    break;
}

/* ANNN — Set I */
case 0xA000:
{
    I = opcode & 0x0FFF;
    break;
}



/* =========================================================
   JUMPS WITH OFFSET
   ========================================================= */

/* BNNN — Jump to NNN + V0 */
case 0xB000:
    pc = (opcode & 0x0FFF) + V[0];
    break;


/* =========================================================
   GRAPHICS
   ========================================================= */

/* DXYN — Draw sprite */
case 0xD000:
{
    uint8_t x = V[(opcode & 0x0F00) >> 8];
    uint8_t y = V[(opcode & 0x00F0) >> 4];
    uint8_t height = opcode & 0x000F;

    V[0xF] = 0;

    for (int row = 0; row < height; row++)
    {
        uint8_t spriteData = memory[I + row];

        for (int col = 0; col < 8; col++)
        {
            if (spriteData & (0x80 >> col))
            {
                int pixelX = (x + col) % 64;
                int pixelY = (y + row) % 32;

                if (display[pixelX][pixelY])
                    V[0xF] = 1;

                display[pixelX][pixelY] ^= 1;
            }
        }
    }

    drawFlag = true;
    break;
}

/* =========================================================
   TIMER & MEMORY STUFF
   ========================================================= */


case 0xF000: {
    uint8_t x = (opcode & 0X0F00) >> 8;
    uint8_t y = (opcode & 0X00F0) >> 4;

    switch (opcode & 0x00FF) {
       
    /* FX07 - Set VX = delay timer value */
    case(0x07):
    {
        V[x] = delayTimer;
        break;
    }
    /* FX0A - Wait for key press, store in VX */
    case 0x0A: {
        bool keyPressed = false;
        for (int i = 0; i < 16; i++) {
            if (keys[i]) {
                V[x] = i;
                keyPressed = true;
                break;
            }
        }

        
        if (!keyPressed) {
            pc -= 2;
        }
        break;
    }
    /* FX15 - Set delay timer = VX */
    case(0x15): {
        delayTimer = V[x];
        break;
    }
    /* FX18 - Set sound timer = VX */
    case(0x18): {
        soundTimer = V[x];
        break;
    }
    /* FX1E - Add VX to I (set I = I + VX) */
    case(0x1E): {
        I += V[x];
        break;
    }
    /* FX29 - Set I = location of sprite for digit VX */
    case(0x29): {
        I = V[x] * 5;
        break;
    }
    /* FX33 - Store BCD representation of VX in memory[I], memory[I+1], memory[I+2] */
    case(0x33): {
        memory[I] = V[x] / 100;            // Hundreds digit
        memory[I + 1] = (V[x] / 10) % 10;  // Tens digit
        memory[I + 2] = V[x] % 10;         // Ones digit
        break;
    }
    /* FX55 - Store V0 through VX in memory starting at I */
    case(0x55): {
        for (int i = 0; i <= x; ++i) {
            memory[I + i] = V[i];
        }
        break;
    }
    /* FX65 - Load V0 through VX from memory starting at I */
    case(0x65): {
        for (int i = 0; i <= x; ++i) {
            V[i] = memory[I + i];
        }
        break;
    }
    }

}




}

    // Update timers
    if (delayTimer > 0) delayTimer--;
    if (soundTimer > 0) soundTimer--;
}