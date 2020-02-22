#include<stdio.h>
#include<stdlib.h>

void freematrix(int**);
void multiplymat(int**, int**);

void freematrix(int** mat){
  for(int i = 0; i < (sizeof(mat)/sizeof(int*)); i++){
    free(mat[i]);
  }
  free(mat);
}

int main(int argc, char** argv){
  FILE* fp = fopen(argv[1], "r");
  int** mata;
  int** matb;
  int m;
  int n;
  int o;
  int p;
  fscanf(fp, "%d\t%d\n", &m, &n);
  mata = malloc(m * sizeof(int*));
  for(int i = 0; i < m; i++){
    mata[i] = malloc(n * sizeof(int));
  }
  for(int i = 0; i < m; i++){
    for(int j = 0; j < n; j++){
      if(j != n-1){
	fscanf(fp, "%d\t", &mata[i][j]);
      }
      else{
	fscanf(fp, "%d\n", &mata[i][j]);
      }
    }
  }
  fscanf(fp, "%d\t%d\n", &o, &p);
  if(n != o){
    freematrix(mata);
    printf("bad-matrices\n");
    exit(0);
  }
  matb = malloc(o * sizeof(int*));
  for(int i = 0; i < o; i++){
    matb[i] = malloc(p * sizeof(int));
  }
  for(int i = 0; i < o; i++){
    for(int j = 0; j < p; j++){
      if(j != p-1){
	fscanf(fp, "%d\t", &matb[i][j]);
      }
      else{
	fscanf(fp, "%d\n", &matb[i][j]);
      }
    }
  }
  // Multiplication of matrices and output
  for(int i = 0; i < m; i++){
    for(int j = 0; j < p; j++){
      int temp = 0;
      for(int l = 0; l < n; l++){
	temp += mata[i][l] * matb[l][j];
      }
      if(j != p-1){
	printf("%d\t", temp);
      }
      else{
	printf("%d\n", temp);
      }
    }
  }
  freematrix(mata);
  freematrix(matb);
  fclose(fp);
  return(0);
}
