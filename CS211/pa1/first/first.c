#include<stdio.h>
#include<stdlib.h>

void truncPrimeCheck(int);

void truncPrimeCheck(int check){
  while(check > 0){
    for(int i = 2; i < check/2; i++){
      if(check % i == 0){
	printf("no\n");
	return;
      }
    }
    check /= 10;
  }
  printf("yes\n");
}

int main(int argc, char** argv){
  FILE* fp = fopen(argv[1], "r");
  int num = 0;
  fscanf(fp, "%d\n", &num);
  for(int i = 0; i < num; i++){
    int temp;
    fscanf(fp, "%d\n", &temp);
    truncPrimeCheck(temp);
  }
  fclose(fp);
  return 0;
}
