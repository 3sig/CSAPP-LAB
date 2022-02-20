/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans32(int M, int N, int A[N][M], int B[M][N]);
void trans64(int M, int N, int A[N][M], int B[M][N]);
void trans61(int M, int N, int A[N][M], int B[M][N]);
/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M==32){
        trans32(M, N, A, B);
    }else if(M==64){
        trans64(M,N,A,B);
    }else {
        trans61(M,N,A,B);
    }

}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

void trans32(int M, int N, int A[N][M], int B[M][N]){
    int i,j, a;
    int a1,a2,a3,a4,a5,a6,a7,a8;
    for( i=0;i<M;i+=8){
        for( j=0;j<N;j+=8){
            for( a = 0;a<8;++a){
                a1 = A[i+a][j];
                a2 = A[i+a][j+1];
                a3 = A[i+a][j+2];
                a4 = A[i+a][j+3];
                a5 = A[i+a][j+4];
                a6 = A[i+a][j+5];
                a7 = A[i+a][j+6];
                a8 = A[i+a][j+7];
                B[j][i+a] = a1;
                B[j+1][i+a] = a2;
                B[j+2][i+a] = a3;
                B[j+3][i+a] = a4;
                B[j+4][i+a] = a5;
                B[j+5][i+a] = a6;
                B[j+6][i+a] = a7;
                B[j+7][i+a] = a8;
            }
        }
    }
}

void trans64(int M, int N, int A[N][M], int B[M][N]){

    int i, j, k;
    int a1, a2, a3, a4, a5, a6, a7,a8;
    for( i=0;i<M;i+=8){
        for( j=0;j<N;j+=8){
//            if(i==j){
//                for(a5 = i;a5<i+8;a5+=4){
//                    for(a6 = j;a6<j+8;a6+=4){
//                        for(k=0;k<4;++k){
//                            a1 = A[a5+k][a6+0];
//                            a2 = A[a5+k][a6+1];
//                            a3 = A[a5+k][a6+2];
//                            a4 = A[a5+k][a6+3];
//                            B[a6+0][a5+k] = a1;
//                            B[a6+1][a5+k] = a2;
//                            B[a6+2][a5+k] = a3;
//                            B[a6+3][a5+k] = a4;
//                        }
//                    }
//                }
//                continue;
//            }

            //step 1
            for( k = 0; k< 4;++k){
                a1 = A[i+k][j];
                a2 = A[i+k][j+1];
                a3 = A[i+k][j+2];
                a4 = A[i+k][j+3];
                a5 = A[i+k][j+4];
                a6 = A[i+k][j+5];
                a7 = A[i+k][j+6];
                a8 = A[i+k][j+7];

                B[j][i+k] = a1;
                B[j+1][i+k] = a2;
                B[j+2][i+k] = a3;
                B[j+3][i+k] = a4;
                B[j][i+k+4] = a5;
                B[j+1][i+k+4] = a6;
                B[j+2][i+k+4] = a7;
                B[j+3][i+k+4] = a8;
            }
            for( k=0;k<4;++k){
                a1 = A[i+4][j+k];
                a2 = A[i+5][j+k];
                a3 = A[i+6][j+k];
                a4 = A[i+7][j+k];
                a5 = B[j+k][i+4];
                a6 = B[j+k][i+5];
                a7 = B[j+k][i+6];
                a8 = B[j+k][i+7];
                B[j+k][i+4] = a1;
                B[j+k][i+5] = a2;
                B[j+k][i+6] = a3;
                B[j+k][i+7] = a4;
                B[j+4+k][i+0] = a5;
                B[j+4+k][i+1] = a6;
                B[j+4+k][i+2] = a7;
                B[j+4+k][i+3] = a8;
            }
            for(k = 0;k<4;++k){
                B[j+4][i+4+k] = A[i+4+k][j+4];
                B[j+5][i+4+k] = A[i+4+k][j+5];
                B[j+6][i+4+k] = A[i+4+k][j+6];
                B[j+7][i+4+k] = A[i+4+k][j+7];
            }

        }

    }
}


void trans61(int M, int N, int A[N][M], int B[M][N]){
    int i,j, k,l;
    int blockSize = 17;
    for(i=0;i<N;i+=blockSize){
        for(j=0;j<M;j+=blockSize){
            for(k=i;k<i+blockSize && k<N;k++){
                for(l=j;l<j+blockSize&&l<M;l++){
                    B[l][k]=A[k][l];
                }
            }
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

