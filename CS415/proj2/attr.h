/**********************************************
        CS415  Project 2
        Spring  2021
        Student Version
**********************************************/

#ifndef ATTR_H
#define ATTR_H

typedef union {int num; char *str;} tokentype;

typedef enum type_expression {TYPE_INT=0, TYPE_BOOL, TYPE_ERROR} Type_Expression;

typedef struct {
	Type_Expression type;
	int targetRegister;
	char *inputStr;
} regInfo;

typedef struct {
	int thenLabel;
	int elseLabel;
	int afterIfLabel;
} ifStmtLabels;

typedef struct {
	Type_Expression type;
	int arrayCheck;
} varType;

typedef struct {
	Type_Expression type;
} varSType;

typedef struct {
	char **idStrs;
	int numId;
} idNames;

typedef struct {
	int isDeclared;
	int indOffset;
	int headLabel;
	int afterForLabel;
} forStmtVals;

#endif


  
