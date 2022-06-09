/*
 *********************************************
 *  415 Compilers                            *
 *  Spring 2021                              *
 *  Students                                 *
 *********************************************
 */

#include <stdint.h>

#ifndef INSTR_H
#define INSTR_H

typedef enum {LOADI, LOAD, LOADAI, LOADAO, STORE, STOREAI, STOREAO, ADD, SUB, MUL, DIV, LSHIFTI, RSHIFTI, OUTPUTAI} OpCode;

typedef struct InstructionInfo Instruction;

struct InstructionInfo {
	OpCode opcode;
	int field1;
	int field2;
	int field3;
	Instruction *prev;	/* previous instruction */
	Instruction *next;	/* next instruction */
	uint8_t critical;
};

#endif