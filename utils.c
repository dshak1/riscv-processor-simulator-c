#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

/* Sign extends the given field to a 32-bit integer where field is
 * interpreted an n-bit integer. */
int sign_extend_number(unsigned int field, unsigned int n) {
  // Check if the sign bit (bit n-1) is set
  unsigned int sign_bit = 1U << (n - 1);
  if (field & sign_bit) {
    // Sign bit is set, extend with 1s
    unsigned int mask = ((1U << (32 - n)) - 1) << n;
    return (int)(field | mask);
  } else {
    // Sign bit is not set, extend with 0s
    return (int)field;
  }
}

/* Unpacks the 32-bit machine code instruction given into the correct
 * type within the instruction struct */
Instruction parse_instruction(uint32_t instruction_bits) {
  Instruction instruction;
  instruction.bits = instruction_bits;
  return instruction;
}

/* Return the number of bytes (from the current PC) to the branch label using
 * the given branch instruction */
int get_branch_offset(Instruction instruction) {
  // SB-type immediate format: [12|10:5|4:1|11]
  // imm[12] = instruction.bits[31]
  // imm[10:5] = instruction.bits[30:25]  
  // imm[4:1] = instruction.bits[11:8]
  // imm[11] = instruction.bits[7]
  
  unsigned int imm12 = (instruction.bits >> 31) & 0x1;
  unsigned int imm10_5 = (instruction.bits >> 25) & 0x3F;
  unsigned int imm4_1 = (instruction.bits >> 8) & 0xF;
  unsigned int imm11 = (instruction.bits >> 7) & 0x1;
  
  unsigned int imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
  
  return sign_extend_number(imm, 13);
}

/* Returns the number of bytes (from the current PC) to the jump label using the
 * given jump instruction */
int get_jump_offset(Instruction instruction) {
  // UJ-type immediate format: [20|10:1|11|19:12]
  // imm[20] = instruction.bits[31]
  // imm[10:1] = instruction.bits[30:21]
  // imm[11] = instruction.bits[20]
  // imm[19:12] = instruction.bits[19:12]
  
  unsigned int imm20 = (instruction.bits >> 31) & 0x1;
  unsigned int imm10_1 = (instruction.bits >> 21) & 0x3FF;
  unsigned int imm11 = (instruction.bits >> 20) & 0x1;
  unsigned int imm19_12 = (instruction.bits >> 12) & 0xFF;
  
  unsigned int imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
  
  return sign_extend_number(imm, 21);
}

int get_store_offset(Instruction instruction) {
  // S-type immediate format: [11:5|4:0]
  // imm[11:5] = instruction.bits[31:25]
  // imm[4:0] = instruction.bits[11:7]
  
  unsigned int imm11_5 = (instruction.bits >> 25) & 0x7F;
  unsigned int imm4_0 = (instruction.bits >> 7) & 0x1F;
  
  unsigned int imm = (imm11_5 << 5) | imm4_0;
  
  return sign_extend_number(imm, 12);
}

void handle_invalid_instruction(Instruction instruction) {
  printf("Invalid Instruction: 0x%08x\n", instruction.bits);
}

void handle_invalid_read(Address address) {
  printf("Bad Read. Address: 0x%08x\n", address);
  exit(-1);
}

void handle_invalid_write(Address address) {
  printf("Bad Write. Address: 0x%08x\n", address);
  exit(-1);
}
