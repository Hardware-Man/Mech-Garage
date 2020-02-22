#include<stdio.h>
#include<stdlib.h>

int main(int argc, char** argv){
  if(argc < 2) return 0;
  for(int i = 1; i < argc; i++){
    for(int j = 0; argv[i][j] != '\0'; j++){
      if(argv[i][j+1] == '\0'){
        printf("%c", argv[i][j]);
	break;
      }
    }
  }
  printf("\n");
  return 0;
}

