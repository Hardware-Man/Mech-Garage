#include<stdio.h>
#include<stdlib.h>

double** multiply_mat(double**, double**, int, int, int);
double* multiply_row(double*, double, int);
double* subtract_row(double*, double*, int);
double** inverse_mat(double**, int);
void free_mat(double**, int);

double** multiply_mat(double** mata, double** matb, int m, int n, int o){
  double** result = malloc(m * sizeof(double*));
  for(int i = 0; i < m; i++){
    result[i] = malloc(o * sizeof(double));
  }
  for(int i = 0; i < m; i++){
    for(int j = 0; j < o; j++){
      double temp = 0;
      for(int k = 0; k < n; k++){
	temp += (mata[i][k] * matb[k][j]);
      }
      result[i][j] = temp;
    }
  }
  return result;
}

double** inverse_mat(double** mat, int n){
  double** result = malloc(n * sizeof(double*));
  double** computeinv = malloc(n * sizeof(double*));
  for(int i = 0; i < n; i++){
    result[i] = malloc(n * sizeof(double));
    computeinv[i] = malloc(2 * n * sizeof(double));
    for(int j = 0; j < n; j++){
      computeinv[i][j] = mat[i][j];
      if(i == j) computeinv[i][j+n] = 1.0;
      else computeinv[i][j+n] = 0.0;
    }
  }
  for(int i = 0; i < n; i++){
    double fac = (1.0/computeinv[i][i]);
    if(computeinv[i][i] != 1.0) computeinv[i] = multiply_row(computeinv[i], fac, 2 * n);
    for(int j = 0; j < n; j++){
      if(i != j){
        computeinv[j] = subtract_row(computeinv[j], multiply_row(computeinv[i], computeinv[j][i], 2 * n), 2 * n);
      }
    }
  }
  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      result[i][j] = computeinv[i][j+n];
    }
  }
  return result;
}

double* multiply_row(double* row, double factor, int n){
  double* res = malloc(n * sizeof(double));
  for(int i = 0; i < n; i++){
    res[i] = row[i] * factor;
  }
  return res;
}

double* subtract_row(double* rowa, double* rowb, int n){
  double* res = malloc(n * sizeof(double));
  for(int i = 0; i < n; i++){
    res[i] = rowa[i] - rowb[i];
  }
  return res;
}

void free_mat(double** mat, int n){
  for(int i = 0; i < n; i++){
    free(mat[i]);
  }
  free(mat);
}

int main(int argc, char** argv){
  FILE* fp = fopen(argv[1], "r");
  double** matX;
  double** matW;
  double** matY;
  int K;
  int N;
  fscanf(fp, "%d\n", &K);
  fscanf(fp, "%d\n", &N);
  matX = malloc(N * sizeof(double*));
  matY = malloc(N * sizeof(double*));
  for(int i = 0; i < N; i++){
    matX[i] = malloc((K+1) * sizeof(double));
    matX[i][0] = 1.0;
    matY[i] = malloc(sizeof(double));
    for(int j = 0; j < K; j++){
      fscanf(fp, "%lf,", &matX[i][j+1]);
    }
    fscanf(fp, "%lf\n", &matY[i][0]);
  }
  double** matXT = malloc((K+1) * sizeof(double*));
  for(int i = 0; i < (K+1); i++){
    matXT[i] = malloc(N * sizeof(double));
    for(int j = 0; j < N; j++){
      matXT[i][j] = matX[j][i];
    }
  }
  double** matXTXinv = inverse_mat(multiply_mat(matXT, matX, (K+1), N, (K+1)), (K+1));
  double** matpsuedoinv = multiply_mat(matXTXinv, matXT, (K+1), (K+1), N);
  matW = multiply_mat(matpsuedoinv, matY, (K+1), N, 1);
  free_mat(matX, N);
  free_mat(matY, N);
  free_mat(matXT, (K+1));
  free_mat(matXTXinv, (K+1));
  free_mat(matpsuedoinv, (K+1));
  fp = fopen(argv[2], "r");
  fscanf(fp, "%d\n", &N);
  matX = malloc(N * sizeof(double*));
  matY = malloc(N * sizeof(double*));
  for(int i = 0; i < N; i++){
    matX[i] = malloc((K+1) * sizeof(double));
    matX[i][0] = 1.0;
    matY[i] = malloc(sizeof(double));
    for(int j = 0; j < K; j++){
      fscanf(fp, "%lf,", &matX[i][j+1]);
    } 
  }
  matY = multiply_mat(matX, matW, N, (K+1), 1);
  for(int i = 0; i < N; i++){
    printf("%0.0lf\n", matY[i][0]);
  }
  free_mat(matX, N);
  free_mat(matY, N);
  free_mat(matW, (K+1));
  fclose(fp);
  return(0);
}
