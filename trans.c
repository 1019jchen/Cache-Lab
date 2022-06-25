/*
 * Name: James Chen
 * ID: jyc8938
 * 
 */
//#include <stdio.h>
#include "lab3.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int blockForRow, blockForCol;//helps to divide matrix A into blocks 
    int r, c;//iterate through the rows and columns of A
    int w0;//the following variables w0 through w7 are used in different ways to temporarily store values of A
    int w1;
    int w2;
    int w3;
    int w4;
    int w5;
    int w6;
    int w7;
    //32x32 case
    if (N == 32) {
        for (blockForCol = 0; blockForCol < M; blockForCol += 8) {
            for (blockForRow = 0; blockForRow < N; blockForRow += 8) {
                for (r = blockForRow; r < blockForRow + 8; r++) {
                    for (c = blockForCol; c < blockForCol + 8; c++) {
                        if (r != c) {
                            B[c][r] = A[r][c];//only transpose from block in A into block in B if not on the diagonal
                        } else {
                            w0 = A[r][c];//store the diagonal element in w0
                            w1 = r;//store diagonal row/column index in w1
                        }
                    }
                    if (blockForRow == blockForCol) {
                        B[w1][w1] = w0;//move the diagonal element into the corresponding place in B at the end
                    }
                }
            }
        }
    } else if (N == 64) {//64x64 case
        for (blockForCol = 0; blockForCol < M; blockForCol += 8) {
            for (blockForRow = 0; blockForRow < N; blockForRow += 8) {
                for (r = blockForRow; r < blockForRow + 4; r++) {
                    w0 = A[r][blockForCol];//store an entire row of the current block in A into w0 through w7
                    w1 = A[r][blockForCol + 1];
                    w2 = A[r][blockForCol + 2];
                    w3 = A[r][blockForCol + 3];
                    w4 = A[r][blockForCol + 4];
                    w5 = A[r][blockForCol + 5];
                    w6 = A[r][blockForCol + 6];
                    w7 = A[r][blockForCol + 7];
                    B[blockForCol][r] = w0;//store the first half of the row into the right column in B
                    B[blockForCol + 1][r] = w1;
                    B[blockForCol + 2][r] = w2;
                    B[blockForCol + 3][r] = w3;
                    B[blockForCol][r + 4] = w4;//intentionally store the second half of the row into the wrong column of B in the upper right quadrant
                    B[blockForCol + 1][r + 4] = w5;
                    B[blockForCol + 2][r + 4] = w6;
                    B[blockForCol + 3][r + 4] = w7;
                }
                for (c = blockForCol; c < blockForCol + 4; c++) {

                   //store elements from second half of a column in the bottom left quadrant of A
                    w4 = A[blockForRow + 4][c];
                    w5 = A[blockForRow + 5][c];
                    w6 = A[blockForRow + 6][c];
                    w7 = A[blockForRow + 7][c];

                  //store elements in upper right quadrant of B that we intentionally misplaced
                    w0 = B[c][blockForRow + 4];
                    w1 = B[c][blockForRow + 5];
                    w2 = B[c][blockForRow + 6];
                    w3 = B[c][blockForRow + 7];

                    //put elements from bottom left quadrant of A into upper right quadrant of B, overwriting our previously wrong values
                    B[c][blockForRow + 4] = w4;
                    B[c][blockForRow + 5] = w5;
                    B[c][blockForRow + 6] = w6;
                    B[c][blockForRow + 7] = w7;

                    //store elements that were originally from upper right quadrant of B into the lower right quadrant of B
                    B[c + 4][blockForRow] = w0;
                    B[c + 4][blockForRow + 1] = w1;
                    B[c + 4][blockForRow + 2] = w2;
                    B[c + 4][blockForRow + 3] = w3;

                   //directly transfer data from bottom right quadrant of A into bottom right quadrant of B 
                    B[c + 4][blockForRow + 4] = A[blockForRow + 4][c + 4];
                    B[c + 4][blockForRow + 5] = A[blockForRow + 5][c + 4];
                    B[c + 4][blockForRow + 6] = A[blockForRow + 6][c + 4];
                    B[c + 4][blockForRow + 7] = A[blockForRow + 7][c + 4];
                }
            }
        }
    } else {//last case
        for (blockForCol = 0; blockForCol < M; blockForCol += 16) {
            for (blockForRow = 0; blockForRow < N; blockForRow += 16) {
                for (r = blockForRow; (r < N) && (r < blockForRow + 16); r++) {//make sure that the row does not go past N
                    for (c = blockForCol; (c < M) && (c < blockForCol + 16); c++) {//make sure that the row does not go past M
                        if (r != c) {
                            B[c][r] = A[r][c];
                        } else {
                            w0 = A[r][c];
                            w1 = r;
                        }
                    }
                    if (blockForRow == blockForCol) {
                        B[w1][w1] = w0;
                    }
                }
            }
        }
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
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
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
void registerFunctions() {
    /* register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
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
