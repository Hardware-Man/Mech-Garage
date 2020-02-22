#include<stdio.h>
#include<stdlib.h>

void checkmagic(int**, int);

int main(int argc, char** argv){
  FILE* fp = fopen(argv[1], "r");
  int dim;
  fscanf(fp, "%d\n", &dim);
  int** matrix = {NULL};
  //initialize(matrix, dim);
  matrix = malloc(dim * sizeof(int*));
  for(int i = 0; i < dim; i++){
    matrix[i] = calloc(dim, sizeof(int));
  }
  for(int i = 0; i < dim; i++){
    for(int j = 0; j < dim-1; j++){
      fscanf(fp, "%d\t", &matrix[i][j]);
    }
    fscanf(fp, "%d\n", &matrix[i][dim-1]);
  }
  checkmagic(matrix, dim);
  for(int i = 0; i < (sizeof(matrix)/sizeof(int*)); i++){
    free(matrix[i]);
  }
  free(matrix);
  fclose(fp);
  return(0);
}

void checkmagic(int** mat, int n){
  int** matrix = mat;
  const int size = (n*n);
  int dupcheck[size];
  for(int i = 0; i < 9; i++){
    dupcheck[i] = 0;
  }
  int prev = 0;
  int result = 0;
  for(int i = 0; i < n; i++){
    int sumA = 0;
    int sumB = 0;
    for(int j = 0; j < n; j++){
      int ta = matrix[i][j];
      int tb = matrix[j][i];
      if(ta > size){
	result = 1;
	break;
      }
      if(dupcheck[ta-1] == 1){
        result = 1;
	break;
      }
      dupcheck[ta-1] = 1;
      sumA += ta;
      sumB += tb;
    }
    if(sumA != sumB){
      result = 1;
    }
    if(prev == 0){
      prev = sumA;
    }
    if(prev != sumA){
      result = 1;
    }
    if(result == 1){
      printf("not-magic\n");
      return;
    }
  }
  printf("magic\n");
}
