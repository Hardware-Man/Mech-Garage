#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct {
	char name[10];
	int assignedPReg;
	int firstUse;
	int lastUse;
	int occs;
	int offset;
} VReg;

typedef struct {
	char opCode[10];
	int type;
	char src1[10];
	VReg *src1Reg;
	char src2[10];
	VReg *src2Reg;
	char tar1[10];
	VReg *tarReg;
	char tar2[10];
} OpLine;

int checkOp(char*);
void getInstr(OpLine**,VReg**,int*,int*,FILE*);
void insertSpill(OpLine**,OpLine,int,int*,int*);
int priorityCmp(const void*,const void*);
int priorityLiveCmp(const void*,const void*);
int priorityCustomCmp(const void* a, const void* b);
VReg* prioritySet(VReg*,int,char);
void setTopInstr(OpLine*,int,char*,char*);
void outInstr(OpLine*,int,char*,char*);

void bottomUp(int,FILE*,char*);
void simpleTop(int,FILE*,char*);
void liveRangeTop(int,FILE*,char*);
void customTop(int,FILE*,char*);

int checkOp(char* op) {
	if(strcmp(op,"loadI") == 0) return 0;
	else if(strcmp(op,"addI") == 0 || strcmp(op,"subI") == 0 || strcmp(op,"rsubI") == 0 || strcmp(op,"multI") == 0 || strcmp(op,"divI") == 0 || strcmp(op,"rdivI") == 0 || strcmp(op,"lshiftI") == 0 || strcmp(op,"rshiftI") == 0 || strcmp(op,"andI") == 0 || strcmp(op,"orI") == 0 || strcmp(op,"xorI") == 0 || strcmp(op,"loadAI") == 0) return 1;
	else if(strcmp(op,"add") == 0 || strcmp(op,"sub") == 0 || strcmp(op,"mult") == 0 || strcmp(op,"div") == 0 || strcmp(op,"lshift") == 0 || strcmp(op,"rshift") == 0 || strcmp(op,"and") == 0 || strcmp(op,"or") == 0 || strcmp(op,"xor") == 0) return 2;
	else if(strcmp(op,"load") == 0 || strcmp(op,"i2i") == 0 || strcmp(op,"c2c") == 0 || strcmp(op,"c2i") == 0 || strcmp(op,"i2c") == 0) return 3;
	else if(strcmp(op,"store") == 0) return 4;
	else if(strcmp(op,"storeAI") == 0) return 5;
	else if(strcmp(op,"output") == 0) return 6;
	else if(strcmp(op,"outputAI") == 0) return 7;
	else return -1;
}

void getInstr(OpLine **opLinesPtr, VReg **vRegsPtr, int *opNumPtr, int *vRegNumPtr, FILE *inFile) {
	char inLine[500] = "";
	OpLine *opLines = malloc(10*sizeof(OpLine));
	for(int i = 0; i < 10; i++) {
		opLines[i] = (OpLine) {"",0,"",0,"",0,"",0,""};
	}
	int opCap = 10;
	int opNum = 0;
	VReg *vRegs = malloc(10*sizeof(VReg));
	for(int i = 0; i < 10; i++) {
		vRegs[i] = (VReg) {"",-1,0,0,0,0};
	}
	int vRegCap = 10;
	int vRegNum = 0;

	char *delims = " ,=>\r\t\n\0";

	while(fgets(inLine, 500, inFile) != NULL) {
		char *token = strtok(inLine, delims);
		if(token == NULL) continue;
		int opType = checkOp(token);

		if(opType == -1) continue;

		if(opNum == opCap) {
			opCap += 10;
			opLines = realloc(opLines, opCap*sizeof(OpLine));
			for(int i = opNum; i < opCap; i++) {
				opLines[i] = (OpLine) {"",0,"",0,"",0,"",0,""};
			}
		}

		strcpy(opLines[opNum].opCode,token);
		opLines[opNum].type = opType;

		token = strtok(NULL, delims);
		strcpy(opLines[opNum].src1, token);

		if(opType == 6) {
			opNum++;
			strcpy(inLine,"");
			continue;
		}
		else if(opType == 7) {
			if(strcmp(token,"r0") != 0) {
				for(int i = 0; i < vRegNum; i++) {
					if(strcmp(vRegs[i].name,token) == 0) {
						opLines[opNum].src1Reg = (VReg*)((long)i+1);
						vRegs[i].lastUse = opNum;
						vRegs[i].occs++;
						break;
					}
				}
			}
			token = strtok(NULL, delims);
			strcpy(opLines[opNum].src2, token);
		}
		else if(opType == 0 || opType == 1 || opType == 2 || opType == 3) {
			if(opType == 1 || opType == 2 || opType == 3) {
				for(int i = 0; i < vRegNum; i++) {
					if(strcmp(vRegs[i].name,token) == 0) {
						opLines[opNum].src1Reg = (VReg*)((long)i+1);
						vRegs[i].lastUse = opNum;
						vRegs[i].occs++;
						break;
					}
				}
			}
			if(opType == 1 || opType == 2) {
				token = strtok(NULL, delims);
				strcpy(opLines[opNum].src2, token);
				if(opType == 2 && strcmp("r0",token) != 0) {
					for(int i = 0; i < vRegNum; i++) {
						if(strcmp(vRegs[i].name,token) == 0) {
							opLines[opNum].src2Reg = (VReg*)((long)i+1);
							vRegs[i].lastUse = opNum;
							if(strcmp(opLines[opNum].src1,token) != 0) vRegs[i].occs++;
							break;
						}
					}
				}
			}

			if(vRegNum == vRegCap) {
				vRegCap += 10;
				vRegs = realloc(vRegs, vRegCap*sizeof(VReg));
				for(int i = vRegNum; i < vRegCap; i++) {
					vRegs[i] = (VReg) {"",-1,0,0,0,0};
				}
			}

			token = strtok(NULL, delims);
			strcpy(opLines[opNum].tar1, token);
			if(strcmp(token,"r0") != 0) {
				strcpy(vRegs[vRegNum].name, token);
				opLines[opNum].tarReg = (VReg*)((long)vRegNum+1);
				vRegs[vRegNum].firstUse = opNum;
				vRegs[vRegNum].lastUse = opNum;
				vRegs[vRegNum].occs++;
				vRegNum++;
			}
		}
		else {
			for(int i = 0; i < vRegNum; i++) {
				if(strcmp(vRegs[i].name,token) == 0) {
					opLines[opNum].src1Reg = (VReg*)((long)i+1);
					vRegs[i].lastUse = opNum;
					vRegs[i].occs++;
					break;
				}
			}

			token = strtok(NULL, delims);
			strcpy(opLines[opNum].tar1, token);
			for(int i = 0; i < vRegNum; i++) {
				if(strcmp(vRegs[i].name,token) == 0) {
					opLines[opNum].tarReg = (VReg*)((long)i+1);
					vRegs[i].lastUse = opNum;
					if(strcmp(opLines[opNum].src1,token) != 0) vRegs[i].occs++;
					break;
				}
			}

			if(opType == 5) {
				token = strtok(NULL, delims);
				strcpy(opLines[opNum].tar2, token);
			}
		}
		opNum++;
		strcpy(inLine,"");
	}

	*opLinesPtr = opLines;
	*vRegsPtr = vRegs;
	*opNumPtr = opNum;
	*vRegNumPtr = vRegNum;
}

void insertSpill(OpLine **opLinesPtr, OpLine spill, int pos, int *opNumPtr, int *opCapPtr) {
	OpLine *opLines = *opLinesPtr;
	int opNum = *opNumPtr;
	int opCap = *opCapPtr;

	if(opNum == opCap) {
		opCap += 10;
		opLines = realloc(opLines, opCap*sizeof(OpLine));
		for(int i = opNum; i < opCap; i++) opLines[i] = (OpLine) {"",0,"",0,"",0,"",0,""};
	}

	for(int i = opNum; i > pos ; i--) {
		opLines[i] = opLines[i-1];
	}
	opLines[pos] = spill;
	opNum++;

	*opLinesPtr = opLines;
	*opNumPtr = opNum;
	*opCapPtr = opCap;
}

int priorityCmp(const void* a, const void* b) {
	int occsA = ((VReg*)a)->occs;
	int occsB = ((VReg*)b)->occs;
	if(occsA > occsB) return -1;
	else if(occsA == occsB) return 0;
	else return 1;
}

int priorityLiveCmp(const void* a, const void* b) {
	int occsA = ((VReg*)a)->occs;
	int occsB = ((VReg*)b)->occs;
	int rangeA = ((VReg*)a)->lastUse - ((VReg*)a)->firstUse;
	int rangeB = ((VReg*)b)->lastUse - ((VReg*)b)->firstUse;
	if(occsA > occsB) return -1;
	else if(occsA == occsB) {
		if(rangeA < rangeB) return -1;
		else if(rangeA == rangeB) return 0;
		else return 1;
	}
	else return 1;
}

int priorityCustomCmp(const void* a, const void* b) {
	int occsA = ((VReg*)a)->occs;
	int occsB = ((VReg*)b)->occs;
	int consecA = ((VReg*)a)->offset;
	int consecB = ((VReg*)b)->offset;
	if(occsA > occsB) return -1;
	else if(occsA == occsB) {
		if(consecA > consecB) return -1;
		else if(consecA == consecB) return 0;
		else return 1;
	}
	else return 1;
}


VReg* prioritySet(VReg* vRegs, int vRegNum, char cmpType) {
	VReg* prioritized = malloc(vRegNum*sizeof(VReg));
	memcpy(prioritized, vRegs, vRegNum*sizeof(VReg));
	for(int i = 0; i < vRegNum; i++) {
		prioritized[i].assignedPReg = i;
	}
	if(cmpType == 's') qsort(prioritized, vRegNum, sizeof(VReg), priorityCmp);
	else if(cmpType == 't') qsort(prioritized, vRegNum, sizeof(VReg), priorityLiveCmp);
	else qsort(prioritized, vRegNum, sizeof(VReg), priorityCustomCmp);
	return prioritized;
}

void setTopInstr(OpLine* opLines, int opNum, char *strat, char *fileName) {
	int opCap = ((opNum % 10) == 0) ? opNum : (opNum + (10-(opNum%10)));
	int regOffset = 0;

	for(int i = 0; i < opNum; i++) {
		if(opLines[i].type == 6) continue;
		char f1Used = 'n';
		if(opLines[i].src1Reg != 0) {
			if(opLines[i].src1Reg->assignedPReg != -1) sprintf(opLines[i].src1, "r%d", opLines[i].src1Reg->assignedPReg+1);
			else {
				OpLine spillLoad = {"loadAI",1,"r0",0,"",0,"r1",0,""};
				strcpy(opLines[i].src1, "r1");
				sprintf(spillLoad.src2, "%d", opLines[i].src1Reg->offset);
				insertSpill(&opLines,spillLoad,i,&opNum,&opCap);
				f1Used = 'y';
				i++;
			}
		}
		if(opLines[i].src2Reg != 0){
			if(opLines[i].src2Reg->assignedPReg != -1) sprintf(opLines[i].src2, "r%d", opLines[i].src2Reg->assignedPReg+1);
			else {
				OpLine spillLoad = {"loadAI",1,"r0",0,"",0,"r1",0,""};
				if(f1Used == 'n') strcpy(opLines[i].src2, "r1");
				else {
					strcpy(spillLoad.tar1, "r2");
					strcpy(opLines[i].src2, "r2");
				}
				sprintf(spillLoad.src2, "%d", opLines[i].src2Reg->offset);
				insertSpill(&opLines,spillLoad,i,&opNum,&opCap);
				i++;
			}
		}
		if(opLines[i].tarReg != 0) {
			if(opLines[i].tarReg->assignedPReg != -1) sprintf(opLines[i].tar1, "r%d", opLines[i].tarReg->assignedPReg+1);
			else if(opLines[i].type == 4 || opLines[i].type == 5) {
				OpLine spillLoad = {"loadAI",1,"r0",0,"",0,"r1",0,""};
				if(f1Used == 'n') strcpy(opLines[i].tar1, "r1");
				else {
					strcpy(spillLoad.tar1, "r2");
					strcpy(opLines[i].tar1, "r2");
				}
				sprintf(spillLoad.src2, "%d", opLines[i].tarReg->offset);
				insertSpill(&opLines,spillLoad,i,&opNum,&opCap);
				i++;
			}
			else {
				regOffset -= 4;
				opLines[i].tarReg->offset = regOffset;
				OpLine spillStore = {"storeAI",5,"r1",0,"",0,"r0",0,""};
				strcpy(opLines[i].tar1, "r1");
				sprintf(spillStore.tar2, "%d", opLines[i].tarReg->offset);
				i++;
				insertSpill(&opLines,spillStore,i,&opNum,&opCap);
			}
		}
	}

	outInstr(opLines, opNum, strat, fileName);

	free(opLines);
}

void outInstr(OpLine* opLines, int opNum, char *strat, char *fileName) {
	char outFileName[strlen(fileName)+strlen(strat)+1];
	strcpy(outFileName,strat);
	strcat(outFileName,fileName);
	FILE *outFile = fopen(outFileName,"w");
	for(int i = 0; i < opNum; i++) {
		int opType = opLines[i].type;
		if(opType == 0 || opType == 3 || opType == 4) {
			fprintf(outFile, "%s %s => %s\n", opLines[i].opCode, opLines[i].src1, opLines[i].tar1);
		}
		else if(opType == 7) {
			fprintf(outFile, "%s %s, %s\n", opLines[i].opCode, opLines[i].src1, opLines[i].src2);
		}
		else if(opType == 6) {
			fprintf(outFile, "%s %s\n", opLines[i].opCode, opLines[i].src1);
		}
		else if(opType == 5) {
			fprintf(outFile, "%s %s => %s, %s\n", opLines[i].opCode, opLines[i].src1, opLines[i].tar1, opLines[i].tar2);
		}
		else {
			fprintf(outFile, "%s %s, %s => %s\n", opLines[i].opCode, opLines[i].src1, opLines[i].src2, opLines[i].tar1);
		}
	}
}

void bottomUp(int regsGiven, FILE *inFile, char *fileName) {
	OpLine *opLines;
	VReg *vRegs;
	int opNum;
	int vRegNum;
	getInstr(&opLines, &vRegs, &opNum, &vRegNum, inFile);
	char strat[7];
	sprintf(strat, "%d_b_", regsGiven);

	if(vRegNum <= regsGiven) {
		outInstr(opLines, opNum, strat, fileName);
		free(opLines);
		free(vRegs);
		return;
	}

	for(int i = 0; i < opNum; i++) {
		if(opLines[i].src1Reg != 0) opLines[i].src1Reg = &vRegs[(long)opLines[i].src1Reg-1];
		if(opLines[i].src2Reg != 0) opLines[i].src2Reg = &vRegs[(long)opLines[i].src2Reg-1];
		if(opLines[i].tarReg != 0) opLines[i].tarReg = &vRegs[(long)opLines[i].tarReg-1];
	}

	char pRegs[regsGiven][10];
	for(int i = 0; i < regsGiven; i++) {
		strcpy(pRegs[i],"");
	}

	int opCap = ((opNum % 10) == 0) ? opNum : (opNum + (10-(opNum%10)));
	int opOffset = 0;
	int regOffset = 0;

	for(int i = 0; i < opNum; i++) {
		if(opLines[i].src1Reg != 0){
			if(opLines[i].src1Reg->assignedPReg == -2) {
				for(int j = 0; j < regsGiven; j++) {
					if(strcmp(pRegs[j],"") == 0) {
						strcpy(pRegs[j],opLines[i].src1);
						sprintf(opLines[i].src1, "r%d", j+1);
						opLines[i].src1Reg->assignedPReg = j;
						OpLine spillLoad = {"loadAI",1,"r0",0,"",0,"s",opLines[i].src1Reg,""};
						sprintf(spillLoad.src2, "%d", opLines[i].src1Reg->offset);
						strcpy(spillLoad.tar1, opLines[i].src1);
						insertSpill(&opLines,spillLoad,i,&opNum,&opCap);
						i++;
						opOffset++;
						break;
					}
				}

				if(opLines[i].src1Reg->assignedPReg == -2) {
					int farUse = -1;
					int farPReg = -1;
					VReg *farVReg = 0;
					for(int j = 0; j < regsGiven; j++) {
						if(strcmp(opLines[i].src2,pRegs[j]) == 0) continue;
						for(int k = i+1; k < opNum; k++) {
							if(strcmp(pRegs[j],opLines[k].src1) == 0 || strcmp(pRegs[j],opLines[k].src2) == 0 || strcmp(pRegs[j],opLines[k].tar1) == 0) {
								if(farUse < k) {
									if(strcmp(pRegs[j],opLines[k].src1) == 0) farVReg = opLines[k].src1Reg;
									else if(strcmp(pRegs[j],opLines[k].src2) == 0) farVReg = opLines[k].src2Reg;
									else farVReg = opLines[k].tarReg;
									farUse = k;
									farPReg = j;
								}
								break;
							}
						}
					}

					opLines[i].src1Reg->assignedPReg = farPReg;
					strcpy(pRegs[farPReg],opLines[i].src1);
					sprintf(opLines[i].src1, "r%d", farPReg+1);
					farVReg->assignedPReg = -2;
					if(farVReg->offset == 0) {
						regOffset -= 4;
						farVReg->offset = regOffset;
						OpLine spillStore = {"storeAI",5,"s",farVReg,"",0,"r0",0,""};
						sprintf(spillStore.tar2, "%d", farVReg->offset);
						strcpy(spillStore.src1, opLines[i].src1);
						insertSpill(&opLines,spillStore,i,&opNum,&opCap);
						i++;
						opOffset++;
					}
					OpLine spillLoad = {"loadAI",1,"r0",0,"",0,"s",farVReg,""};
					sprintf(spillLoad.src2, "%d", opLines[i].src1Reg->offset);
					strcpy(spillLoad.tar1, opLines[i].src1);
					insertSpill(&opLines,spillLoad,i,&opNum,&opCap);
					i++;
					opOffset++;
				}
			}
			else {
				sprintf(opLines[i].src1, "r%d", opLines[i].src1Reg->assignedPReg+1);
			}
			if(opLines[i].src1Reg->lastUse + opOffset == i) {
				strcpy(pRegs[opLines[i].src1Reg->assignedPReg],"");
			}
		}

		if(opLines[i].src2Reg != 0) {
			if(opLines[i].src2Reg->assignedPReg == -2) {
				for(int j = 0; j < regsGiven; j++) {
					if(opLines[i].src1Reg->assignedPReg == j) continue;
					if(strcmp(pRegs[j],"") == 0) {
						strcpy(pRegs[j],opLines[i].src2);
						sprintf(opLines[i].src2, "r%d", j+1);
						opLines[i].src2Reg->assignedPReg = j;
						OpLine spillLoad = {"loadAI",1,"r0",0,"",0,"s",opLines[i].src2Reg,""};
						sprintf(spillLoad.src2, "%d", opLines[i].src2Reg->offset);
						strcpy(spillLoad.tar1, opLines[i].src2);
						insertSpill(&opLines,spillLoad,i,&opNum,&opCap);
						i++;
						opOffset++;
						break;
					}
				}

				if(opLines[i].src2Reg->assignedPReg == -2) {
					int farUse = -1;
					int farPReg = -1;
					VReg *farVReg = opLines[i].src2Reg;
					for(int j = 0; j < regsGiven; j++) {
						if(opLines[i].src1Reg->assignedPReg == j) continue;
						for(int k = i+1; k < opNum; k++) {
							if(strcmp(pRegs[j],opLines[k].src1) == 0 || strcmp(pRegs[j],opLines[k].src2) == 0 || strcmp(pRegs[j],opLines[k].tar1) == 0) {
								if(farUse < k) {
									if(strcmp(pRegs[j],opLines[k].src1) == 0) farVReg = opLines[k].src1Reg;
									else if(strcmp(pRegs[j],opLines[k].src2) == 0) farVReg = opLines[k].src2Reg;
									else farVReg = opLines[k].tarReg;
									farUse = k;
									farPReg = j;
								}
								break;
							}
						}
					}

					opLines[i].src2Reg->assignedPReg = farPReg;
					strcpy(pRegs[farPReg],opLines[i].src2);
					sprintf(opLines[i].src2, "r%d", farPReg+1);
					farVReg->assignedPReg = -2;
					if(farVReg->offset == 0) {
						regOffset -= 4;
						farVReg->offset = regOffset;
						OpLine spillStore = {"storeAI",5,"s",farVReg,"",0,"r0",0,""};
						sprintf(spillStore.tar2, "%d", farVReg->offset);
						strcpy(spillStore.src1, opLines[i].src2);
						insertSpill(&opLines,spillStore,i,&opNum,&opCap);
						i++;
						opOffset++;
					}
					OpLine spillLoad = {"loadAI",1,"r0",0,"",0,"s",farVReg,""};
					sprintf(spillLoad.src2, "%d", opLines[i].src2Reg->offset);
					strcpy(spillLoad.tar1, opLines[i].src2);
					insertSpill(&opLines,spillLoad,i,&opNum,&opCap);
					i++;
					opOffset++;
				}
			}
			else {
				sprintf(opLines[i].src2, "r%d", opLines[i].src2Reg->assignedPReg+1);
			}
			if(opLines[i].src2Reg->lastUse + opOffset == i) {
				strcpy(pRegs[opLines[i].src2Reg->assignedPReg],"");
			}
		}

		if(opLines[i].tarReg != 0) {
			if(opLines[i].type == 4 || opLines[i].type == 5) {
				if(opLines[i].tarReg->assignedPReg == -2) {
					for(int j = 0; j < regsGiven; j++) {
						if(opLines[i].src1Reg->assignedPReg == j) continue;
						if(strcmp(pRegs[j],"") == 0) {
							strcpy(pRegs[j],opLines[i].tar1);
							sprintf(opLines[i].tar1, "r%d", j+1);
							opLines[i].tarReg->assignedPReg = j;
							OpLine spillLoad = {"loadAI",1,"r0",0,"",0,"s",opLines[i].tarReg,""};
							sprintf(spillLoad.src2, "%d", opLines[i].tarReg->offset);
							strcpy(spillLoad.tar1, opLines[i].tar1);
							insertSpill(&opLines,spillLoad,i,&opNum,&opCap);
							i++;
							opOffset++;
							break;
						}
					}

					if(opLines[i].tarReg->assignedPReg == -2) {
						int farUse = -1;
						int farPReg = -1;
						VReg *farVReg = opLines[i].tarReg;
						for(int j = 0; j < regsGiven; j++) {
							if(opLines[i].src1Reg->assignedPReg == j) continue;
							for(int k = i+1; k < opNum; k++) {
								if(strcmp(pRegs[j],opLines[k].src1) == 0 || strcmp(pRegs[j],opLines[k].src2) == 0 || strcmp(pRegs[j],opLines[k].tar1) == 0) {
									if(farUse < k) {
										if(strcmp(pRegs[j],opLines[k].src1) == 0) farVReg = opLines[k].src1Reg;
										else if(strcmp(pRegs[j],opLines[k].src2) == 0) farVReg = opLines[k].src2Reg;
										else farVReg = opLines[k].tarReg;
										farUse = k;
										farPReg = j;
									}
									break;
								}
							}
						}

						opLines[i].tarReg->assignedPReg = farPReg;
						strcpy(pRegs[farPReg],opLines[i].tar1);
						sprintf(opLines[i].tar1, "r%d", farPReg+1);
						farVReg->assignedPReg = -2;
						if(farVReg->offset == 0) {
							regOffset -= 4;
							farVReg->offset = regOffset;
							OpLine spillStore = {"storeAI",5,"s",farVReg,"",0,"r0",0,""};
							sprintf(spillStore.tar2, "%d", farVReg->offset);
							strcpy(spillStore.src1, opLines[i].tar1);
							insertSpill(&opLines,spillStore,i,&opNum,&opCap);
							i++;
							opOffset++;
						}
						OpLine spillLoad = {"loadAI",1,"r0",0,"",0,"s",farVReg,""};
						sprintf(spillLoad.src2, "%d", opLines[i].tarReg->offset);
						strcpy(spillLoad.tar1, opLines[i].tar1);
						insertSpill(&opLines,spillLoad,i,&opNum,&opCap);
						i++;
						opOffset++;
					}
				}
				else {
					sprintf(opLines[i].tar1, "r%d", opLines[i].tarReg->assignedPReg+1);
				}
				if(opLines[i].tarReg->lastUse + opOffset == i) {
					strcpy(pRegs[opLines[i].tarReg->assignedPReg],"");
				}
			}
			else {
				if(opLines[i].tarReg->assignedPReg == -1) {
					for(int j = 0; j < regsGiven; j++) {
						if(strcmp(pRegs[j],"") == 0) {
							strcpy(pRegs[j],opLines[i].tar1);
							sprintf(opLines[i].tar1, "r%d", j+1);
							opLines[i].tarReg->assignedPReg = j;
							break;
						}
					}

					if(opLines[i].tarReg->assignedPReg == -1) {
						int farUse = -1;
						int farPReg = -1;
						VReg *farVReg = opLines[i].tarReg;
						for(int j = 0; j < regsGiven; j++) {
							for(int k = i+1; k < opNum; k++) {
								if(strcmp(pRegs[j],opLines[k].src1) == 0 || strcmp(pRegs[j],opLines[k].src2) == 0 || strcmp(pRegs[j],opLines[k].tar1) == 0) {
									if(farUse < k) {
										if(strcmp(pRegs[j],opLines[k].src1) == 0) farVReg = opLines[k].src1Reg;
										else if(strcmp(pRegs[j],opLines[k].src2) == 0) farVReg = opLines[k].src2Reg;
										else farVReg = opLines[k].tarReg;
										farUse = k;
										farPReg = j;
									}
									break;
								}
							}
						}

						opLines[i].tarReg->assignedPReg = farPReg;
						strcpy(pRegs[farPReg],opLines[i].tar1);
						sprintf(opLines[i].tar1, "r%d", farPReg+1);
						farVReg->assignedPReg = -2;
						if(farVReg->offset == 0) {
							regOffset -= 4;
							farVReg->offset = regOffset;
							OpLine spillStore = {"storeAI",5,"s",farVReg,"",0,"r0",0,""};
							sprintf(spillStore.tar2, "%d", farVReg->offset);
							strcpy(spillStore.src1, opLines[i].tar1);
							insertSpill(&opLines,spillStore,i,&opNum,&opCap);
							i++;
							opOffset++;
						}
					}
				}
			}
		}
	}

	outInstr(opLines, opNum, strat, fileName);
	free(opLines);
	free(vRegs);	
}

void simpleTop(int regsGiven, FILE *inFile, char *fileName) {
	OpLine *opLines;
	VReg *vRegs;
	int opNum;
	int vRegNum;
	getInstr(&opLines, &vRegs, &opNum, &vRegNum, inFile);
	char strat[7];
	sprintf(strat, "%d_s_", regsGiven);

	if(vRegNum <= regsGiven) {
		outInstr(opLines, opNum, strat, fileName);
		free(opLines);
		free(vRegs);
		return;
	}

	VReg* prioritized = prioritySet(vRegs, vRegNum, 's');

	for(int i = 2; i < regsGiven; i++) {
		vRegs[prioritized[i-2].assignedPReg].assignedPReg = i; 
	}

	free(prioritized);

	for(int i = 0; i < opNum; i++) {
		if(opLines[i].src1Reg != 0) opLines[i].src1Reg = &vRegs[(long)opLines[i].src1Reg-1];
		if(opLines[i].src2Reg != 0) opLines[i].src2Reg = &vRegs[(long)opLines[i].src2Reg-1];
		if(opLines[i].tarReg != 0) opLines[i].tarReg = &vRegs[(long)opLines[i].tarReg-1];
	}

	setTopInstr(opLines, opNum, strat, fileName);

	free(vRegs);
}

void liveRangeTop(int regsGiven, FILE *inFile, char *fileName) {
	OpLine *opLines;
	VReg *vRegs;
	int opNum;
	int vRegNum;
	getInstr(&opLines, &vRegs, &opNum, &vRegNum, inFile);
	char strat[7];
	sprintf(strat, "%d_t_", regsGiven);

	if(vRegNum <= regsGiven) {
		outInstr(opLines, opNum, strat, fileName);
		free(opLines);
		free(vRegs);
		return;
	}

	for(int i = 0; i < opNum; i++) {
		if(opLines[i].src1Reg != 0) opLines[i].src1Reg = &vRegs[(long)opLines[i].src1Reg-1];
		if(opLines[i].src2Reg != 0) opLines[i].src2Reg = &vRegs[(long)opLines[i].src2Reg-1];
		if(opLines[i].tarReg != 0) opLines[i].tarReg = &vRegs[(long)opLines[i].tarReg-1];
	}

	VReg* prioritized = prioritySet(vRegs, vRegNum, 't');
	int pRegs[regsGiven-2][vRegNum];
	for(int i = 0; i < regsGiven-2; i++) {
		for(int j = 0; j < vRegNum; j++) {
			pRegs[i][j] = -1;
		}
	}

	for(int i = 0; i < vRegNum; i++) {
		char regSet = 'n';
		for(int j = 0; j < regsGiven-2; j++) {
			for(int k = 0; k < vRegNum; k++) {
				if(pRegs[j][k] == -1) {
					pRegs[j][k] = prioritized[i].assignedPReg;
					vRegs[prioritized[i].assignedPReg].assignedPReg = j+2;
					regSet = 'y';
					break;
				}
				else {
					if(vRegs[prioritized[i].assignedPReg].firstUse < vRegs[pRegs[j][k]].lastUse && vRegs[prioritized[i].assignedPReg].firstUse >= vRegs[pRegs[j][k]].firstUse) break;
					if(vRegs[prioritized[i].assignedPReg].lastUse > vRegs[pRegs[j][k]].firstUse && vRegs[prioritized[i].assignedPReg].lastUse <= vRegs[pRegs[j][k]].lastUse) break;
					if(vRegs[pRegs[j][k]].firstUse < vRegs[prioritized[i].assignedPReg].lastUse && vRegs[pRegs[j][k]].firstUse >= vRegs[prioritized[i].assignedPReg].firstUse) break;
					if(vRegs[pRegs[j][k]].lastUse > vRegs[prioritized[i].assignedPReg].firstUse && vRegs[pRegs[j][k]].lastUse <= vRegs[prioritized[i].assignedPReg].lastUse) break;
				}
			}
			if(regSet == 'y') break;
		}
	}

	free(prioritized);

	setTopInstr(opLines, opNum, strat, fileName);

	free(vRegs);
}

void customTop(int regsGiven, FILE *inFile, char *fileName) {
	OpLine *opLines;
	VReg *vRegs;
	int opNum;
	int vRegNum;
	getInstr(&opLines, &vRegs, &opNum, &vRegNum, inFile);
	char strat[7];
	sprintf(strat, "%d_o_", regsGiven);

	if(vRegNum <= regsGiven) {
		outInstr(opLines, opNum, strat, fileName);
		free(opLines);
		free(vRegs);
		return;
	}

	for(int i = 0; i < opNum; i++) {
		if(opLines[i].src1Reg != 0) opLines[i].src1Reg = &vRegs[(long)opLines[i].src1Reg-1];
		if(opLines[i].src2Reg != 0) opLines[i].src2Reg = &vRegs[(long)opLines[i].src2Reg-1];
		if(opLines[i].tarReg != 0) opLines[i].tarReg = &vRegs[(long)opLines[i].tarReg-1];
	}

	for(int i = 1; i < opNum; i++) {
		if(opLines[i-1].src1Reg != 0) {
			if(opLines[i-1].src1Reg == opLines[i].src1Reg || opLines[i-1].src1Reg == opLines[i].src2Reg || opLines[i-1].src1Reg == opLines[i].tarReg) {
				if(opLines[i-1].src1Reg->assignedPReg == -1) opLines[i-1].src1Reg->assignedPReg = 0;
				opLines[i-1].src1Reg->assignedPReg++;
			}
			else {
				if(opLines[i-1].src1Reg->offset < opLines[i-1].src1Reg->assignedPReg) opLines[i-1].src1Reg->offset = opLines[i-1].src1Reg->assignedPReg;
				opLines[i-1].src1Reg->assignedPReg = -1;
			}
		}
		if(opLines[i-1].src2Reg != 0 && opLines[i-1].src2Reg != opLines[i-1].src1Reg) {
			if(opLines[i-1].src2Reg == opLines[i].src1Reg || opLines[i-1].src2Reg == opLines[i].src2Reg || opLines[i-1].src2Reg == opLines[i].tarReg) {
				if(opLines[i-1].src2Reg->assignedPReg == -1) opLines[i-1].src2Reg->assignedPReg = 0;
				opLines[i-1].src2Reg->assignedPReg++;
			}
			else {
				if(opLines[i-1].src2Reg->offset < opLines[i-1].src2Reg->assignedPReg) opLines[i-1].src2Reg->offset = opLines[i-1].src2Reg->assignedPReg;
				opLines[i-1].src2Reg->assignedPReg = -1;
			}
		}
		if(opLines[i-1].tarReg != 0 && opLines[i-1].tarReg != opLines[i-1].src1Reg) {
			if(opLines[i-1].tarReg == opLines[i].src1Reg || opLines[i-1].tarReg == opLines[i].src2Reg || opLines[i-1].tarReg == opLines[i].tarReg) {
				if(opLines[i-1].tarReg->assignedPReg == -1) opLines[i-1].tarReg->assignedPReg = 0;
				opLines[i-1].tarReg->assignedPReg++;
			}
			else {
				if(opLines[i-1].tarReg->offset < opLines[i-1].tarReg->assignedPReg) opLines[i-1].tarReg->offset = opLines[i-1].tarReg->assignedPReg;
				opLines[i-1].tarReg->assignedPReg = -1;
			}
		}
	}

	if(opLines[opNum-1].src1Reg != 0) opLines[opNum-1].src1Reg->assignedPReg = -1;
	if(opLines[opNum-1].src2Reg != 0) opLines[opNum-1].src2Reg->assignedPReg = -1;
	if(opLines[opNum-1].tarReg != 0) opLines[opNum-1].tarReg->assignedPReg = -1;

	VReg* prioritized = prioritySet(vRegs, vRegNum, 'o');
	int pRegs[regsGiven-2][vRegNum];
	for(int i = 0; i < regsGiven-2; i++) {
		for(int j = 0; j < vRegNum; j++) {
			pRegs[i][j] = -1;
		}
	}

	for(int i = 0; i < vRegNum; i++) {
		char regSet = 'n';
		for(int j = 0; j < regsGiven-2; j++) {
			for(int k = 0; k < vRegNum; k++) {
				if(pRegs[j][k] == -1) {
					pRegs[j][k] = prioritized[i].assignedPReg;
					vRegs[prioritized[i].assignedPReg].assignedPReg = j+2;
					regSet = 'y';
					break;
				}
				else {
					if(vRegs[prioritized[i].assignedPReg].firstUse < vRegs[pRegs[j][k]].lastUse && vRegs[prioritized[i].assignedPReg].firstUse >= vRegs[pRegs[j][k]].firstUse) break;
					if(vRegs[prioritized[i].assignedPReg].lastUse > vRegs[pRegs[j][k]].firstUse && vRegs[prioritized[i].assignedPReg].lastUse <= vRegs[pRegs[j][k]].lastUse) break;
					if(vRegs[pRegs[j][k]].firstUse < vRegs[prioritized[i].assignedPReg].lastUse && vRegs[pRegs[j][k]].firstUse >= vRegs[prioritized[i].assignedPReg].firstUse) break;
					if(vRegs[pRegs[j][k]].lastUse > vRegs[prioritized[i].assignedPReg].firstUse && vRegs[pRegs[j][k]].lastUse <= vRegs[prioritized[i].assignedPReg].lastUse) break;
				}
			}
			if(regSet == 'y') break;
		}
	}

	free(prioritized);

	setTopInstr(opLines, opNum, strat, fileName);

	free(vRegs);
}

int main(int argc, char** argv) {
	if(argc != 4) {
		printf("%d inputs - Incorrect number of inputs!\n",argc);
		return 0;
	}

	int regsGiven = atoi(argv[1]);
	if(regsGiven == 0) {
		printf("Regs Given = %s - Invalid input!\n", argv[1]);
		return 0;
	}

	FILE *inFile = fopen(argv[3],"r");
	if(inFile == NULL) {
		printf("Invalid input\n");
		return 0;
	}

	if(strcmp(argv[2], "b") == 0) {
		bottomUp(regsGiven, inFile, argv[3]);
	}
	else if(strcmp(argv[2], "s") == 0) {
		simpleTop(regsGiven, inFile, argv[3]);
	}
	else if(strcmp(argv[2], "t") == 0) {
		liveRangeTop(regsGiven, inFile, argv[3]);
	}
	else if(strcmp(argv[2], "o") == 0) {
		customTop(regsGiven, inFile, argv[3]);
	}
	else {
		printf("Invalid input!\n");
	}

	return 0;
}