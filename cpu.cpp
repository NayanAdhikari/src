#include "cpu.h"
#include <iostream>

static const uint8_t CYCLE_TABLE[256] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    7, 0, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0, // 0x00
    0, 0, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0x10
    6, 0, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0, // 0x20
    0, 0, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0x30
    6, 0, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0, // 0x40
    0, 0, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0x50
    6, 0, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0, // 0x60
    0, 0, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0x70
    0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0, // 0x80
    0, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0, // 0x90
    2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0, // 0xA0
    0, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0, // 0xB0
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0, // 0xC0
    0, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0xD0
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0, // 0xE0
    0, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 0xF0
};

// ----------------------------------------------------------------
// Core
// ----------------------------------------------------------------

void CPU::reset(Memory& mem)
{
    A = X = Y = 0;
    SP = 0xFF;
    P  = 0b00100000; // bit 5 hardwired to 1

    mem.reset();

    // Read PC from RESET vector at $FFFC/$FFFD
    uint8_t lo = mem.read(0xFFFC);
    uint8_t hi = mem.read(0xFFFD);
    PC = (hi << 8) | lo;
}

uint8_t CPU::fetch(Memory& mem)
{
    return mem.read(PC++);
}

void CPU::step(Memory& mem)
{
    uint8_t opcode = fetch(mem);
    execute(opcode, mem);
    cycles += CYCLE_TABLE[opcode]; // accurate per-instruction cycle count
}

// ----------------------------------------------------------------
// Addressing Modes
// ----------------------------------------------------------------

uint8_t CPU::immediate(Memory& mem)
{
    return fetch(mem);
}

uint8_t CPU::zeroPage(Memory& mem)
{
    uint8_t address = fetch(mem);
    return mem.read(address);
}

uint8_t CPU::zeroPageAddress(Memory& mem)
{
    return fetch(mem);
}

uint16_t CPU::absoluteAddress(Memory& mem)
{
    uint8_t lo = fetch(mem);
    uint8_t hi = fetch(mem);
    return (hi << 8) | lo;
}

uint8_t CPU::absolute(Memory& mem)
{
    return mem.read(absoluteAddress(mem));
}

// ----------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------

void CPU::setZN(uint8_t value)
{
    setZ(value == 0);
    setN((value & 0x80) != 0);
}

void CPU::push(Memory& mem, uint8_t value)
{
    mem.write(0x0100 + SP, value);
    SP--;
}

uint8_t CPU::pop(Memory& mem)
{
    SP++;
    return mem.read(0x0100 + SP);
}

// ----------------------------------------------------------------
// Execute
// ----------------------------------------------------------------

void CPU::execute(uint8_t opcode, Memory& mem)
{
    switch (opcode)
    {
        // --- Load / Store ---
        case 0xA9: LDA_IM(mem);  break;
        case 0xA5: LDA_ZP(mem);  break;
        case 0xAD: A = absolute(mem); setZN(A); break; // LDA ABS
        case 0x85: STA_ZP(mem);  break;
        case 0x8D: mem.write(absoluteAddress(mem), A); break; // STA ABS

        // --- Transfer ---
        case 0xAA: TAX(); break;

        // --- Increment / Decrement ---
        case 0xE8: INX(); break;
        case 0xCA: DEX(); break;
        case 0xC8: INY(); break;
        case 0x88: DEY(); break;

        // --- Arithmetic ---
        case 0x69: ADC_IM(mem); break;

        // --- Stack ---
        case 0x48: PHA(mem); break;
        case 0x68: PLA(mem); break;
        case 0x08: PHP(mem); break;
        case 0x28: PLP(mem); break;

        // --- Jumps / Calls ---
        case 0x20: JSR(mem);     break;
        case 0x60: RTS(mem);     break;
        case 0x4C: JMP_ABS(mem); break;

        // --- System ---
        case 0x00: BRK(mem); break;

        default:
            std::cout << "Unknown opcode: 0x"
                      << std::hex << (int)opcode << "\n";
            break;
    }
}

// ----------------------------------------------------------------
// Instructions
// ----------------------------------------------------------------

void CPU::LDA_IM(Memory& mem)
{
    A = immediate(mem);
    setZN(A);
}

void CPU::LDA_ZP(Memory& mem)
{
    A = zeroPage(mem);
    setZN(A);
}

void CPU::STA_ZP(Memory& mem)
{
    uint8_t address = zeroPageAddress(mem);
    mem.write(address, A);
}

void CPU::ADC_IM(Memory& mem)
{
    uint8_t  value  = immediate(mem);
    uint16_t result = A + value + getC();

    setC(result > 0xFF);
    setV((~(A ^ value) & (A ^ (uint8_t)result) & 0x80) != 0);

    A = result & 0xFF;
    setZN(A);
}

void CPU::TAX()
{
    X = A;
    setZN(X);
}

void CPU::INX()
{
    X++;
    setZN(X);
}

void CPU::DEX()
{
    X--;
    setZN(X);
}

void CPU::INY()
{
    Y++;
    setZN(Y);
}

void CPU::DEY()
{
    Y--;
    setZN(Y);
}

void CPU::PHA(Memory& mem)
{
    push(mem, A);
}

void CPU::PLA(Memory& mem)
{
    A = pop(mem);
    setZN(A);
}

void CPU::PHP(Memory& mem)
{
    // B flag is set in the pushed value but not in P itself
    push(mem, P | 0x10);
}

void CPU::PLP(Memory& mem)
{
    P = pop(mem);
    P |= 0x20;  // bit 5 always 1
    P &= ~0x10; // B flag cleared when pulled back
}

void CPU::JSR(Memory& mem)
{
    uint8_t lo = fetch(mem);
    uint8_t hi = fetch(mem);

    // Push return address minus 1 — this is the real 6502 behaviour
    uint16_t ret = PC - 1;
    push(mem, (ret >> 8) & 0xFF);
    push(mem, ret & 0xFF);

    PC = (hi << 8) | lo;
}

void CPU::RTS(Memory& mem)
{
    uint8_t lo = pop(mem);
    uint8_t hi = pop(mem);
    PC = ((hi << 8) | lo) + 1; // +1 undoes the JSR quirk
}

void CPU::JMP_ABS(Memory& mem)
{
    PC = absoluteAddress(mem);
}

void CPU::BRK(Memory& mem)
{
    PC++;                         // skip the padding byte after BRK
    push(mem, (PC >> 8) & 0xFF); // push PC high
    push(mem, PC & 0xFF);         // push PC low
    setB(true);
    push(mem, P);                 // push status with B set
    setI(true);                   // disable further IRQs

    uint8_t lo = mem.read(0xFFFE);
    uint8_t hi = mem.read(0xFFFF);
    PC = (hi << 8) | lo;

    isHalted = true;
    std::cout << "[BRK] Pushed state, jumped to IRQ vector.\n";
}