#pragma once
#include <cstdint>
#include "memory.h"

struct CPU {
    bool isHalted = false;

    // Registers
    uint8_t  A  = 0;
    uint8_t  X  = 0;
    uint8_t  Y  = 0;
    uint8_t  SP = 0;
    uint16_t PC = 0;
    uint64_t cycles = 0;

    // Processor Status Register
    // Bit layout: N V - B D I Z C
    //             7 6 5 4 3 2 1 0
    // Bit 5 is hardwired to 1 always.
    uint8_t P = 0b00100000;

    void reset(Memory& mem);
    void step(Memory& mem);

private:

    // --- P register accessors ---
    uint8_t getC() const { return (P >> 0) & 1; }
    uint8_t getZ() const { return (P >> 1) & 1; }
    uint8_t getI() const { return (P >> 2) & 1; }
    uint8_t getD() const { return (P >> 3) & 1; }
    uint8_t getB() const { return (P >> 4) & 1; }
    uint8_t getV() const { return (P >> 6) & 1; }
    uint8_t getN() const { return (P >> 7) & 1; }

    void setC(bool v) { v ? (P |= 0x01) : (P &= ~0x01); }
    void setZ(bool v) { v ? (P |= 0x02) : (P &= ~0x02); }
    void setI(bool v) { v ? (P |= 0x04) : (P &= ~0x04); }
    void setD(bool v) { v ? (P |= 0x08) : (P &= ~0x08); }
    void setB(bool v) { v ? (P |= 0x10) : (P &= ~0x10); }
    void setV(bool v) { v ? (P |= 0x40) : (P &= ~0x40); }
    void setN(bool v) { v ? (P |= 0x80) : (P &= ~0x80); }

    // --- Core ---
    uint8_t fetch(Memory& mem);
    void execute(uint8_t opcode, Memory& mem);

    // --- Helpers ---
    void setZN(uint8_t value);
    void push(Memory& mem, uint8_t value);
    uint8_t pop(Memory& mem);

    // --- Addressing Modes ---
    uint8_t  immediate(Memory& mem);
    uint8_t  zeroPage(Memory& mem);
    uint8_t  zeroPageAddress(Memory& mem);
    uint16_t absoluteAddress(Memory& mem);   // NEW
    uint8_t  absolute(Memory& mem);          // NEW

    // --- Instructions ---
    void LDA_IM(Memory& mem);
    void LDA_ZP(Memory& mem);
    void STA_ZP(Memory& mem);
    void ADC_IM(Memory& mem);
    void PHA(Memory& mem);
    void PLA(Memory& mem);
    void PHP(Memory& mem);    // NEW
    void PLP(Memory& mem);    // NEW
    void JSR(Memory& mem);    // NEW
    void RTS(Memory& mem);    // NEW
    void JMP_ABS(Memory& mem); // NEW
    void BRK(Memory& mem);    // CHANGED — now takes Memory&
    void TAX();
    void INX();
    void DEX();
    void INY();
    void DEY();
};