#include <stdio.h> // for stderr
#include <stdlib.h> // for exit()
#include "types.h"
#include "utils.h"
#include "riscv.h"

void execute_rtype(Instruction, Processor *);
void execute_itype_except_load(Instruction, Processor *);
void execute_branch(Instruction, Processor *);
void execute_jal(Instruction, Processor *);
void execute_load(Instruction, Processor *, Byte *);
void execute_store(Instruction, Processor *, Byte *);
void execute_ecall(Processor *, Byte *);
void execute_lui(Instruction, Processor *);

void execute_instruction(uint32_t instruction_bits, Processor *processor,Byte *memory) {    
    Instruction instruction = parse_instruction(instruction_bits);
    switch(instruction.opcode) {
        case 0x33:
            execute_rtype(instruction, processor);
            break;
        case 0x13:
            execute_itype_except_load(instruction, processor);
            break;
        case 0x73:
            execute_ecall(processor, memory);
            break;
        case 0x63:
            execute_branch(instruction, processor);
            break;
        case 0x6F:
            execute_jal(instruction, processor);
            break;
        case 0x23:
            execute_store(instruction, processor, memory);
            break;
        case 0x03:
            execute_load(instruction, processor, memory);
            break;
        case 0x37:
            execute_lui(instruction, processor);
            break;
        default: // undefined opcode
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
}

void execute_rtype(Instruction instruction, Processor *processor) {
    switch (instruction.rtype.funct3){
        case 0x0:
            switch (instruction.rtype.funct7) {
                case 0x0:
                  // Add
                  processor->R[instruction.rtype.rd] =
                      ((sWord)processor->R[instruction.rtype.rs1]) +
                      ((sWord)processor->R[instruction.rtype.rs2]);

                  break;
                case 0x1:
                  // Mul
                  processor->R[instruction.rtype.rd] =
                      ((sWord)processor->R[instruction.rtype.rs1]) *
                      ((sWord)processor->R[instruction.rtype.rs2]);
                  break;
                case 0x20:
                    // Sub
                    processor->R[instruction.rtype.rd] =
                        ((sWord)processor->R[instruction.rtype.rs1]) -
                        ((sWord)processor->R[instruction.rtype.rs2]);
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;
        case 0x1:
            switch (instruction.rtype.funct7) {
                case 0x0:
                    // SLL
                    processor->R[instruction.rtype.rd] =
                        processor->R[instruction.rtype.rs1] << 
                        (processor->R[instruction.rtype.rs2] & 0x1F);
                    break;
                case 0x1:
                    // MULH
                    {
                        sDouble result = (sDouble)((sWord)processor->R[instruction.rtype.rs1]) * 
                                        (sDouble)((sWord)processor->R[instruction.rtype.rs2]);
                        processor->R[instruction.rtype.rd] = (Word)(result >> 32);
                    }
                    break;
            }
            break;
        case 0x2:
            // SLT
            processor->R[instruction.rtype.rd] = 
                ((sWord)processor->R[instruction.rtype.rs1] < 
                 (sWord)processor->R[instruction.rtype.rs2]) ? 1 : 0;
            break;
        case 0x4:
            switch (instruction.rtype.funct7) {
                case 0x0:
                    // XOR
                    processor->R[instruction.rtype.rd] =
                        processor->R[instruction.rtype.rs1] ^
                        processor->R[instruction.rtype.rs2];
                    break;
                case 0x1:
                    // DIV
                    {
                        sWord dividend = (sWord)processor->R[instruction.rtype.rs1];
                        sWord divisor = (sWord)processor->R[instruction.rtype.rs2];
                        if (divisor == 0) {
                            processor->R[instruction.rtype.rd] = 0xFFFFFFFF;
                        } else {
                            processor->R[instruction.rtype.rd] = (Word)(dividend / divisor);
                        }
                    }
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;
        case 0x5:
            switch (instruction.rtype.funct7) {
                case 0x0:
                // SRL      
                    processor->R[instruction.rtype.rd] =
                        processor->R[instruction.rtype.rs1] >> 
                        (processor->R[instruction.rtype.rs2] & 0x1F);
                    break;
                case 0x20:
                    // SRA
                    {
                        sWord value = (sWord)processor->R[instruction.rtype.rs1];
                        int shift = processor->R[instruction.rtype.rs2] & 0x1F;
                        processor->R[instruction.rtype.rd] = (Word)(value >> shift);
                    }
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                break;
            }
            break;
        case 0x6:
            switch (instruction.rtype.funct7) {
                case 0x0:
                    // OR
                    processor->R[instruction.rtype.rd] =
                        processor->R[instruction.rtype.rs1] |
                        processor->R[instruction.rtype.rs2];
                    break;
                case 0x1:
                    // REM
                    {
                        sWord dividend = (sWord)processor->R[instruction.rtype.rs1];
                        sWord divisor = (sWord)processor->R[instruction.rtype.rs2];
                        if (divisor == 0) {
                            processor->R[instruction.rtype.rd] = (Word)dividend;
                        } else {
                            processor->R[instruction.rtype.rd] = (Word)(dividend % divisor);
                        }
                    }
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;
        case 0x7:
            // AND
            processor->R[instruction.rtype.rd] =
                processor->R[instruction.rtype.rs1] &
                processor->R[instruction.rtype.rs2];
            break;
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
    processor->PC += 4;
}

void execute_itype_except_load(Instruction instruction, Processor *processor) {
    switch (instruction.itype.funct3) {
        case 0x0:
            // ADDI
            {
                int imm = sign_extend_number(instruction.itype.imm, 12);
                processor->R[instruction.itype.rd] = 
                    ((sWord)processor->R[instruction.itype.rs1]) + imm;
            }
            break;
        case 0x1:
            // SLLI
            processor->R[instruction.itype.rd] = 
                processor->R[instruction.itype.rs1] << 
                (instruction.itype.imm & 0x1F);
            break;
        case 0x2:
            // SLTI
            {
                int imm = sign_extend_number(instruction.itype.imm, 12);
                processor->R[instruction.itype.rd] = 
                    ((sWord)processor->R[instruction.itype.rs1] < imm) ? 1 : 0;
            }
            break;
        case 0x4:
            // XORI
            {
                int imm = sign_extend_number(instruction.itype.imm, 12);
                processor->R[instruction.itype.rd] = 
                    processor->R[instruction.itype.rs1] ^ imm;
            }
            break;
        case 0x5:
            // Shift Right (You must handle both logical and arithmetic)
            {
                int shift_amount = instruction.itype.imm & 0x1F;
                int shift_type = instruction.itype.imm >> 10;
                if (shift_type == 0x0) {
                    // SRLI - logical right shift
                    processor->R[instruction.itype.rd] = 
                        processor->R[instruction.itype.rs1] >> shift_amount;
                } else if (shift_type == 0x1) {
                    // SRAI - arithmetic right shift
                    sWord value = (sWord)processor->R[instruction.itype.rs1];
                    processor->R[instruction.itype.rd] = (Word)(value >> shift_amount);
                } else {
                    handle_invalid_instruction(instruction);
                    exit(-1);
                }
            }
            break;
        case 0x6:
            // ORI
            {
                int imm = sign_extend_number(instruction.itype.imm, 12);
                processor->R[instruction.itype.rd] = 
                    processor->R[instruction.itype.rs1] | imm;
            }
            break;
        case 0x7:
            // ANDI
            {
                int imm = sign_extend_number(instruction.itype.imm, 12);
                processor->R[instruction.itype.rd] = 
                    processor->R[instruction.itype.rs1] & imm;
            }
            break;
        default:
            handle_invalid_instruction(instruction);
            break;
    }
    processor->PC += 4;
}

void execute_ecall(Processor *p, Byte *memory) {
    Register i;
    
    // syscall number is given by a0 (x10)
    // argument is given by a1
    switch(p->R[10]) {
        case 1: // print an integer
            printf("%d",p->R[11]);
            break;
        case 4: // print a string
            for(i=p->R[11];i<MEMORY_SPACE && load(memory,i,LENGTH_BYTE);i++) {
                printf("%c",load(memory,i,LENGTH_BYTE));
            }
            break;
        case 10: // exit
            printf("exiting the simulator\n");
            exit(0);
            break;
        case 11: // print a character
            printf("%c",p->R[11]);
            break;
        default: // undefined ecall
            printf("Illegal ecall number %d\n", p->R[10]);
            exit(-1);
            break;
    }
}

void execute_branch(Instruction instruction, Processor *processor) {
    switch (instruction.sbtype.funct3) {
        case 0x0:
            // BEQ
            {
                int offset = get_branch_offset(instruction);
                if (processor->R[instruction.sbtype.rs1] == processor->R[instruction.sbtype.rs2]) {
                    processor->PC += offset;
                } else {
                    processor->PC += 4;
                }
            }
            break;
        case 0x1:
            // BNE
            {
                int offset = get_branch_offset(instruction);
                if (processor->R[instruction.sbtype.rs1] != processor->R[instruction.sbtype.rs2]) {
                    processor->PC += offset;
                } else {
                    processor->PC += 4;
                }
            }
            break;
        case 0x4:
            // BLT
            {
                int offset = get_branch_offset(instruction);
                if ((sWord)processor->R[instruction.sbtype.rs1] < (sWord)processor->R[instruction.sbtype.rs2]) {
                    processor->PC += offset;
                } else {
                    processor->PC += 4;
                }
            }
            break;
        case 0x5:
            // BGE
            {
                int offset = get_branch_offset(instruction);
                if ((sWord)processor->R[instruction.sbtype.rs1] >= (sWord)processor->R[instruction.sbtype.rs2]) {
                    processor->PC += offset;
                } else {
                    processor->PC += 4;
                }
            }
            break;
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
    processor->PC += 4;
}

void execute_load(Instruction instruction, Processor *processor, Byte *memory) {
    switch (instruction.itype.funct3) {
        case 0x0:
            // LB
            {
                int imm = sign_extend_number(instruction.itype.imm, 12);
                Address addr = processor->R[instruction.itype.rs1] + imm;
                processor->R[instruction.itype.rd] = load(memory, addr, LENGTH_BYTE);
            }
            break;
        case 0x1:
            // LH
            {
                int imm = sign_extend_number(instruction.itype.imm, 12);
                Address addr = processor->R[instruction.itype.rs1] + imm;
                processor->R[instruction.itype.rd] = load(memory, addr, LENGTH_HALF_WORD);
            }
            break;
        case 0x2:
            // LW
            {
                int imm = sign_extend_number(instruction.itype.imm, 12);
                Address addr = processor->R[instruction.itype.rs1] + imm;
                processor->R[instruction.itype.rd] = load(memory, addr, LENGTH_WORD);
            }
            break;
        default:
            handle_invalid_instruction(instruction);
            break;
    }
    processor->PC += 4;
}

void execute_store(Instruction instruction, Processor *processor, Byte *memory) {
    switch (instruction.stype.funct3) {
        case 0x0:
            // SB
            {
                int offset = get_store_offset(instruction);
                Address addr = processor->R[instruction.stype.rs1] + offset;
                store(memory, addr, LENGTH_BYTE, processor->R[instruction.stype.rs2]);
            }
            break;
        case 0x1:
            // SH
            {
                int offset = get_store_offset(instruction);
                Address addr = processor->R[instruction.stype.rs1] + offset;
                store(memory, addr, LENGTH_HALF_WORD, processor->R[instruction.stype.rs2]);
            }
            break;
        case 0x2:
            // SW
            {
                int offset = get_store_offset(instruction);
                Address addr = processor->R[instruction.stype.rs1] + offset;
                store(memory, addr, LENGTH_WORD, processor->R[instruction.stype.rs2]);
            }
            break;
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
    processor->PC += 4;
}

void execute_jal(Instruction instruction, Processor *processor) {
    int offset = get_jump_offset(instruction);
    processor->R[instruction.ujtype.rd] = processor->PC + 4;
    processor->PC += offset;
}

void execute_lui(Instruction instruction, Processor *processor) {
    processor->R[instruction.utype.rd] = instruction.utype.imm << 12;
    processor->PC += 4;
}

void store(Byte *memory, Address address, Alignment alignment, Word value) {
    if (address + alignment > MEMORY_SPACE) {
        handle_invalid_write(address);
        return;
    }
    
    switch (alignment) {
        case LENGTH_BYTE:
            memory[address] = (Byte)(value & 0xFF);
            break;
        case LENGTH_HALF_WORD:
            memory[address] = (Byte)(value & 0xFF);
            memory[address + 1] = (Byte)((value >> 8) & 0xFF);
            break;
        case LENGTH_WORD:
            memory[address] = (Byte)(value & 0xFF);
            memory[address + 1] = (Byte)((value >> 8) & 0xFF);
            memory[address + 2] = (Byte)((value >> 16) & 0xFF);
            memory[address + 3] = (Byte)((value >> 24) & 0xFF);
            break;
        default:
            handle_invalid_write(address);
            break;
    }
}

Word load(Byte *memory, Address address, Alignment alignment) {
    if (address + alignment > MEMORY_SPACE) {
        handle_invalid_read(address);
        return 0;
    }
    
    Word value = 0;
    switch (alignment) {
        case LENGTH_BYTE:
            value = (Word)memory[address];
            // Sign extend byte
            if (value & 0x80) {
                value |= 0xFFFFFF00;
            }
            break;
        case LENGTH_HALF_WORD:
            value = (Word)memory[address] | ((Word)memory[address + 1] << 8);
            // Sign extend half word
            if (value & 0x8000) {
                value |= 0xFFFF0000;
            }
            break;
        case LENGTH_WORD:
            value = (Word)memory[address] | 
                    ((Word)memory[address + 1] << 8) |
                    ((Word)memory[address + 2] << 16) |
                    ((Word)memory[address + 3] << 24);
            break;
        default:
            handle_invalid_read(address);
            return 0;
    }
    return value;
}
