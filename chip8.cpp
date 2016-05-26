#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>

class chip8 {
    private:
        unsigned short opcode;
        unsigned char memory[4096];

        unsigned char V[16];

        unsigned short I;
        unsigned short pc;

        

        unsigned char delay_timer;
        unsigned char sound_timer;

        unsigned short stack[16];
        unsigned short sp;

        void init();

        

public:
        chip8();
        ~chip8();
        void initialize();
        void emulateCycle();
        bool loadApplication(const char *filename);
        
        bool drawFlag;
        unsigned char gfx[64 * 32];
        unsigned char key[16];
};

unsigned char chip8_fontset[80] =
{ 
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

chip8::chip8() {};

chip8::~chip8() {};

void chip8::initialize() {
    pc = 0x200; 
    opcode = 0;
    I = 0;
    sp = 0;

    // clear display
    for (int i = 0; i < 64 * 32; i++) {
        gfx[i] = 0;
    }

    // clear stack & registers
    for (int i = 0; i < 16; i++) {
        stack[i] = 0;
        V[i] = 0;
    }

    // clear memory
    for (int i = 0; i < 4096; i++) {
        memory[i] = 0;
    }


    // load fontset to memory
    for (int i = 0; i < 80; i++) {
        memory[i] = chip8_fontset[i];
    }

    // reset timers
    delay_timer = 0;
    sound_timer = 0;

    drawFlag = true;

}

void chip8::emulateCycle() {
    //fetch
    opcode = memory[pc] << 8 | memory[pc + 1];

    //decode
    switch(opcode & 0xF000) {
        case 0x000:
            switch (opcode & 0x000F) {
                case 0x0000: // Clears the screen
                    for (int i = 0; i < 32 * 64; i++) {
                        gfx[i] = 0;
                    }
                    drawFlag = true;
                    break; 
                    pc += 2;
                case 0x000E: // Returns from a subroutine
                    pc = stack[sp];
                    --sp;
                    pc += 2;
                    break; 
                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;
        case 0x1000: // Jumps to address NNN
            pc = opcode & 0x0FFF;
            break;
        case 0x2000: // Calls subroutine at NNN
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
            break;
        case 0x3000: // Skips the next instruction if VX equals NN
            if (V[opcode & 0x0F00] == opcode & 0x00FF) {
                pc +=2;
            }
            pc += 2;
            break;
        case 0x4000: // Skips the next instruction if VX doesn't equal NN.
            if (V[opcode & 0x0F00] != opcode & 0x00FF) {
                pc +=2;
            }
            pc += 2;
            break;
        case 0x5000: // Skips the next instruction if VX equals VY.
            if (V[opcode & 0x0F00] == V[opcode & 0x00F0]) {
                pc += 2;
            }
            pc += 2;
            break;
        case 0x6000: // Sets VX to NN
            V[opcode & 0x0F00] == opcode & 0x00FF;
            pc += 2;
            break;
        case 0x7000: // Adds NN to VX
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;
        case 0x8000:
            switch(opcode & 0x000F) {
                case 0x0000: // Sets VX to the value of VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0001: // Sets VX to VX or VY.
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0002: // Sets VX to VX and VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0003: // Sets VX to VX xor VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0004: // Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
                    if (V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4] > 0xFF) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0005: // VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                    if (V[(opcode & 0x0F00) >> 8] - V[(opcode & 0x00F0) >> 4] <= 0) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0006: // Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x01;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;
                case 0x0007: // Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                    if (V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8] <= 0) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x000E: // Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x80;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;
                default:
                    printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
            }
            break;
        case 0x9000: // Skips the next instruction if VX doesn't equal VY
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
                pc += 2;
            }
            pc += 2;
            break;
        case 0xA000: // Sets I to the address NNN.
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        case 0xB000: // Jumps to the address NNN plus V0
            pc = V[0x0] + (opcode & 0x0FFF);
            break;
        case 0xC000: // Sets VX to the result of a bitwise and operation on a random number and NN
            V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
            pc += 2;
            break; // 
        case 0xD000: // Sprites stored in memory at location in index register (I), 8bits wide. Wraps around the screen. If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen pixels). Sprites are drawn starting at position VX, VY. N is the number of 8bit rows that need to be drawn. If N is greater than 1, second line continues at position VX, VY+1, and so on.
            {
                unsigned short x = V[(opcode & 0x0F00) >> 8];
                unsigned short y = V[(opcode & 0x00F0) >> 4];
                unsigned short height = opcode & 0x000F;
                unsigned short pixle;

                V[0xF] = 0;
                for (int yline = 0; yline < height; yline++) {

                    pixle = memory[I + yline]; // early memory is Sprites, first ones are fontset
                    for (int xline = 0; xline < 8; xline++) {

                        if ((pixle & (0x80 >> xline)) != 0) {

                            if (gfx[(x + xline + ((y + yline) * 64))] == 1) {
                                V[0xF] = 1;
                            }
                            gfx[(x + xline + ((y + yline) * 64))] ^= 1; //flips the bit
                        }
                    }
                }
            }
            drawFlag = true;
            pc += 2;
            break;
        case 0xE000:
            switch (opcode & 0x000F) {
                case 0x000E: // Skips the next instruction if the key stored in VX is pressed
                    if (key[(opcode & 0x0F00) >> 8] == 1) {
                        pc += 2;
                    }
                    pc += 2;
                    break;
                case 0x0001: // Skips the next instruction if the key stored in VX isn't pressed
                    if (key[(opcode & 0x0F00) >> 8] == 0) {
                        pc += 2;
                    }
                    pc += 2;
                    break;
                default:
                    printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: // Sets VX to the value of the delay timer
                    V[(opcode & 0x0F00) >> 8] = delay_timer;
                    pc += 2;
                    break;
                case 0x000A: // A key press is awaited, and then stored in VX
                    {
                        bool keypress = false;

                        for (int i = 0; i < 16; i++) {
                            if (key[i] != 0) {
                                V[(opcode & 0x0F00) >> 8] = i;
                                keypress = true;
                            }
                        }

                        if (!keypress) {
                            return;
                        }
                    }
                    pc += 2;
                    break;
                case 0x0015: // Sets the delay timer to VX
                    delay_timer = V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0018: // Sets the sound timer to VX
                    sound_timer = V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0029: // Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
                    if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x001E: // Adds VX to I
                    if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x0033: // Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2
                    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I+1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I+2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;
                    break;
                case 0x0055: // Stores V0 to VX (including VX) in memory starting at address I
                    for (int i = 0; i < ((opcode & 0x0F00) >> 8); i++) {
                        memory[I+i] = V[i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;
                case 0x0065: // Fills V0 to VX (including VX) with values from memory starting at address I
                    for (int i = 0; i < ((opcode & 0x0F00) >> 8); i++) {
                        V[i] = memory[I+i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;
                default:
                    printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
            }
            break;
        default:
            printf("Unknown opcode: 0x%X\n", opcode);

    }

    // Update timers
    if (delay_timer > 0) {
        delay_timer--;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            printf("Beep!\n");
        }
        sound_timer--;
    }
}

bool chip8::loadApplication(const char *filename) {
    initialize();

    std::fstream file;
    file.open(filename, std::ios::binary | std::ios::in | std::ios::ate);
    long fsize = (long) file.tellg();
    file.seekg(0, std::ios::beg);

    char *buffer = (char*)malloc(fsize*sizeof(char));
    if (buffer == NULL) {
        printf("Memory Error\n");
        return false;
    }

    file.read(buffer, fsize);
  

    if ((4096 - 512) > fsize) {
        for (int i = 0; i < fsize; i++) {
            memory[i + 512] = buffer[i];
        }
    } else {
        printf("File too large\n");
        return false;
    }

    file.close();
    free(buffer);

    return true;

}

int main() {
    return 1;
}



