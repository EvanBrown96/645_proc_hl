#include "z80.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "register.h"

// Main Memory
uint8_t *memory = (uint8_t*) malloc((0xFFFF) * sizeof(uint8_t));

// Registers
Register AF(0x01B0); // AAAAAAAAZNHCxxxx
Register BC(0x0804); // BBBBBBBBCCCCCCCC
Register DE(0x0201); // DDDDDDDDEEEEEEEE
Register HL(0x0000); // HHHHHHHHLLLLLLLL
Register SP(0xFFFE); // Stack Pointer
Register PC(0x0100); // Program Counter

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

uint8_t readByte(unsigned int location) {
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

    int instructions_4_cycles[] = {
      0x04, 0x05, 0x0C, 0x0D, 0x14, 0x15, 0x1C, 0x1D, 0x24, 0x25, 0x2C, 0x2D,
      0x3C, 0x3D, 0x41, 0x4A, 0x53, 0x78, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85,
      0x87, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x97, 0xA0, 0xA1, 0xA2, 0xA3,
      0xA4, 0xA5, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAF, 0xB0, 0xB1,
      0xB2, 0xB3, 0xB4, 0xB5, 0xB7
    };

    for(int i = 0; i < 53; i++){
      clockCycles[instructions_4_cycles[i]] = 4;
    }

    int instructions_7_cycles[] = {
      0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x3E, 0x46, 0x70
    };

    for(int i = 0; i < 9; i++){
      clockCycles[instructions_7_cycles[i]] = 7;
    }

    int instructions_10_cycles[] = {
      0x21, 0x36
    };

    for(int i = 0; i < 2; i++){
      clockCycles[instructions_10_cycles[i]] = 10;
    }

    int instructions_11_cycles[] = {
      0x34, 0x35
    };

    for(int i = 0; i < 2; i++){
      clockCycles[instructions_11_cycles[i]] = 11;
    }
}

void z80::loadTest() {
    writeByte(0x0100, 0x04);
    writeByte(0x0101, 0x05);
    writeByte(0x0102, 0x0C);
    writeByte(0x0103, 0x0D);
    writeByte(0x0104, 0x14);
    writeByte(0x0105, 0x15);
    writeByte(0x0106, 0x1C);
    writeByte(0x0107, 0x1D);
    writeByte(0x0108, 0x24);
    writeByte(0x0109, 0x25);
    writeByte(0x010A, 0x2C);
    writeByte(0x010B, 0X2D);
    writeByte(0x010C, 0x3C);
    writeByte(0x010D, 0x3D);
    // writeByte(0x010E
    // writeByte(0x010F
    // writeByte(0x0110
    // writeByte(0x0111
    // writeByte(0x0112
    // writeByte(0x0113
    // writeByte(0x0114
    // writeByte(0x0115
    // writeByte(0x0116
    // writeByte(0x0117
    // writeByte(0x0118
    // writeByte(0x0119
    // writeByte(0x011A


    writeByte(0x010E, 0x76);
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

void z80::loadSecond() {
 writeByte(0x0100, 0x3C);
 writeByte(0x0101, 0x3C);
 writeByte(0x0102, 0x53);
 writeByte(0x0103, 0x4A);
 writeByte(0x0104, 0x41);
 writeByte(0x0105, 0x78);
 writeByte(0x0106, 0x06);
 writeByte(0x0107, 0x47);
 writeByte(0x0108, 0x26);
 writeByte(0x0109, 0x02);
 writeByte(0x010A, 0x2E);
 writeByte(0x010B, 0x00);
 writeByte(0x010C, 0x36);
 writeByte(0x010D, 0x99);
 writeByte(0x010E, 0x34);
 writeByte(0x010F, 0x34);
 writeByte(0x010F, 0x46);
 writeByte(0x0110, 0x76);
}


uint8_t fetch() {
    //read the next byte and increment the program counter
    uint8_t byte = readByte(PC.getFull());
    PC++;
    return byte;
}

uint16_t readWord()
{
    // little endian
    uint8_t low = readByte(PC.getFull());
    PC++;
    uint8_t high = readByte(PC.getFull());
    PC++;
    return low | (high << 8);
}

void printRegisters() {
    printf("PC: %04x, AF: %04x, BC: %04x, DE: %04x, HL: %04x, SP: %04x\n",
     PC.getFull(), AF.getFull(), BC.getFull(), DE.getFull(), HL.getFull(), SP.getFull());
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

    // INC B
    case 0x04:
      BC.incHigh();
      break;

    // DEC B
    case 0x05:
      BC.decHigh();
      break;

    // LD B, *
    case 0x06:
      BC.setHigh(fetch());
      break;

    // INC C
    case 0x0C:
      BC.incLow();
      break;

    // DEC C
    case 0x0D:
      BC.decLow();
      break;

    // LD C, *
    case 0x0E:
      BC.setLow(fetch());
      break;

    // INC D
    case 0x14:
      DE.incHigh();
      break;

    // DEC D
    case 0x15:
      DE.decHigh();
      break;

    // LD D, *
    case 0x16:
      DE.setHigh(fetch());
      break;

    // INC E
    case 0x1C:
      DE.incLow();
      break;

    // DEC E
    case 0x1D:
      DE.decLow();
      break;

    // LD E, *
    case 0x1E:
      DE.setLow(fetch());
      break;

    // LD HL, **
    case 0x21:
      HL.setFull(readWord());
      break;

    // INC H
    case 0x24:
      HL.incHigh();
      break;

    // DEC H
    case 0x25:
      HL.decHigh();
      break;

    // LD H, *
    case 0x26:
      HL.setHigh(fetch());
      break;

    // INC L
    case 0x2C:
      HL.incLow();
      break;

    // DEC L
    case 0x2D:
      HL.decLow();
      break;

    // LD L, *
    case 0x2E:
      HL.setLow(fetch());
      break;

    // INC (HL)
    case 0x34: {
      uint8_t data = readByte(HL.getFull()) + 1;
      writeByte(HL.getFull(), data);
    } break;

    // DEC (HL)
    case 0x35: {
      uint8_t data = readByte(HL.getFull()) - 1;
      writeByte(HL.getFull(), data);
    } break;

    // LD (HL), *
    case 0x36:
      writeByte(HL.getFull(), fetch());
      break;

    // INC A
    case 0x3C:
      AF.incHigh();
      //ignore setting flags for now
      break;

    // DEC A
    case 0x3D:
      AF.decHigh();
      break;

    // LD A, *
    case 0x3E:
      AF.setHigh(fetch());
      break;

    // LD B, C
    case 0x41:
      BC.setHigh(BC.getLow());
      break;

    // LD B, (HL)
    case 0x46:
      BC.setHigh(readByte(HL.getFull()));
      break;

    // LD C, D
    case 0x4A:
      BC.setLow(DE.getHigh());
      break;

    // LD D, E
    case 0x53:
      DE.setHigh(DE.getLow());
      break;

    // LD (HL), B
    case 0x70:
      writeByte(HL.getFull(), BC.getHigh());
      break;

    // HALT
    case 0x76:
      halted = 1;
      break;

    // LD A, B
    case 0x78:
      AF.setHigh(BC.getHigh());
      break;

    // ADD B
    case 0x80:
      AF.setHigh(AF.getHigh() + BC.getHigh());
      //ignore setting flags for now
      break;

    // ADD C
    case 0x81:
      AF.setHigh(AF.getHigh() + BC.getLow());
      //ignore setting flags for now
      break;

    // ADD D
    case 0x82:
      AF.setHigh(AF.getHigh() + DE.getHigh());
      //ignore setting flags for now
      break;

    // ADD E
    case 0x83:
      AF.setHigh(AF.getHigh() + DE.getLow());
      //ignore setting flags for now
      break;

    // ADD H
    case 0x84:
      AF.setHigh(AF.getHigh() + HL.getHigh());
      //ignore setting flags for now
      break;

    // ADD L
    case 0x85:
      AF.setHigh(AF.getHigh() + HL.getLow());
      //ignore setting flags for now
      break;

    // ADD A
    case 0x87:
      AF.setHigh(AF.getHigh() + AF.getHigh());
      //ignore setting flags for now
      break;

    // SUB B
    case 0x90:
      AF.setHigh(AF.getHigh() - BC.getHigh());
      //ignore setting flags for now
      break;

    // SUB C
    case 0x91:
      AF.setHigh(AF.getHigh() - BC.getLow());
      //ignore setting flags for now
      break;

    // SUB D
    case 0x92:
      AF.setHigh(AF.getHigh() - DE.getHigh());
      //ignore setting flags for now
      break;

    // SUB E
    case 0x93:
      AF.setHigh(AF.getHigh() - DE.getLow());
      //ignore setting flags for now
      break;

    // SUB H
    case 0x94:
      AF.setHigh(AF.getHigh() - HL.getHigh());
      //ignore setting flags for now
      break;

    // SUB L
    case 0x95:
      AF.setHigh(AF.getHigh() - HL.getLow());
      //ignore setting flags for now
      break;

    // SUB A
    case 0x97:
      AF.setHigh(AF.getHigh() - AF.getHigh());
      //ignore setting flags for now
      break;

    // AND B
    case 0xA0:
      AF.setHigh(AF.getHigh() & BC.getHigh());
      break;

    // AND C
    case 0xA1:
      AF.setHigh(AF.getHigh() & BC.getLow());
      break;

    // AND D
    case 0xA2:
      AF.setHigh(AF.getHigh() & DE.getHigh());
      break;

    // AND E
    case 0xA3:
      AF.setHigh(AF.getHigh() & DE.getLow());
      break;

    // AND H
    case 0xA4:
      AF.setHigh(AF.getHigh() & HL.getHigh());
      break;

    // AND L
    case 0xA5:
      AF.setHigh(AF.getHigh() & HL.getLow());
      break;

    // AND A
    case 0xA7:
      AF.setHigh(AF.getHigh() & AF.getHigh());
      break;

    // XOR B
    case 0xA8:
      AF.setHigh(AF.getHigh() ^ BC.getHigh());
      break;

    // XOR C
    case 0xA9:
      AF.setHigh(AF.getHigh() ^ BC.getLow());
      break;

    // XOR D
    case 0xAA:
      AF.setHigh(AF.getHigh() ^ DE.getHigh());
      break;

    // XOR E
    case 0xAB:
      AF.setHigh(AF.getHigh() ^ DE.getLow());
      break;

    // XOR H
    case 0xAC:
      AF.setHigh(AF.getHigh() ^ HL.getHigh());
      break;

    // XOR L
    case 0xAD:
      AF.setHigh(AF.getHigh() ^ HL.getLow());
      break;

    // XOR A
    case 0xAF:
      AF.setHigh(AF.getHigh() ^ AF.getHigh());
      break;

    // OR B
    case 0xB0:
      AF.setHigh(AF.getHigh() | BC.getHigh());
      break;

    // OR C
    case 0xB1:
      AF.setHigh(AF.getHigh() | BC.getLow());
      break;

    // OR D
    case 0xB2:
      AF.setHigh(AF.getHigh() | DE.getHigh());
      break;

    // OR E
    case 0xB3:
      AF.setHigh(AF.getHigh() | DE.getLow());
      break;

    // OR H
    case 0xB4:
      AF.setHigh(AF.getHigh() | HL.getHigh());
      break;

    // OR L
    case 0xB5:
      AF.setHigh(AF.getHigh() | HL.getLow());
      break;

    // OR A
    case 0xB7:
      AF.setHigh(AF.getHigh() | AF.getHigh());
      break;

    default:
      printf("Instruction %02x not valid (at %04x)\n\n", instruction, PC.getFull() - 1);
      halt();
      break;
  }

}
