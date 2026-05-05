#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define EPS 1e-3

typedef struct {
    size_t rows;
    size_t columns;
    double *data;
} DMatrix;

typedef struct {
    size_t rows;
    size_t columns;
    int *data;
} IMatrix;

DMatrix dmatrix_init(size_t rows, size_t columns) {
    DMatrix matrix = {.rows = rows, .columns = columns, .data = NULL};
    size_t size = sizeof(*matrix.data) * rows * columns;
    matrix.data = malloc(size);

    if (matrix.data == NULL) {
        perror("dmatrix_init malloc failed");
        exit(1);
    }
    return matrix;
}

DMatrix dmatrix_init_rand(size_t rows, size_t columns) {
    DMatrix matrix = dmatrix_init(rows, columns);

    for (double *p = matrix.data; p < matrix.data + rows * columns; p++) {
        *p = 100 * (double)rand() / RAND_MAX;
    }

    return matrix;
}

void dmatrix_free(DMatrix mat) { free(mat.data); }
void dmatrix_free_ptr(DMatrix *mat) { free(mat->data); }

void dmatrix_free_many(size_t count, ...) {
    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i++) {
        DMatrix ptr = va_arg(args, DMatrix);
        dmatrix_free(ptr);
    }

    va_end(args);
}

void dmatrix_copy(DMatrix src, DMatrix *dest) {
    dest->rows = src.rows;
    dest->columns = src.columns;
    free(dest->data);
    dest->data = malloc(sizeof(*dest->data) * src.rows * src.columns);
    if (dest->data == NULL) {
        perror("dmatrix_copy malloc failed");
        exit(1);
    }
    memcpy(dest->data, src.data, sizeof(*dest->data) * src.rows * src.columns);
}

void dmatrix_print(DMatrix mat) {
    if (mat.rows == 0 || mat.columns == 0) return;

    for (size_t i = 0; i < mat.rows; i++) {
        for (size_t j = 0; j < mat.columns; j++) {
            printf("%lf ", mat.data[j + mat.columns * i]);
        }
        printf("\n");
    }
}

int dmatrix_check_equal(DMatrix a, DMatrix b) {
    if (a.rows != b.rows || a.columns != b.columns) {
        return -1;
    }

    if (a.data == NULL || b.data == NULL) {
        if (a.data == NULL && b.data == NULL && a.rows == 0 && b.rows == 0)
            return 1;
        return -1;
    }

    for (size_t i = 0; i < a.rows * a.columns; i++) {
        if (fabs(a.data[i] - b.data[i]) > EPS) {
            printf("at position %ld it should be %lf but is %lf\n", i,
                   a.data[i], b.data[i]);
            return -1;
        }
    }
    return 1;
}

/* Add: Rectangular access with add/mul overhead */
int dmatrix_add_bad(DMatrix a, DMatrix b, DMatrix *c) {
    if (a.rows != b.rows || a.columns != b.columns) {
        fprintf(stderr, "dmatrix_add_bad, dimensions of inputs don't match\n");
        return -1;
    }

    if (c->data != NULL) free(c->data);

    c->rows = a.rows;
    c->columns = a.columns;
    c->data = malloc(sizeof(*c->data) * a.rows * a.columns);
    if (c->data == NULL) {
        perror("dmatrix_add_bad malloc failed");
        return -1;
    }

    for (size_t i = 0; i < a.columns; i++) {
        for (size_t j = 0; j < a.rows; j++) {
            c->data[i + a.columns * j] =
                a.data[i + a.columns * j] + b.data[i + a.columns * j];
        }
    }

    return 1;
}

/* Add: Linear access */
int dmatrix_add(DMatrix a, DMatrix b, DMatrix *c) {
    if (a.rows != b.rows || a.columns != b.columns) {
        fprintf(stderr, "dmatrix_add, dimensions of inputs don't match\n");
        return -1;
    }

    if (c->data != NULL) free(c->data);

    c->rows = a.rows;
    c->columns = a.columns;
    c->data = malloc(sizeof(*c->data) * a.rows * a.columns);
    if (c->data == NULL) {
        perror("dmatrix_add malloc failed");
        return -1;
    }

    for (size_t i = 0; i < a.rows * a.columns; i++) {
        c->data[i] = a.data[i] + b.data[i];
    }

    return 1;
}

/* Multiply: ijk */
int dmatrix_multiply_bad(DMatrix a, DMatrix b, DMatrix *c) {
    if (b.rows != a.columns) {
        fprintf(stderr,
                "dmatrix_multiply_bad, dimensions of inputs don't match\n");
        return -1;
    }

    if (c->data != NULL) free(c->data);

    size_t rows = a.rows;
    size_t columns = b.columns;

    c->rows = rows;
    c->columns = columns;
    c->data = malloc(sizeof(*c->data) * rows * columns);
    if (c->data == NULL) {
        perror("dmatrix_multiply_bad malloc failed");
        return -1;
    }
    memset(c->data, 0, sizeof(*c->data) * rows * columns);

    for (size_t i = 0; i < rows; i++) {
        double *ap_orig = &a.data[a.columns * i];
        for (size_t j = 0; j < columns; j++) {
            double *start = &(c->data[j + columns * i]);
            double val = *start;
            double *ap = ap_orig;
            double *bp = &b.data[j];
            for (size_t k = 0; k < a.columns; k++) {
                val += *(ap++) * *bp;
                bp += b.columns;
            }
            *start = val;
        }
    }

    return 1;
}

/* Multiply: ikj */
int dmatrix_multiply(DMatrix a, DMatrix b, DMatrix *c) {
    if (b.rows != a.columns) {
        fprintf(stderr, "dmatrix_multiply, dimensions of inputs don't match\n");
        return -1;
    }

    if (c->data != NULL) free(c->data);

    size_t rows = a.rows;
    size_t columns = b.columns;

    c->rows = rows;
    c->columns = columns;
    c->data = malloc(sizeof(*c->data) * rows * columns);
    if (c->data == NULL) {
        perror("dmatrix_multiply malloc failed");
        return -1;
    }
    memset(c->data, 0, sizeof(*c->data) * rows * columns);

    for (size_t i = 0; i < rows; i++) {
        double *c_start_orig = &c->data[columns * i];
        double *b_start = &b.data[0];
        for (size_t k = 0; k < a.columns; k++) {
            double a_val = a.data[k + a.columns * i];
            double *c_start = c_start_orig;
            for (size_t j = 0; j < columns; j++) {
                *(c_start++) += a_val * *(b_start++);
            }
        }
    }

    return 1;
}

/* Multiply: Tiling */
int dmatrix_multiply_better(DMatrix a, DMatrix b, DMatrix *c) {
    if (b.rows != a.columns) {
        fprintf(stderr,
                "dmatrix_multiply_better, dimensions of inputs don't match\n");
        return -1;
    }

    if (c->data != NULL) free(c->data);

    size_t rows = a.rows;
    size_t columns = b.columns;

    c->rows = rows;
    c->columns = columns;
    c->data = malloc(sizeof(*c->data) * rows * columns);
    if (c->data == NULL) {
        perror("dmatrix_multiply_better malloc failed");
        return -1;
    }
    memset(c->data, 0, sizeof(*c->data) * rows * columns);

    size_t tile_size = 200;  // TODO: val? sqrt(M)?

    for (size_t i = 0; i < rows; i += tile_size) {
        for (size_t j = 0; j < columns; j += tile_size) {
            for (size_t k = 0; k < a.columns; k += tile_size) {
                for (size_t i1 = i; i1 < MIN(i + tile_size, rows); i1++) {
                    double *c_start_orig = &c->data[j + columns * i1];
                    double *a_start = &a.data[k + a.columns * i1];
                    for (size_t k1 = k; k1 < MIN(k + tile_size, a.columns);
                         k1++) {
                        double a_val = *(a_start++);
                        double *b_start = &b.data[j + columns * k1];
                        double *c_start = c_start_orig;
                        for (size_t j1 = j; j1 < MIN(j + tile_size, columns);
                             j1++) {
                            *(c_start++) += a_val * *(b_start++);
                        }
                    }
                }
            }
        }
    }

    return 1;
}

/* Transpose: In-place swapping */
void dmatrix_transpose(DMatrix *mat) {
    size_t rows = mat->rows;
    size_t columns = mat->columns;
    const size_t N = rows * columns;

    mat->rows = columns;
    mat->columns = rows;

    if (rows == 0 || columns == 0 || mat->data == NULL) return;

    if (rows == columns) {  // square matrix shortcut
        for (size_t i = 0; i < rows - 1; i++) {
            for (size_t j = i + 1; j < columns; j++) {
                double tmp = mat->data[i + columns * j];
                mat->data[i + columns * j] = mat->data[j + columns * i];
                mat->data[j + columns * i] = tmp;
            }
        }
        return;
    }

    _Bool *visited = malloc(sizeof(_Bool) * N);
    if (visited == NULL) {
        perror("dmatrix_transponse_bad malloc failed");
        exit(1);
    }
    memset(visited, 0, sizeof(*visited) * N);
    visited[0] = 1;
    visited[N - 1] = 1;

    size_t i = 1;
    while (1) {
        size_t nl = N + 1;
        const size_t i_copy = i;
        double val = mat->data[i];
        while (nl != i_copy && !visited[i]) {
            visited[i] = 1;
            nl = (i * rows) % (N - 1);
            double val_temp = mat->data[nl];
            mat->data[nl] = val;
            val = val_temp;
            i = nl;
        }

        for (i = 1; i < N - 1; i++) {
            if (visited[i] == 0) {
                break;
            }
        }
        if (i == N - 1) break;
    }
}

double dmatrix_det_bad_impl(DMatrix mat) {
    if (mat.rows == 0) return 0;
    if (mat.rows == 1) {
        return mat.data[0];
    }
    size_t n = mat.rows;

    double ans = 0;
    DMatrix mat_new = dmatrix_init(n - 1, n - 1);

    for (size_t col = 0; col < n; col++) {
        size_t mi = 0;
        for (size_t i = 1; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
                if (j == col) continue;
                mat_new.data[mi++] = mat.data[j + n * i];
            }
        }

        double sign = (col % 2 == 0) ? 1 : -1;
        if (mat.data[col] != 0)
            ans += sign * mat.data[col] * dmatrix_det_bad_impl(mat_new);
    }
    dmatrix_free(mat_new);
    return ans;
}

int dmatrix_lu_decomposition_pivot(DMatrix *A, DMatrix *L, DMatrix *U,
                                   size_t *perm, int *swaps) {
    if (A->rows != A->columns) return -1;

    size_t n = A->rows;
    free(L->data);
    free(U->data);
    L->rows = A->rows;
    L->columns = A->columns;
    U->rows = A->rows;
    U->columns = A->columns;

    L->data = calloc(n * n, sizeof(double));
    U->data = calloc(n * n, sizeof(double));

    if (!L->data || !U->data) {
        free(L->data);
        free(U->data);
        L->data = NULL;
        U->data = NULL;
        return -1;
    }

    DMatrix tmp = {0};
    dmatrix_copy(*A, &tmp);
    if (swaps != NULL) *swaps = 0;
    if (perm != NULL)
        for (size_t i = 0; i < n; i++) perm[i] = i;

    for (size_t k = 0; k < n; k++) {
        size_t pivot = k;
        double max = fabs(tmp.data[k + k * n]);
        for (size_t i = k + 1; i < n; i++) {
            double val = fabs(tmp.data[k + i * n]);
            if (val > max) {
                max = val;
                pivot = i;
            }
        }

        if (max == 0.0) {
            dmatrix_free(tmp);
            return -1;
        }

        if (pivot != k) {
            for (size_t j = 0; j < n; j++) {
                double t = tmp.data[j + k * n];
                tmp.data[j + k * n] = tmp.data[j + pivot * n];
                tmp.data[j + pivot * n] = t;
            }
            if (perm != NULL) {
                size_t t = perm[k];
                perm[k] = perm[pivot];
                perm[pivot] = t;
            }
            if (swaps != NULL) (*swaps)++;
        }

        for (size_t i = k + 1; i < n; i++) {
            tmp.data[k + i * n] /= tmp.data[k + k * n];
            for (size_t j = k + 1; j < n; j++) {
                tmp.data[j + i * n] -=
                    tmp.data[k + i * n] * tmp.data[j + k * n];
            }
        }
    }

    for (size_t i = 0; i < n; i++) {
        L->data[i + i * n] = 1.0;
        for (size_t j = 0; j < n; j++) {
            if (i > j)
                L->data[j + i * n] = tmp.data[j + i * n];
            else
                U->data[j + i * n] = tmp.data[j + i * n];
        }
    }

    dmatrix_free(tmp);

    return 0;
}

int dmatrix_lu_decomposition(DMatrix *A, DMatrix *L, DMatrix *U) {
    return dmatrix_lu_decomposition_pivot(A, L, U, NULL, NULL);
}

/* Determinant: Laplace Expansion Cofactors */
int dmatrix_det_bad(DMatrix mat, double *ans) {
    *ans = 0;
    if (mat.columns != mat.rows) {
        fprintf(stderr, "dmatrix_det_bad, matrix should be a square matrix\n");
        return -1;
    }
    *ans = dmatrix_det_bad_impl(mat);
    return 1;
}

/* Determinant: LU Decomposition */
int dmatrix_det(DMatrix mat, double *ans) {
    *ans = 0;
    if (mat.columns != mat.rows) {
        fprintf(stderr, "dmatrix_det, matrix should be a square matrix\n");
        return -1;
    }
    DMatrix L = {0}, U = {0};
    int swaps = 0;
    int success = dmatrix_lu_decomposition_pivot(&mat, &L, &U, NULL, &swaps);
    if (success == -1) {
        dmatrix_free_many(2, L, U);
        return 1;
    }

    *ans = swaps % 2 == 0 ? 1 : -1;
    for (size_t i = 0; i < mat.rows; i++) {
        *ans *= U.data[i + i * mat.rows];
    }

    dmatrix_free_many(2, L, U);

    return 1;
}

void getCof(DMatrix *mat, DMatrix *cof, size_t p, size_t q, size_t n) {
    size_t i = 0, j = 0;

    for (size_t row = 0; row < n; row++) {
        for (size_t col = 0; col < n; col++) {
            if (row != p && col != q) {
                cof->data[j++ + i * (n - 1)] = mat->data[col + row * n];
                if (j == n - 1) {
                    j = 0;
                    i++;
                }
            }
        }
    }
}

void adjoint(DMatrix mat, DMatrix *adj) {
    size_t n = mat.rows;

    if (n == 1) {
        adj->data[0] = 1;
        return;
    }

    int sign = 1;
    DMatrix cof = dmatrix_init(n - 1, n - 1);

    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            getCof(&mat, &cof, i, j, n);
            sign = (i + j) % 2 == 0 ? 1 : -1;
            double det;
            dmatrix_det(cof, &det);
            adj->data[j + i * n] = sign * det;
        }
    }
    dmatrix_free(cof);
}

/* Inverse: Adjugate Method */
int dmatrix_inverse_bad(DMatrix mat, DMatrix *inv) {
    if (mat.columns != mat.rows) {
        fprintf(stderr,
                "dmatrix_inverse_bad, matrix should be a square matrix\n");
        return -1;
    }
    free(inv->data);

    double det = 0;
    dmatrix_det(mat, &det);
    if (fabs(det) < EPS) {
        fprintf(stderr, "dmatrix_inverse_bad, matrix is singular\n");
        return -1;
    }

    inv->rows = mat.rows;
    inv->columns = mat.columns;
    inv->data = malloc(sizeof(*inv->data) * inv->rows * inv->columns);
    if (inv->data == NULL) {
        fprintf(stderr, "dmatrix_inverse_bad, malloc failed\n");
        return -1;
    }

    adjoint(mat, inv);
    dmatrix_transpose(inv);

    for (size_t i = 0; i < mat.rows * mat.columns; i++) {
        inv->data[i] /= det;
    }

    return 1;
}

/* Inverse: LU Decomposition */
int dmatrix_inverse(DMatrix mat, DMatrix *inv) {
    if (mat.columns != mat.rows) {
        fprintf(stderr, "dmatrix_inverse, matrix should be a square matrix\n");
        return -1;
    }

    double det = 0;
    dmatrix_det(mat, &det);
    if (det == 0) {
        fprintf(stderr, "dmatrix_inverse, matrix is singular\n");
        return -1;
    }

    free(inv->data);
    inv->data = NULL;

    DMatrix L = {0}, U = {0};
    size_t *perm = malloc(sizeof(*perm) * mat.rows);
    if (perm == NULL) {
        perror("dmatrix_inverse malloc failed");
        return -1;
    }
    int ret = dmatrix_lu_decomposition_pivot(&mat, &L, &U, perm, NULL);
    if (ret == -1) {
        dmatrix_free_many(2, L, U);
        free(perm);
        return -1;
    }

    inv->rows = mat.rows;
    inv->columns = mat.columns;
    inv->data = malloc(sizeof(*inv->data) * inv->rows * inv->columns);
    if (inv->data == NULL) {
        perror("dmatrix_inverse malloc failed");
        dmatrix_free_many(2, L, U);
        free(perm);
        return -1;
    }

    double *y = malloc(sizeof(*y) * mat.rows);
    double *x = malloc(sizeof(*x) * mat.rows);
    if (y == NULL || x == NULL) {
        perror("dmatrix_inverse malloc failed");
        free(y);
        free(x);
        free(inv->data);
        inv->data = NULL;
        dmatrix_free_many(2, L, U);
        free(perm);
        return -1;
    }

    size_t n = mat.rows;
    for (size_t col = 0; col < n; col++) {
        for (size_t i = 0; i < n; i++) {
            double sum = perm[i] == col ? 1.0 : 0.0;
            for (size_t j = 0; j < i; j++) sum -= L.data[j + i * n] * y[j];
            y[i] = sum;
        }

        for (size_t i = n; i-- > 0;) {
            double sum = y[i];
            for (size_t j = i + 1; j < n; j++) sum -= U.data[j + i * n] * x[j];
            x[i] = sum / U.data[i + i * n];
        }

        for (size_t i = 0; i < n; i++) inv->data[col + i * n] = x[i];
    }

    free(y);
    free(x);
    dmatrix_free_many(2, L, U);
    free(perm);

    return 1;
}

#define BENCH_ITERATIONS 21  // odd to see transpose results
#define BENCH(label, ...)                                            \
    do {                                                             \
        struct timespec _start, _end;                                \
        clock_gettime(CLOCK_MONOTONIC, &_start);                     \
        for (int i = 0; i < BENCH_ITERATIONS; i++) {                 \
            __VA_ARGS__                                              \
        }                                                            \
        clock_gettime(CLOCK_MONOTONIC, &_end);                       \
        double _ms = (double)(_end.tv_sec - _start.tv_sec) * 1e3 +   \
                     (double)(_end.tv_nsec - _start.tv_nsec) * 1e-6; \
        printf("%s: %.06lf ms\n", label, _ms / BENCH_ITERATIONS);    \
    } while (0)

int main(void) {
    srand(0);
#if 1
    DMatrix A = dmatrix_init_rand(200, 300);
    DMatrix B = dmatrix_init_rand(200, 300);
    DMatrix C = dmatrix_init_rand(200, 2000);
    DMatrix D = dmatrix_init_rand(2000, 400);
    DMatrix E = dmatrix_init_rand(10, 10);
#else
    DMatrix A = dmatrix_init_rand(100, 100);
    DMatrix B = dmatrix_init_rand(100, 100);
    DMatrix C = dmatrix_init_rand(2, 3);
    C.data[0] = 1;
    C.data[1] = 2;
    C.data[2] = 3;
    C.data[3] = 4;
    C.data[4] = 5;
    C.data[5] = 6;
    DMatrix D = dmatrix_init_rand(3, 2);
    D.data[0] = 10;
    D.data[1] = 11;
    D.data[2] = 20;
    D.data[3] = 21;
    D.data[4] = 30;
    D.data[5] = 31;
    DMatrix E = dmatrix_init_rand(3, 3);
    E.data[0] = -1;
    E.data[1] = 2;
    E.data[2] = 3;
    E.data[3] = 4;
    E.data[4] = 5;
    E.data[5] = 6;
    E.data[6] = 7;
    E.data[7] = 8;
    E.data[8] = 9;
#endif

    /* BENCH("dmatrix_init_rand", { */
    /*     DMatrix A = dmatrix_init_rand(100, 100); */
    /*     DMatrix B = dmatrix_init_rand(100, 100); */
    /* }); */

    DMatrix AR = {0}, AR2 = {0};
    BENCH("dmatrix_add_bad", { dmatrix_add_bad(A, B, &AR); });
    BENCH("dmatrix_add", { dmatrix_add(A, B, &AR2); });
    assert(dmatrix_check_equal(AR, AR2) == 1 && "AR and AR2 are not equal");

    DMatrix MR = {0}, MR2 = {0}, MR3 = {0};
    BENCH("dmatrix_multiply_bad", {
        dmatrix_multiply_bad(C, D, &MR);
        volatile double sink = MR.data[0];
    });
    BENCH("dmatrix_multiply", {
        dmatrix_multiply(C, D, &MR2);
        volatile double sink = MR2.data[0];
    });
    BENCH("dmatrix_multiply_better", {
        dmatrix_multiply_better(C, D, &MR3);
        volatile double sink = MR3.data[0];
    });
    assert(dmatrix_check_equal(MR, MR2) == 1 &&
           dmatrix_check_equal(MR, MR3) == 1 &&
           "MR, MR2 and MR3 are not equal");

    DMatrix T1 = {0};
    dmatrix_copy(C, &T1);
    BENCH("dmatrix_transpose", { dmatrix_transpose(&T1); });

    double det1 = 0, det2 = 0;
    BENCH("dmatrix_det_bad", { dmatrix_det_bad(E, &det1); });
    BENCH("dmatrix_det", { dmatrix_det(E, &det2); });
    assert((det1 - det2) < EPS && "det1 and det2 are not equal");

    DMatrix IR = {0}, IR2 = {0};
    BENCH("dmatrix_inverse_bad", { dmatrix_inverse_bad(E, &IR); });
    BENCH("dmatrix_inverse", { dmatrix_inverse(E, &IR2); });
    assert(dmatrix_check_equal(IR, IR2) == 1 && "IR and IR2 are not equal");

    dmatrix_free_many(12, A, B, AR, AR2, MR, MR2, MR3, C, D, T1, IR, IR2);

    return 0;
}
