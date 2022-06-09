%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "attr.h"
  #include "instrutil.h"
  int yylex();
  void yyerror(char * s);
  #include "symtab.h"

  FILE *outfile;
  char *CommentBuffer;

%}

%union {tokentype token;
  regInfo targetReg;
  ifStmtLabels ifLabels;
  varType vType;
  varSType vSType;
  idNames iNames;
  forStmtVals forVals;
}

%token PROG PERIOD VAR
%token INT BOOL PRT THEN IF DO FI ENDFOR
%token ARRAY OF
%token BEG END ASG
%token EQ NEQ LT LEQ GT GEQ AND OR TRUE FALSE
%token ELSE
%token FOR
%token <token> ID ICONST

%type <targetReg> exp
%type <targetReg> lhs
%type <ifLabels> ifhead
%type <ifLabels> condexp
%type <vType> type
%type <vSType> stype
%type <iNames> idlist
%type <forVals> ctrlexp

%start program

%nonassoc EQ NEQ LT LEQ GT GEQ 
%left '+' '-' AND
%left '*' OR

%nonassoc THEN
%nonassoc ELSE

%%
program
  : {
      emitComment("Assign STATIC_AREA_ADDRESS to register \"r0\"");
      emit(NOLABEL, LOADI, STATIC_AREA_ADDRESS, 0, EMPTY);
    } 
    PROG ID ';' block PERIOD
;

block : variables cmpdstmt ;

variables
  : /* empty */
  | VAR vardcls
;

vardcls
  : vardcls vardcl ';'
  | vardcl ';'
  | error ';' {yyerror("***Error: illegal variable declaration\n");}  
;

vardcl 
  : idlist ':' type {
    if ($3.arrayCheck != -1) {
      for (int i = 0; i < $1.numId; i++) {
        if (lookup($1.idStrs[i]) != NULL) {
          printf("\n***Error: duplicate declaration of %s\n", $1.idStrs[i]);
        }
        insert($1.idStrs[i], $3.type, NextOffset($3.arrayCheck), 1);
        free($1.idStrs[i]);
      }
    }
    else {
      for (int i = 0; i < $1.numId; i++) {
        if (lookup($1.idStrs[i]) != NULL) {
          printf("\n***Error: duplicate declaration of %s\n", $1.idStrs[i]);
        }
        insert($1.idStrs[i], $3.type, NextOffset(1), 0);
        free($1.idStrs[i]);
      }
    }
    free($1.idStrs);
  }
;

idlist
  : idlist ',' ID {
    $$.idStrs = malloc((1+$1.numId)*sizeof(char*));
    for (int i = 0; i < $1.numId; i++) {
      $$.idStrs[i] = malloc(sizeof($1.idStrs[i]));
      sprintf($$.idStrs[i], "%s", $1.idStrs[i]);
      free($1.idStrs[i]);
    }
    free($1.idStrs);

    $$.idStrs[$1.numId] = malloc(sizeof($3.str));
    sprintf($$.idStrs[$1.numId], "%s", $3.str);
    $$.numId = 1+$1.numId;
  }

  | ID {
    $$.idStrs = malloc(sizeof(char*));
    $$.idStrs[0] = malloc(sizeof($1.str));
    sprintf($$.idStrs[0], "%s", $1.str);
    $$.numId = 1;
  }
;


type
  : ARRAY '[' ICONST ']' OF stype {
    $$.type = $6.type;
    $$.arrayCheck = $3.num;
  }

  | stype {
    $$.type = $1.type;
    $$.arrayCheck = -1;
  }
;

stype
  : INT {$$.type = TYPE_INT;}
  | BOOL {$$.type = TYPE_BOOL;}
;

stmtlist 
  : stmtlist ';' stmt
  | stmt
  | error {yyerror("***Error: ';' expected or illegal statement \n");}
;

stmt
  : ifstmt
  | fstmt
  | astmt
  | writestmt
  | cmpdstmt
;

cmpdstmt : BEG stmtlist END ;

ifstmt
  : ifhead
    THEN {
      emit($1.thenLabel, NOP, EMPTY, EMPTY, EMPTY);
    }
    stmt {
      emit(NOLABEL, BR, $1.afterIfLabel, EMPTY, EMPTY);
    }
    ELSE {
      emit($1.elseLabel, NOP, EMPTY, EMPTY, EMPTY);
    }
    stmt {
      emit(NOLABEL, BR, $1.afterIfLabel, EMPTY, EMPTY);
    }
    FI {
      emit($1.afterIfLabel, NOP, EMPTY, EMPTY, EMPTY);
    }
;

ifhead
  : IF condexp {
    $$.thenLabel = $2.thenLabel;
    $$.elseLabel = $2.elseLabel;
    $$.afterIfLabel = $2.afterIfLabel;
  }
;

writestmt
  : PRT '(' exp ')' {
    int printOffset = -4; /* default location for printing */
    sprintf(CommentBuffer, "Code for \"PRINT\" from offset %d", printOffset);
    emitComment(CommentBuffer);

    if ($3.type == TYPE_ERROR) {
      printf("\n***Error: illegal type for write\n");
    }

    emit(NOLABEL, STOREAI, $3.targetRegister, 0, printOffset);
    emit(NOLABEL, OUTPUTAI, 0, printOffset, EMPTY);
    free($3.inputStr);
  }
;

fstmt	
  : FOR ctrlexp DO stmt {
    if ($2.isDeclared == 1) {
      int newReg1 = NextRegister();
      int newReg2 = NextRegister();
      int newReg3 = NextRegister();
      emit(NOLABEL, LOADAI, 0, $2.indOffset, newReg1);
      emit(NOLABEL, LOADI, 1, newReg2, EMPTY);
      emit(NOLABEL, ADD, newReg1, newReg2, newReg3);
      emit(NOLABEL, STOREAI, newReg3, 0, $2.indOffset);
      emit(NOLABEL, BR, $2.headLabel, EMPTY, EMPTY);
      emit($2.afterForLabel, NOP, EMPTY, EMPTY, EMPTY);
    }
  }
  ENDFOR
;

astmt
  : lhs ASG exp {
    if (! ((($1.type == TYPE_INT) && ($3.type == TYPE_INT)) || (($1.type == TYPE_BOOL) && ($3.type == TYPE_BOOL)))) {
      printf("\n***Error: assignment types do not match\n");
    }

    emit(NOLABEL, STORE, $3.targetRegister, $1.targetRegister, EMPTY);
    free($3.inputStr);
  }
;

lhs
  : ID { /* BOGUS  - needs to be fixed */
    int newReg1 = NextRegister();

    SymTabEntry *symPtr = lookup($1.str);
    Type_Expression symType;

    if (symPtr != NULL) {
      int newReg2 = NextRegister();
      if (symPtr->isArray != 1) symType = symPtr->type;
      else {
        printf("\n***Error: assignment to whole array\n");
        symType = TYPE_ERROR;
      }
      sprintf(CommentBuffer, "Compute address of variable \"%s\" at offset %d in register %d", $1.str, symPtr->offset, newReg2);
      emitComment(CommentBuffer);
      emit(NOLABEL, LOADI, symPtr->offset, newReg1, EMPTY);
      emit(NOLABEL, ADD, 0, newReg1, newReg2);

      $$.targetRegister = newReg2;
    } 
    else {
      printf("\n***Error: undeclared identifier %s\n", $1.str);
      symType = TYPE_ERROR;
      $$.targetRegister = newReg1;
    }
    
    $$.type = symType;
  }

  | ID '[' exp ']' {
    int newReg1 = NextRegister();
    
    SymTabEntry *symPtr = lookup($1.str);
    Type_Expression symType;

    if (symPtr != NULL) {
      symType = symPtr->type;
      int newReg2 = NextRegister();
      int newReg3 = NextRegister();
      int newReg4 = NextRegister();
      int newReg5 = NextRegister();

      if (symPtr->isArray != 1) printf("\n***Error: id %s is not an array\n", $1.str);

      if ($3.type != TYPE_INT) printf("\n***Error: subscript exp not type integer\n");

      sprintf(CommentBuffer, "Compute address of variable \"%s\" at offset %d in register %d", $1.str, symPtr->offset, newReg5);
      emitComment(CommentBuffer);
      emit(NOLABEL, LOADI, 4, newReg1, EMPTY);
      emit(NOLABEL, MULT, $3.targetRegister, newReg1, newReg2);
      emit(NOLABEL, LOADI, symPtr->offset, newReg3, EMPTY);
      emit(NOLABEL, ADD, newReg3, newReg2, newReg4);
      emit(NOLABEL, ADD, 0, newReg4, newReg5);

      $$.targetRegister = newReg5;
    } 
    else {
      printf("\n***Error: undeclared identifier %s\n", $1.str);
      symType = TYPE_ERROR;
      $$.targetRegister = newReg1;
    }

    $$.type = symType;
    free($3.inputStr);
  }
;

exp	
  : exp '+' exp {
    int newReg = NextRegister();
    Type_Expression expType;

    if (! (($1.type == TYPE_INT) && ($3.type == TYPE_INT))) {
      printf("\n***Error: types of operands for operation %s + %s do not match\n", $1.inputStr, $3.inputStr);
      expType = TYPE_ERROR;
    }
    else expType = $1.type;

    $$.type = expType;
    
    $$.type = $1.type;
    $$.targetRegister = newReg;
    int length = snprintf(NULL, 0, "%s + %s", $1.inputStr, $3.inputStr);
    $$.inputStr = malloc(length+1);
    sprintf($$.inputStr, "%s + %s", $1.inputStr, $3.inputStr);
    emit(NOLABEL, ADD, $1.targetRegister, $3.targetRegister, newReg);
    free($1.inputStr); free($3.inputStr);
  } 

  | exp '-' exp {
    int newReg = NextRegister();
    Type_Expression expType;

    if (! (($1.type == TYPE_INT) && ($3.type == TYPE_INT))) {
      printf("\n***Error: types of operands for operation %s - %s do not match\n", $1.inputStr, $3.inputStr);
      expType = TYPE_ERROR;
    }
    else expType = $1.type;

    $$.type = expType;
    
    $$.type = $1.type;
    $$.targetRegister = newReg;
    int length = snprintf(NULL, 0, "%s - %s", $1.inputStr, $3.inputStr);
    $$.inputStr = malloc(length+1);
    sprintf($$.inputStr, "%s - %s", $1.inputStr, $3.inputStr);
    emit(NOLABEL, SUB, $1.targetRegister, $3.targetRegister, newReg);
    free($1.inputStr); free($3.inputStr);
  }

  | exp '*' exp {
    int newReg = NextRegister();
    Type_Expression expType;

    if (! (($1.type == TYPE_INT) && ($3.type == TYPE_INT))) {
      printf("\n***Error: types of operands for operation %s * %s do not match\n", $1.inputStr, $3.inputStr);
      expType = TYPE_ERROR;
    }
    else expType = $1.type;

    $$.type = expType;

    $$.type = $1.type;
    $$.targetRegister = newReg;
    int length = snprintf(NULL, 0, "%s * %s", $1.inputStr, $3.inputStr);
    $$.inputStr = malloc(length+1);
    sprintf($$.inputStr, "%s * %s", $1.inputStr, $3.inputStr);
    emit(NOLABEL, MULT, $1.targetRegister, $3.targetRegister, newReg);
    free($1.inputStr); free($3.inputStr);
  }
  
  | exp AND exp {
    int newReg = NextRegister();
    Type_Expression expType;

    if (! (($1.type == TYPE_BOOL) && ($3.type == TYPE_BOOL))) {
      printf("\n***Error: types of operands for operation %s AND %s do not match\n", $1.inputStr, $3.inputStr);
      expType = TYPE_ERROR;
    }
    else expType = $1.type;

    $$.type = expType;

    $$.type = $1.type;
    $$.targetRegister = newReg;
    int length = snprintf(NULL, 0, "%s AND %s", $1.inputStr, $3.inputStr);
    $$.inputStr = malloc(length+1);
    sprintf($$.inputStr, "%s AND %s", $1.inputStr, $3.inputStr);
    emit(NOLABEL, AND_INSTR, $1.targetRegister, $3.targetRegister, newReg);
    free($1.inputStr); free($3.inputStr);
  }
  
  | exp OR exp {
    int newReg = NextRegister();
    Type_Expression expType;

    if (! (($1.type == TYPE_BOOL) && ($3.type == TYPE_BOOL))) {
      printf("\n***Error: types of operands for operation %s OR %s do not match\n", $1.inputStr, $3.inputStr);
      expType = TYPE_ERROR;
    }
    else expType = $1.type;

    $$.type = expType;
    $$.targetRegister = newReg;
    int length = snprintf(NULL, 0, "%s OR %s", $1.inputStr, $3.inputStr);
    $$.inputStr = malloc(length+1);
    sprintf($$.inputStr, "%s OR %s", $1.inputStr, $3.inputStr);
    emit(NOLABEL, OR_INSTR, $1.targetRegister, $3.targetRegister, newReg);
    free($1.inputStr); free($3.inputStr);
  }
  
  | ID { /* BOGUS  - needs to be fixed */
    int newReg = NextRegister();
    SymTabEntry *symPtr = lookup($1.str);
    Type_Expression symType;

    if (symPtr != NULL) {
      if (symPtr->isArray != 1) symType = symPtr->type;
      else {
        printf("\n***Error: id %s is not a scalar variable\n", $1.str);
        symType = TYPE_ERROR;
      }
      sprintf(CommentBuffer, "Load RHS value of variable \"%s\" at offset %d in register %d", $1.str, symPtr->offset, newReg);
      emitComment(CommentBuffer);
      emit(NOLABEL, LOADAI, 0, symPtr->offset, newReg);
    } 
    else {
      printf("\n***Error: undeclared identifier %s\n", $1.str);
      symType = TYPE_ERROR;
    }

    $$.targetRegister = newReg;
    $$.type = symType;
    $$.inputStr = malloc(sizeof($1.str));
    sprintf($$.inputStr, "%s", $1.str);
  }

  | ID '[' exp ']' {
    int newReg1 = NextRegister();
    SymTabEntry *symPtr = lookup($1.str);
    Type_Expression symType;

    if (symPtr != NULL) {
      symType = symPtr->type;
      int newReg2 = NextRegister();
      int newReg3 = NextRegister();
      int newReg4 = NextRegister();
      int newReg5 = NextRegister();

      if (symPtr->isArray != 1) printf("\n***Error: id %s is not an array\n", $1.str);

      if ($3.type != TYPE_INT) printf("\n***Error: subscript exp not type integer\n");
        
      sprintf(CommentBuffer, "Load RHS value of variable \"%s\" at offset %d in register %d", $1.str, symPtr->offset, newReg5);
      emitComment(CommentBuffer);
      emit(NOLABEL, LOADI, 4, newReg1, EMPTY);
      emit(NOLABEL, MULT, $3.targetRegister, newReg1, newReg2);
      emit(NOLABEL, LOADI, symPtr->offset, newReg3, EMPTY);
      emit(NOLABEL, ADD, newReg3, newReg2, newReg4);
      emit(NOLABEL, LOADAO, 0, newReg4, newReg5);

      $$.targetRegister = newReg5;
    } 
    else {
      printf("\n***Error: undeclared identifier %s\n", $1.str);
      symType = TYPE_ERROR;
      $$.targetRegister = newReg1;
    }
    
    $$.type = symType;
    int length = snprintf(NULL, 0, "%s[%s]", $1.str, $3.inputStr);
    $$.inputStr = malloc(length+1);
    sprintf($$.inputStr, "%s[%s]", $1.str, $3.inputStr);
    free($3.inputStr);
  }

  | ICONST {
    int newReg = NextRegister();
    $$.targetRegister = newReg;
    $$.type = TYPE_INT;
    int length = snprintf(NULL, 0, "%d", $1.num);
    $$.inputStr = malloc(length+1);
    sprintf($$.inputStr, "%d", $1.num);
    emit(NOLABEL, LOADI, $1.num, newReg, EMPTY);
  }

  | TRUE {
    int newReg = NextRegister(); /* TRUE is encoded as value '1' */
    $$.targetRegister = newReg;
    $$.type = TYPE_BOOL;
    $$.inputStr = malloc(5*sizeof(char));
    sprintf($$.inputStr, "true");
    emit(NOLABEL, LOADI, 1, newReg, EMPTY);
  }

  | FALSE {
    int newReg = NextRegister(); /* TRUE is encoded as value '0' */
    $$.targetRegister = newReg;
    $$.type = TYPE_BOOL;
    $$.inputStr = malloc(6*sizeof(char));
    sprintf($$.inputStr, "false");
    emit(NOLABEL, LOADI, 0, newReg, EMPTY);
  }

  | error {yyerror("***Error: illegal expression\n");}  
;

ctrlexp
  : ID ASG ICONST ',' ICONST {
    SymTabEntry *symPtr = lookup($1.str);

    if (symPtr != NULL) {
      $$.isDeclared = 1;
      
      if (symPtr->type != TYPE_INT || symPtr->isArray == 1) printf("\n***Error: induction variable not scalar integer variable\n");

      if ($3.num > $5.num) printf("\n***Error: lower bound exceeds upper bound\n");

      int newReg1 = NextRegister();
      int newReg2 = NextRegister();
      int newReg3 = NextRegister();
      int newReg4 = NextRegister();
      int newReg5 = NextRegister();
      int newReg6 = NextRegister();
      int newLabel1 = NextLabel();
      int newLabel2 = NextLabel();
      int newLabel3 = NextLabel();
      
      sprintf(CommentBuffer, "Initialize induction variable \"%s\" at offset %d with lower bound value %d", $1.str, symPtr->offset, $3.num);
      emitComment(CommentBuffer);
      emit(NOLABEL, LOADI, symPtr->offset, newReg1, EMPTY);
      emit(NOLABEL, ADD, 0, newReg1, newReg2);
      emit(NOLABEL, LOADI, $3.num, newReg3, EMPTY);
      emit(NOLABEL, LOADI, $5.num, newReg4, EMPTY);
      emit(NOLABEL, STORE, newReg3, newReg2, EMPTY);
      sprintf(CommentBuffer, "Generate control code for \"FOR\"");
      emitComment(CommentBuffer);
      emit(newLabel1, LOADAI, 0, symPtr->offset, newReg5);
      emit(NOLABEL, CMPLE, newReg5, newReg4, newReg6);
      emit(NOLABEL, CBR, newReg6, newLabel2, newLabel3);
      emit(newLabel2, NOP, EMPTY, EMPTY, EMPTY);

      $$.indOffset = symPtr->offset; 
      $$.headLabel = newLabel1;
      $$.afterForLabel = newLabel3;
    }
    else {
      $$.isDeclared = 0;
      printf("\n***Error: undeclared identifier %s\n", $1.str);
      printf("\n***Error: induction variable not scalar integer variable\n");
    }

    
  }
;

condexp	
  : exp NEQ exp	{
    int newReg = NextRegister();
    int newLabel1 = NextLabel();
    int newLabel2 = NextLabel();
    int newLabel3 = NextLabel();

    if (! ((($1.type == TYPE_INT) && ($3.type == TYPE_INT)) || (($1.type == TYPE_BOOL) && ($3.type == TYPE_BOOL)))) {
      printf("\n***Error: exp in if stmt must be boolean\n");
    }

    $$.thenLabel = newLabel1;
    $$.elseLabel = newLabel2;
    $$.afterIfLabel = newLabel3;
    sprintf(CommentBuffer, "\"if %s != %s\"", $1.inputStr, $3.inputStr);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "\"then\" at branch L%d, \"else\" at branch L%d", newLabel1, newLabel2);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "statements afterward at branch L%d", newLabel3);
    emitComment(CommentBuffer);
    emit(NOLABEL, CMPNE, $1.targetRegister, $3.targetRegister, newReg);
    emit(NOLABEL, CBR, newReg, newLabel1, newLabel2);
    free($1.inputStr); free($3.inputStr);
  } 

  | exp EQ exp {
    int newReg = NextRegister();
    int newLabel1 = NextLabel();
    int newLabel2 = NextLabel();
    int newLabel3 = NextLabel();

    if (! ((($1.type == TYPE_INT) && ($3.type == TYPE_INT)) || (($1.type == TYPE_BOOL) && ($3.type == TYPE_BOOL)))) {
      printf("\n***Error: exp in if stmt must be boolean\n");
    }

    $$.thenLabel = newLabel1;
    $$.elseLabel = newLabel2;
    $$.afterIfLabel = newLabel3;
    sprintf(CommentBuffer, "\"if %s == %s\"", $1.inputStr, $3.inputStr);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "\"then\" at branch L%d, \"else\" at branch L%d", newLabel1, newLabel2);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "statements afterward at branch L%d", newLabel3);
    emitComment(CommentBuffer);
    emit(NOLABEL, CMPEQ, $1.targetRegister, $3.targetRegister, newReg);
    emit(NOLABEL, CBR, newReg, newLabel1, newLabel2);
    free($1.inputStr); free($3.inputStr);
  } 

  | exp LT exp {
    int newReg = NextRegister();
    int newLabel1 = NextLabel();
    int newLabel2 = NextLabel();
    int newLabel3 = NextLabel();

    if (! (($1.type == TYPE_INT) && ($3.type == TYPE_INT))) {
      printf("\n***Error: exp in if stmt must be boolean\n");
    }

    $$.thenLabel = newLabel1;
    $$.elseLabel = newLabel2;
    $$.afterIfLabel = newLabel3;
    sprintf(CommentBuffer, "\"if %s < %s\"", $1.inputStr, $3.inputStr);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "\"then\" at branch L%d, \"else\" at branch L%d", newLabel1, newLabel2);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "statements afterward at branch L%d", newLabel3);
    emitComment(CommentBuffer);
    emit(NOLABEL, CMPLT, $1.targetRegister, $3.targetRegister, newReg);
    emit(NOLABEL, CBR, newReg, newLabel1, newLabel2);
    free($1.inputStr); free($3.inputStr);
  }

  | exp LEQ exp {
    int newReg = NextRegister();
    int newLabel1 = NextLabel();
    int newLabel2 = NextLabel();
    int newLabel3 = NextLabel();

    if (! (($1.type == TYPE_INT) && ($3.type == TYPE_INT))) {
      printf("\n***Error: exp in if stmt must be boolean\n");
    }

    $$.thenLabel = newLabel1;
    $$.elseLabel = newLabel2;
    $$.afterIfLabel = newLabel3;
    sprintf(CommentBuffer, "\"if %s <= %s\"", $1.inputStr, $3.inputStr);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "\"then\" at branch L%d, \"else\" at branch L%d", newLabel1, newLabel2);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "statements afterward at branch L%d", newLabel3);
    emitComment(CommentBuffer);
    emit(NOLABEL, CMPLE, $1.targetRegister, $3.targetRegister, newReg);
    emit(NOLABEL, CBR, newReg, newLabel1, newLabel2);
    free($1.inputStr); free($3.inputStr);
  }

  | exp GT exp {
    int newReg = NextRegister();
    int newLabel1 = NextLabel();
    int newLabel2 = NextLabel();
    int newLabel3 = NextLabel();

    if (! (($1.type == TYPE_INT) && ($3.type == TYPE_INT))) {
      printf("\n***Error: exp in if stmt must be boolean\n");
    }

    $$.thenLabel = newLabel1;
    $$.elseLabel = newLabel2;
    $$.afterIfLabel = newLabel3;
    sprintf(CommentBuffer, "\"if %s > %s\"", $1.inputStr, $3.inputStr);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "\"then\" at branch L%d, \"else\" at branch L%d", newLabel1, newLabel2);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "statements afterward at branch L%d", newLabel3);
    emitComment(CommentBuffer);
    emit(NOLABEL, CMPGT, $1.targetRegister, $3.targetRegister, newReg);
    emit(NOLABEL, CBR, newReg, newLabel1, newLabel2);
    free($1.inputStr); free($3.inputStr);
  }

  | exp GEQ exp {
    int newReg = NextRegister();
    int newLabel1 = NextLabel();
    int newLabel2 = NextLabel();
    int newLabel3 = NextLabel();

    if (! (($1.type == TYPE_INT) && ($3.type == TYPE_INT))) {
      printf("\n***Error: exp in if stmt must be boolean\n");
    }

    $$.thenLabel = newLabel1;
    $$.elseLabel = newLabel2;
    $$.afterIfLabel = newLabel3;
    sprintf(CommentBuffer, "\"if %s >= %s\"", $1.inputStr, $3.inputStr);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "\"then\" at branch L%d, \"else\" at branch L%d", newLabel1, newLabel2);
    emitComment(CommentBuffer);
    sprintf(CommentBuffer, "statements afterward at branch L%d", newLabel3);
    emitComment(CommentBuffer);
    emit(NOLABEL, CMPGE, $1.targetRegister, $3.targetRegister, newReg);
    emit(NOLABEL, CBR, newReg, newLabel1, newLabel2);
    free($1.inputStr); free($3.inputStr);
  }

  | error {yyerror("***Error: illegal conditional expression\n");}  
;
%%

void yyerror(char* s) {
  fprintf(stderr,"%s\n",s);
}


int main(int argc, char* argv[]) {

  printf("\n     CS415 Spring 2021 Compiler\n\n");

  outfile = fopen("iloc.out", "w");
  if (outfile == NULL) { 
    printf("ERROR: Cannot open output file \"iloc.out\".\n");
    return -1;
  }

  CommentBuffer = (char *) malloc(1832);  
  InitSymbolTable();

  printf("1\t");
  yyparse();
  printf("\n");

  PrintSymbolTable();

  fclose(outfile);

  return 1;
}




