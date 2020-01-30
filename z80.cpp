#include "z80.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Some useful macros

//send a byte to a specific spot in a word
#define bytetoHigh(word, byte) (((byte) << 8) | ((word) & 0x00FF))
#define bytetoLow(word, byte) ((byte) | ((word) & 0xFF00))

//send a byte from a word to a specific spot in a word
#define HightoHigh(dstword, srcword) (((srcword) & 0xFF00) | ((dstword) & 0x00FF))
#define LowtoHigh(dstword, srcword) (((srcword) << 8 ) | ((dstword) & 0x00FF))
#define HightoLow(dstword, srcword) (((srcword) >> 8 ) | ((dstword) & 0xFF00))
#define LowtoLow(dstword, srcword) (((srcword) & 0x00FF ) | ((dstword) & 0xFF00))


// Main Memory
uint8_t *memory = (uint8_t*) malloc((0xFFFF) * sizeof(uint8_t));

// Registers
uint16_t AF = 0x01B0; // AAAAAAAAZNHCxxxx
uint16_t BC = 0x0804; // BBBBBBBBCCCCCCCC
uint16_t DE = 0x0201; // DDDDDDDDEEEEEEEE
uint16_t HL = 0x0000; // HHHHHHHHLLLLLLLL
uint16_t SP = 0xFFFE; // Stack Pointer
uint16_t PC = 0x0100; // Program Counter

// Clocks per instruction lookup table
uint8_t *clockCycles = (uint8_t*) malloc((0xFF) * sizeof(uint8_t));

// Turn on
volatile bool z80::cpuPower = false;

// Keep count of cycles
volatile uint64_t z80::totalClockCycles = 0;

// Keep count of instructions per type
uint32_t *instructionsCount = (uint32_t*) malloc((0xFF) * sizeof(uint32_t));

// Initial instruction
uint8_t instruction = 0x00;

// HALT
bool halted = 0;

void writeByte(unsigned int location, uint8_t data) {
    memory[location] = data;
}

uint8_t readByte(unsigned int location)
{
   return memory[location];
}

void z80::initMemory() {
    int i;
    // Reset memory to zero
    memset(memory, 0x00, 0xFFFF);
    // default clocks per instruction to 1
    for (i=0; i<255;i++) {
      clockCycles[i]=1;
      instructionsCount[i]=0;
    }

    int instructions_4_cycles[51] = {
      0x03, 0x04, 0x05, 0x0C, 0x0D, 0x14, 0x15,
      0x1C, 0x1D, 0x24, 0x25, 0x2C, 0x2D, 0x3C, 0x3D, 0x76, 0x80, 0x81, 0x82,
      0x83, 0x84, 0x85, 0x87, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x97, 0xA0,
      0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD,
      0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB7
    };

    for(int i = 0; i < 51; i++){
      clockCycles[instructions_4_cycles[i]] = 4;
    }
}

void z80::loadTest() {
    writeByte(0x0100, 0x3C);
    writeByte(0x0101, 0x3C);
    writeByte(0x0102, 0x3C);
    writeByte(0x0103, 0x80);
    writeByte(0x0104, 0x76);
}

void z80::loadFirst() {
    writeByte(0x0100, 0x3C);
    writeByte(0x0101, 0x3C);
    writeByte(0x0102, 0x3C);
    writeByte(0x0103, 0x80);
    writeByte(0x0104, 0xA0);
    writeByte(0x0105, 0xB3);
    writeByte(0x0106, 0x1C);
    writeByte(0x0107, 0xB3);
    writeByte(0x0108, 0xA1);
    writeByte(0x0109, 0x0C);
    writeByte(0x010A, 0xA1);
    writeByte(0x010B, 0xAA);
    writeByte(0x010C, 0x15);
    writeByte(0x010D, 0xAA);
    writeByte(0x010E, 0x76);
}


uint8_t fetch() {
    //read the next byte and increment the program counter
    return readByte(PC++);
}

uint16_t readWord()
{
    // little endian
    uint8_t low = readByte(PC++);
    uint8_t high = readByte(PC++);
    return low | (high << 8);
}

void printRegisters() {
    printf("PC: %04x, AF: %04x, BC: %04x, DE: %04x, HL: %04x, SP: %04x\n", PC, AF, BC, DE, HL, SP);
}

void halt() {
    int i=0;
    printf("Total Clock Cycles: %lu\n", z80::totalClockCycles);
    for (i=0;i<255; i++){
	     if (instructionsCount[i]) {
         printf("Instruction 0x%02x count is  %04x\n", i, instructionsCount[i]);
	     }
	  }
    printf("Halting now.\n");
    exit(1);
}

void z80::cpuStep() {
    uint8_t n, n1, n2, interrupt;
    int8_t sn;
    uint16_t nn, nn1, nn2;
    bool c;

    if (!cpuPower) return;
    printRegisters();
    printf("totalClockCycles at %08lu\n", totalClockCycles);

    // Check if halted
    if (halted) {
        halt();
    }

    //fetch
    instruction = fetch();
    printf("instruction = %04x\n ", instruction);

    //decode
    totalClockCycles += clockCycles[instruction];
    instructionsCount[instruction]++;
    switch (instruction)
    {
    // NOP
    case 0x0:
        break;

    // HALT
    case 0x76:
        halted = 1;
        break;

    // INC A    Example for increments a particular byte
    case 0x3C:
	      AF=HightoHigh(AF, AF+0x0100);
        //ignore setting flags for now
        break;

    // INC BC    Example for increments a word
    case 0x03:
	BC++;
        //ignore setting flags for now
        break;

    // ADD A,B   Add  byte to a byte  result in A
    case 0x80:
        AF = bytetoHigh(AF, (AF >> 8) + (BC >> 8));
        //ignore setting flags for now
        break;

    default:
        printf("Instruction %02x not valid (at %04x)\n\n", instruction, PC - 1);
        halt();
        break;
    }

}
