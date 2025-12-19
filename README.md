# RISC-V Processor Simulator

```
╔═══════════════════════════════════════════════════════════════════╗
║                                                                   ║
║              RISC-V INSTRUCTION SET SIMULATOR                     ║
║                                                                   ║
║          Decode, Execute, and Trace RISC-V Instructions           ║
║                                                                   ║
╚═══════════════════════════════════════════════════════════════════╝
```

## About

This project is a RISC-V processor simulator that can decode and execute RISC-V assembly instructions. It reads machine code instructions and simulates how a real RISC-V processor would execute them, including register operations, memory access, and control flow.

## Technologies Used

- C Programming Language
- RISC-V Instruction Set Architecture
- CUnit Testing Framework
- GNU Make Build System
- Python for Testing Scripts

## Features

- Instruction decoding for all major RISC-V instruction formats
- Register file simulation with 32 registers
- Memory load and store operations
- Branch and jump instruction support
- Step-by-step execution tracing
- Interactive debugging mode

## Building

```bash
make all
```

## Running

Execute RISC-V programs:
```bash
./riscv code/input/simple.input
```

Disassemble instructions:
```bash
./riscv -d code/input/simple.input
```

## Project Structure

- `part1.c` - Instruction decoder implementation
- `part2.c` - Instruction executor implementation
- `riscv.c` - Main simulator loop and utilities
- `utils.c` - Helper functions for instruction parsing
- `types.h` - Data type definitions
- `code/input/` - Test input files
- `code/ref/` - Reference solutions for testing
