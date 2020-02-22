#include<stdio.h>
#include<stdlib.h>

typedef struct node{
  int x;
  int y;
  int solved;
  struct node* next;
}Blanknode;

char** allocate_puzzle();
Blanknode* insert(Blanknode*, int, int);
char** fill_open(char**, int, int);
int open_count(char**, int, int);
int equiv(char);
char equivc(int);
int check_blank(char**);
int check_valid(char**);
void print_solved(char**);
void free_missing(Blanknode*);
void free_puzzle(char**);

char** allocate_puzzle(){
  char** temp = malloc(16 * sizeof(char*));
  for(int i = 0; i < 16; i++){
    temp[i] = malloc(16 * sizeof(char));
  }
  return temp;
}

Blanknode* insert(Blanknode* head, int m, int n){
  Blanknode* temp = malloc(sizeof(Blanknode));
  temp->x = m;
  temp->y = n;
  temp->solved = 0;
  temp->next = head;
  head = temp;
  return head;
}

char** fill_open(char** puz, int m, int n){
  int row[16] = {0};
  int col[16] = {0};
  int sub[16] = {0};
  for(int i = 0; i < 16; i++){
    if(equiv(puz[m][i]) != -1) row[equiv(puz[m][i])] = 1;
    if(equiv(puz[i][n]) != -1) sub[equiv(puz[i][n])] = 1;
  }
  for(int i = (m - (m%4)); i < (m - (m%4) + 4); i++){
    for(int j = (n - (n%4)); j < (n - (n%4) + 4); j++){
      if(puz[i][j] != '-') sub[equiv(puz[i][j])] = 1;
    }
  }
  for(int i = 0; i < 16; i++){
    if(row[i] == 0){
      if(row[i] == col[i] && row[i] == sub[i]){
	puz[m][n] = equivc(i);
	break;
      }
    }
  }
  return puz;
}

int open_count(char** puz, int m, int n){
  int row[16] = {0};
  int col[16] = {0};
  int sub[16] = {0};
  for(int i = 0; i < 16; i++){
    if(equiv(puz[m][i]) != -1) row[equiv(puz[m][i])] = 1;
    if(equiv(puz[i][n]) != -1) sub[equiv(puz[i][n])] = 1;
  }
  for(int i = (m - (m%4)); i < (m - (m%4) + 4); i++){
    for(int j = (n - (n%4)); j < (n - (n%4) + 4); j++){
      if(puz[i][j] != '-') sub[equiv(puz[i][j])] = 1;
    }
  }
  int result = 0;
  for(int i = 0; i < 16; i++){
    if(row[i] == 0){
      if(row[i] == col[i] && row[i] == sub[i]){
        result++;
      }
    }
  }
  return result;
}

int equiv(char piece){
  switch(piece){
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case 'A': return 10;
  case 'B': return 11;
  case 'C': return 12;
  case 'D': return 13;
  case 'E': return 14;
  case 'F': return 15;
  }
  return -1;
}

char equivc(int piece){
  switch(piece){
  case 0: return '0';
  case 1: return '1';
  case 2: return '2';
  case 3: return '3';
  case 4: return '4';
  case 5: return '5';
  case 6: return '6';
  case 7: return '7';
  case 8: return '8';
  case 9: return '9';
  case 10: return 'A';
  case 11: return 'B';
  case 12: return 'C';
  case 13: return 'D';
  case 14: return 'E';
  case 15: return 'F';
  }
  return '-';
}

int check_blank(char** puz){
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      if(puz[i][j] == '-') return 0;
    }
  }
  return 1;
}

int check_valid(char** puz){
  int* row = malloc(16 * sizeof(int));
  int* col = malloc(16 * sizeof(int));
  int* sub = malloc(16 * sizeof(int));
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      if(row[equiv(puz[i][j])] != 1 && col[equiv(puz[j][i])] != 1){
	row[equiv(puz[i][j])] = 1;
	col[equiv(puz[j][i])] = 1;
      }
      else{
	free(row);
	free(col);
	free(sub);
	return 0;
      }
    }
    row = malloc(16 * sizeof(int));
    col = malloc(16 * sizeof(int));
  }
  free(row);
  free(col);
  for(int i = 0; i < 4; i++){
    int m = i*4;
    for(int j = m; j < m+4; j++){
      for(int k = 0; k < 4; k++){
	if(sub[equiv(puz[j][k])] != 1) sub[equiv(puz[j][k])] = 1;
	else{
	  free(sub);
	  return 0;
	}
      }
    }
    sub = malloc(16 * sizeof(int));
    for(int j = m; j < m+4; j++){
      for(int k = 4; k < 8; k++){
	if(sub[equiv(puz[j][k])] != 1) sub[equiv(puz[j][k])] = 1;
	else{
	  free(sub);
	  return 0;
	}
      }
    }
    sub = malloc(16 * sizeof(int));
    for(int j = m; j < m+4; j++){
      for(int k = 8; k < 12; k++){
	if(sub[equiv(puz[j][k])] != 1) sub[equiv(puz[j][k])] = 1;
	else{
	  free(sub);
	  return 0;
	}
      }
    }
    sub = malloc(16 * sizeof(int));
    for(int j = m; j < m+4; j++){
      for(int k = 12; k < 16; k++){
	if(sub[equiv(puz[j][k])] != 1) sub[equiv(puz[j][k])] = 1;
	else{
	  free(sub);
	  return 0;
	}
      }
    }
    sub = malloc(16 * sizeof(int));
  }
  free(sub);
  return 1;
}

void print_solved(char** puz){
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      if(j < 15) printf("%c\t", puz[i][j]);
      else printf("%c\n", puz[i][j]);
    }
  }
}

void free_missing(Blanknode* a){
  while(a != NULL){
    Blanknode* temp = a;
    a = a->next;
    free(temp);
  }
}

void free_puzzle(char** puz){
  for(int i = 0; i < 16; i++){
    free(puz[i]);
  }
  free(puz);
}

int main(int argc, char** argv){
  FILE* fp = fopen(argv[1], "r");
  char** puzzle;
  puzzle = allocate_puzzle();
  Blanknode* missing = NULL;
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      if(j != 15){
	fscanf(fp, "%c\t", &puzzle[i][j]);
	if(puzzle[i][j] == '-') missing = insert(missing, i, j);
      }
      else{
	fscanf(fp, "%c\n", &puzzle[i][j]);
	if(puzzle[i][j] == '-') missing = insert(missing, i, j);
      }
    }
  }
  while(check_blank(puzzle) == 0){
    Blanknode* ptr = missing;
    int sol_count = 0;
    while(ptr != NULL){
      if(ptr->solved == 1){
	ptr = ptr->next;
	continue;
      }
      int check = open_count(puzzle, ptr->x, ptr->y);
      if(check == 0){
	printf("no-solution\n");
	free_missing(missing);
	free_puzzle(puzzle);
	fclose(fp);
	exit(0);
      }
      if(check == 1){
	puzzle = fill_open(puzzle, ptr->x, ptr->y);
	ptr->solved = 1;
	sol_count++;
      }
      ptr = ptr->next;
    }
    if(sol_count == 0){
      printf("no-solution\n");
      free_missing(missing);
      free_puzzle(puzzle);
      fclose(fp);
      exit(0);
    }
  }
  if(check_valid(puzzle) == 0){
    printf("no-solution\n");
    free_missing(missing);
    free_puzzle(puzzle);
    fclose(fp);
    exit(0);
  }
  print_solved(puzzle);
  free_missing(missing);
  free_puzzle(puzzle);
  fclose(fp);
  return(0);
}
