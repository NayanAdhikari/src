#include <iostream>
#include <vector>
#include "cpu.h"
#include "memory.h"

void loadProgram(Memory& mem, uint16_t start, const std::vector<uint8_t>& program)
{
    for (size_t i = 0; i < program.size(); i++)
    {
        mem.write(start + i, program[i]);
    }
}

int main()
{
    Memory mem;
    CPU cpu;

    // Load program BEFORE reset so mem.reset() inside reset() doesn't wipe it
    loadProgram(mem, 0x0200, {
        0xA9, 0x42, // LDA #$42
        0x48,       // PHA
        0xA9, 0x00, // LDA #$00
        0x68,       // PLA
        0x00        // BRK
    });

    // Write RESET vector — tells the CPU where to start
    mem.write(0xFFFC, 0x00); // lo byte of $0200
    mem.write(0xFFFD, 0x02); // hi byte of $0200

    // NOW reset — reads the vector we just wrote
    cpu.reset(mem);

    while (!cpu.isHalted)
    {
        cpu.step(mem);

        std::cout << std::hex
                  << "A=0x"   << (int)cpu.A
                  << " X=0x"  << (int)cpu.X
                  << " Y=0x"  << (int)cpu.Y
                  << " SP=0x" << (int)cpu.SP
                  << " PC=0x" << (int)cpu.PC
                  << " P=0x"  << (int)cpu.P
                  << " CYC="  << std::dec << cpu.cycles
                  << "\n";
    }

    return 0;
}