#include<stdio.h>
#include<stdlib.h>

int count;

char* setword(char*);
void piglatinconvert(char*, int);

char* setword(char* word){
  char* temp = NULL;
  for(int i = 0; word[i] != '\0'; i++){
    temp = realloc(temp, (i+1)*sizeof(char));
    temp[i] = word[i];
    count++;
  }
  return temp;
}

void piglatinconvert(char* word, int size){
  for(int i = 0; i < size; i++){
    char curr = word[i];
    //Vowel check
    if(curr == 'a' || curr == 'e' || curr == 'i' || curr == 'o' || curr == 'u' ||
       curr == 'A' || curr == 'E' || curr == 'I' || curr == 'O' || curr == 'U'){
      while(i < size){
	curr = word[i];
	printf("%c", curr);
	i++;
      }
      printf("yay");
    }
    else{
      //Char array to keep track of consonants
      char* conson = NULL;
      int j = 1;
      //Continuously reallocating array based on the number of beginning consonants
      while(i < size){
	char temp = word[i];
	if(!(temp == 'a' || temp == 'e' || temp == 'i' || temp == 'o' || temp == 'u' || 
	     temp == 'A' || temp == 'E' || temp == 'I' || temp == 'O' || temp == 'U')){
	  curr = word[i];
	  conson = realloc(conson, (j)*sizeof(char));
	  conson[j-1] = curr;
	  i++;
	  j++;
	}
	else break;
      }
      while(i < size){
	curr = word[i];
	printf("%c", curr);
	i++;
      }
      printf("%s", conson);
      free(conson);
      printf("ay");
    }
  }
}

int main(int argc, char** argv){
  if(argc < 2) return(0);
  for(int k = 1; k < argc; k++){
    count = 0;
    char* instr = setword(argv[k]);
    piglatinconvert(instr, count);
    if(k < argc-1) printf(" ");
  }
  printf("\n");
  return(0);
}
