#include "utils.h"
#define NAMESIZE 64
#define LINESIZE 8
#define BUFSIZE 32768

/* Read a matrix */
void read_matrix(char *file, double *A, double **a, int trans, int l, int c)
{
    register int i, k = 0;
    int read;
    char *buf = malloc(l * c * 10);
    double val = 0;
    register double *rA = A;
    register double **ra = a;
    FILE *f;

    /* Open input file */
    f = fopen(file, "r");
    /* Read the entire file at once */
    read = fread(buf, sizeof(char), l * c * 10, f);

    if(trans == 0) {
        /* Parse the input */
        for(i = 0; i < read; ++i)
            if(buf[i] == ' ') {
                *(A++) += val / 1000;
                val = 0;
            }
            else if(buf[i] == '.') {
                *A = val;
                val = 0;
            }
            else if('\n' < buf[i])
                val = 10 * val + buf[i] - '0';

        /* Store the start address of each row */
        for(i = 0; i < l; ++i, rA += c)
            *(ra++) = rA;
    }
    /* Store the transpose of the matrix that is read */
    else {
        /* Store the start address of each row */
        for(i = 0; i < c; ++i, rA += l)
            *(ra++) = rA;
        rA = A;

        /* Parse the input */
        for(i = 0; i < read; ++i)
            if(buf[i] == ' ') {
                ++k;
                *rA += val / 1000;
                val = 0;
                if(k == c) {
                    k = 0;
                    rA = ++A;
                }
                else
                    rA += l;
            }
            else if(buf[i] == '.') {
                *rA = val;
                val = 0;
            }
            else if('\n' < buf[i])
                val = 10 * val + buf[i] - '0';
    }

    /* Free buffer memory and close input file */
    free(buf);
    fclose(f);
}

/* Read matrix and multiply it at the same time */
void read_matrix_mult(char *file, double *A, double **a, int trans, int l,
    int c, double alpha)
{
    register int i, k = 0;
    int read;
    char *buf = malloc(l * c * 10);
    double val = 0;
    register double *rA = A;
    register double **ra = a;
    FILE *f;

    /* Open input file */
    f = fopen(file, "r");
    /* Read the entire file at once */
    read = fread(buf, sizeof(char), l * c * 10, f);

    if(trans == 0) {
        /* Parse input */
        for(i = 0; i < read; ++i)
            if(buf[i] == ' ') {
                *A += val / 1000;
                *(A++) *= alpha;
                val = 0;
            }
            else if(buf[i] == '.') {
                *A = val;
                val = 0;
            }
            else if('\n' < buf[i])
                val = 10 * val + buf[i] - '0';

        /* Store the start address of each row */
        for(i = 0; i < l; ++i, rA += c)
            *(ra++) = rA;
    }
    /* Store the transpose of the matrix that is read */
    else {
        /* Store the start address of each row */
        for(i = 0; i < c; ++i, rA += l)
            *(ra++) = rA;
        rA = A;

        /* Parse input */
        for(i = 0; i < read; ++i)
            if(buf[i] == ' ') {
                ++k;
                *rA += val / 1000;
                *rA *= alpha;
                val = 0;
                if(k == c) {
                    k = 0;
                    rA = ++A;
                }
                else
                    rA += l;
            }
            else if(buf[i] == '.') {
                *rA = val;
                val = 0;
            }
            else if('\n' < buf[i])
                val = 10 * val + buf[i] - '0';
    }

    /* Free buffer memory and close input file */
    free(buf);
    fclose(f);
}

/* Write matrix in a file */
void write_matrix(char *file, double *A, int l, int c)
{
    register int i, j, k;
    int pos = 0, val, aux, posDig;
    register double *ra = A;
    char *buf = malloc(l * c * 100);
    char digits[10];
    FILE *f;

    /* Open output file */
    f = fopen(file, "w");

    /* Fill the buffer to be written */
    for(i = 0; i < l; ++i) {
        for(j = 0; j < c; ++j) {
            val = (int)(*ra);
            aux = val;
            posDig = 0;

            while(aux) {
                digits[posDig++] = aux % 10 + '0';
                aux /= 10;
            }

            for(k = posDig - 1; k >= 0; --k)
                buf[pos++] = digits[k];

            buf[pos++] = '.';
            val = (int)((*ra - (double)val) * 1000);
            buf[pos++] = val / 100 + '0';
            buf[pos++] = (val / 10) % 10 + '0';
            buf[pos++] = val % 10 + '0';

            ++ra;
            buf[pos++] = ' ';
        }
        buf[pos++] = '\n';
    }
    buf[pos] = 0;

    /* Write the entire matrix at once */
    fwrite(buf, sizeof(char), pos, f);
    /* Free buffer memory and close the output file */
    free(buf);
    fclose(f);
}

/* Multiply two matrices, the second one being in transpose mode */
void mult(double **a, double **b, double **c, int M, int N, int K)
{
    register int i, j, k;
    register double suma;
    register double *ra, *rb, *rc = c[0];

    for(i = 0; i < M; ++i) {
        for(j = 0; j < N; ++j) {
            suma = 0;
            ra = &a[i][0];
            rb = &b[j][0];

            /* Unroll loop for faster run */
            for(k = 0; k <= K - 8; k += 8) {
                suma += *(ra) * *(rb) +
                        *(ra + 1) * *(rb + 1) +
                        *(ra + 2) * *(rb + 2) +
                        *(ra + 3) * *(rb + 3) +
                        *(ra + 4) * *(rb + 4) +
                        *(ra + 5) * *(rb + 5) +
                        *(ra + 6) * *(rb + 6) +
                        *(ra + 7) * *(rb + 7);
                ra += 8;
                rb += 8;
            }

            for(; k < K; ++k)
                suma += *(ra++) * *(rb++);
            *(rc++) += suma;
        }
    }
}

int main(int argc, char **argv)
{
    int i;
    struct test **tests;
    char filename[NAMESIZE];
    double *A, *B, *C, **a, **b, **c;
    tests = (struct test **)malloc(sizeof(struct test*));
    (*tests) = (struct test *)calloc(MAXTESTS, sizeof(struct test));

    parse_config("tema2.cfg", tests);

    for(i = 0; i < MAXTESTS; ++i) {
        if(!(*tests)[i].active)
            break;

        /* Allocate memoory for the matrices */
        A = malloc((*tests)[i].M * (*tests)[i].K * sizeof(double));
        B = malloc((*tests)[i].K * (*tests)[i].N * sizeof(double));
        C = malloc((*tests)[i].M * (*tests)[i].N * sizeof(double));
        a = malloc((*tests)[i].M * sizeof(double *));
        b = malloc((*tests)[i].N * sizeof(double *));
        c = malloc((*tests)[i].M * sizeof(double *));

        /* Read matrices */
        sprintf(filename, "input/%s_A.in", (*tests)[i].name);
        if((*tests)[i].transa == 'N')
            read_matrix_mult(filename, A, a, 0, (*tests)[i].M, (*tests)[i].K,
                (*tests)[i].alpha);
        else
            read_matrix_mult(filename, A, a, 1, (*tests)[i].M, (*tests)[i].K,
                (*tests)[i].alpha);

        sprintf(filename, "input/%s_B.in", (*tests)[i].name);
        if((*tests)[i].transb == 'N')
            read_matrix(filename, B, b, 1, (*tests)[i].K, (*tests)[i].N);
        else
            read_matrix(filename, B, b, 0, (*tests)[i].K, (*tests)[i].N);

        sprintf(filename, "input/%s_C.in", (*tests)[i].name);
        read_matrix_mult(filename, C, c, 0, (*tests)[i].M, (*tests)[i].N,
            (*tests)[i].beta);

        /* Compute the result */
        mult(a, b, c, (*tests)[i].M, (*tests)[i].N, (*tests)[i].K);

        /* Write the result */
        sprintf(filename, "out/%s.out", (*tests)[i].name);
        write_matrix(filename, C, (*tests)[i].M, (*tests)[i].N);

        /* Free space allocated for matrices */
        free(A);
        free(B);
        free(C);
        free(a);
        free(b);
        free(c);
    }

    free(*tests);
    free(tests);

    return 0;
}
