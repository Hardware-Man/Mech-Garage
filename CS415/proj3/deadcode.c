/*
 *********************************************
 *  415 Compilers                            *
 *  Spring 2021                              *
 *  Students                                 *
 *********************************************
 */


#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "Instr.h"
#include "InstrUtils.h"

void find_critical_addr(int, Instruction*);
void find_critical_reg(int, Instruction*);

void find_critical_addr(int addr, Instruction *origin) {
	Instruction *ptr = origin->prev;
	while (ptr != NULL) {
		if (ptr->critical != 1) {
			switch (ptr->opcode) {
				case STOREAI:
					if (ptr->field3 == addr) {
						ptr->critical = 1;
						find_critical_reg(ptr->field1, ptr);
					}
				default:;
			}
			// If value necessary for original instruction is found, no need to search further
			if (ptr->critical == 1) break;
		} else {
			// If value necessary for original instruction has already been searched through, no need to search further
			if ((ptr->opcode == STOREAI && ptr->field3 == addr) ||
				((ptr->opcode == LOADAI || ptr->opcode == OUTPUTAI) && ptr->field2 == addr)) break;
		}
		ptr = ptr->prev;
	}
}

void find_critical_reg(int reg, Instruction *origin) {
	Instruction *ptr = origin->prev;
	while (ptr != NULL) {
		if (ptr->critical != 1) {
			switch (ptr->opcode) {
				case LOADI:
					if (ptr->field2 == reg) {
						ptr->critical = 1;
					}
					break;
				case LOADAI:
					if (ptr->field3 == reg) {
						ptr->critical = 1;
						find_critical_addr(ptr->field2, ptr);
					}
					break;
				case ADD:
				case SUB:
				case MUL:
					if (ptr->field3 == reg) {
						ptr->critical = 1;
						find_critical_reg(ptr->field1, ptr);
						find_critical_reg(ptr->field2, ptr);
					}
				default:;
			}
			// If value necessary for original instruction is found, no need to search further
			if (ptr->critical == 1) break;
		} else {
			// If value necessary for original instruction has already been searched through, no need to search further
			if ((ptr->opcode == LOADI && ptr->field2 == reg) || (ptr->opcode == LOADAI && ptr->field3 == reg) ||
				(ptr->opcode == STOREAI && ptr->field1 == reg) ||
				((ptr->opcode == ADD || ptr->opcode == SUB || ptr->opcode == MUL) && (ptr->field1 == reg || ptr->field2 == reg || ptr->field3 == reg))) break;
		}
		ptr = ptr->prev;
	}
}

int main(int argc, char *argv[])
{
	Instruction *InstrList = NULL;
	
	if (argc != 1) {
  	    fprintf(stderr, "Use of command:\n  deadcode  < ILOC file\n");
		exit(-1);
	}

	fprintf(stderr,"------------------------------------------------\n");
	fprintf(stderr,"        Local Deadcode Elimination\n               415 Compilers\n                Spring 2021\n");
	fprintf(stderr,"------------------------------------------------\n");

        InstrList = ReadInstructionList(stdin);
 
        Instruction *ptr = InstrList;
        // Search for instruction which loads static area address
        while (ptr != NULL) {
        	if (ptr->opcode == LOADI) {
        		if (ptr->field2 == 0) {
        			ptr->critical = 1;
        			break;
        		}
        	}
        	ptr = ptr->next;
        }

        // Reset ptr to head
        ptr = InstrList;
        // Search for OUTPUTAI addresses
        while (ptr != NULL) {
        	switch (ptr->opcode) {
        		case OUTPUTAI:
        			ptr->critical = 1;
        			// Search for critical instructions that occur before current instruction
        			find_critical_addr(ptr->field2, ptr);
        		default:
        			ptr = ptr->next;
        	}
        }

        PrintInstructionList(stdout, InstrList);

	fprintf(stderr,"\n-----------------DONE---------------------------\n");
	
	return 0;
}
