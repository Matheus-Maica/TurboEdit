#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include "nvector.h"
#include "main.h"

#define C 299792458.0 // Speed of light
#define FREQ_L1 1575.42e6 // L1 Frequency in Hz
#define FREQ_L2 1227.60e6 // L2 Frequency in Hz
#define min(a, b) (((a) < (b)) ? (a) : (b))

// Wide-lane wavelength
const double LAMBDA_WL = C / (FREQ_L1 - FREQ_L2);
const double LAMBDA_WL_INV = (FREQ_L1 - FREQ_L2) / C;

// L1 and L2 wavelengths
const double LAMBDA_L1 = C / FREQ_L1;
const double LAMBDA_L2 = C / FREQ_L2;
const double FREQ1_RATIO = FREQ_L1 / (FREQ_L1 - FREQ_L2);

typedef struct {
    double* ip;
    double* ir;
    double* b_delta;
} MWPIRComb;

void print_list(double* arr, size_t size) {
    // size_t size = sizeof(arr) / sizeof(arr[0]);
    printf("arr (size: %zu): [", size);
    for (size_t i = 0; i < size; ++i) {
        printf("%.2f", arr[i]);
        if (i < size - 1)
            printf(", ");
    }
    printf("]\n");
}

double* generate_random_array(size_t N) {
    double* array = (double*)malloc(N * sizeof(double));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    for (size_t i = 0; i < N; ++i) {
        array[i] = (double)rand() / RAND_MAX * 100.0;  // scale rand() to [0, 100)
    }

    return array;
}

double* copy_array(const double* arr, size_t length) {
    double* new_array = (double*)malloc(length * sizeof(double));

    cblas_dcopy(length, arr, 1, new_array, 1);

    return new_array;
}

double* vector_sum(const double* a, const double* b, unsigned char coeff, size_t length) { // a + b or a - b
    // coeff = -1 => a - b
    // coeff = 1 => a + b
    double* result = (double*)malloc(length * sizeof(double));
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // result = a (copy a into result)
    cblas_dcopy(length, a, 1, result, 1);

    // result = result - b → result = a - b
    // This is done by result += -1.0 * b
    cblas_daxpy(length, coeff, b, 1, result, 1);

    return result;
}

double* vector_mult(const Vector* a, double alpha) { // a * alpha
    const size_t length = a->size;
    double* result = (double*)malloc(length * sizeof(double));
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    cblas_dcopy(length, a->data, 1, result, 1);
    cblas_dscal(length, alpha, result, 1);

    return result;
}

void solve_linear_system(double* A, double* b, double* x, int n) {
    // A is n x n matrix, b is n vector
    // Solves A x = b, stores result in x
    double* augmented = malloc(n * (n + 1) * sizeof(double));
    
    // Build augmented matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            augmented[i*(n+1) + j] = A[i*n + j];
        augmented[i*(n+1) + n] = b[i];
    }

    // Forward elimination
    for (int i = 0; i < n; i++) {
        // Pivoting
        double max = fabs(augmented[i*(n+1)+i]);
        int max_row = i;
        for (int k = i+1; k < n; k++) {
            if (fabs(augmented[k*(n+1)+i]) > max) {
                max = fabs(augmented[k*(n+1)+i]);
                max_row = k;
            }
        }
        if (max_row != i) {
            for (int k = 0; k < n+1; k++) {
                double tmp = augmented[i*(n+1)+k];
                augmented[i*(n+1)+k] = augmented[max_row*(n+1)+k];
                augmented[max_row*(n+1)+k] = tmp;
            }
        }

        // Eliminate below
        for (int k = i+1; k < n; k++) {
            double factor = augmented[k*(n+1)+i] / augmented[i*(n+1)+i];
            for (int j = i; j < n+1; j++)
                augmented[k*(n+1)+j] -= factor * augmented[i*(n+1)+j];
        }
    }

    // Back substitution
    for (int i = n-1; i >= 0; i--) {
        x[i] = augmented[i*(n+1)+n];
        for (int j = i+1; j < n; j++)
            x[i] -= augmented[i*(n+1)+j] * x[j];
        x[i] /= augmented[i*(n+1)+i];
    }

    free(augmented);
}

double* polynomial_fit(const double* y, int n, int m) {
    // Assumption: t is sequential, (t[0] = 1, t[1] = 2, t[n] = n + 1)

    // Allocate Vandermonde matrix A: m x n
    double* A = calloc(m * n, sizeof(double));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            A[i * n + j] = pow(i + 1, j);

    // Compute AtA = A^T * A: n x n
    double* AtA = calloc(n * n, sizeof(double));
    cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans,
                n, n, m, 1.0, A, n, A, n, 0.0, AtA, n);

    // Compute AtY = A^T * y: n x 1
    double* AtY = calloc(n, sizeof(double));
    cblas_dgemv(CblasRowMajor, CblasTrans,
                m, n, 1.0, A, n, y, 1, 0.0, AtY, 1);

    // Solve AtA * coeffs = AtY
    double* coeffs = malloc(n * sizeof(double));
    solve_linear_system(AtA, AtY, coeffs, n);

    // Cleanup
    free(A); free(AtA); free(AtY);

    return coeffs;
}

double* linear_combination(const double* x, const double* y, double a, double b, int length, int opr) { // (a * x + opr * b * y) / (a + opr * b)
    double* result = (double*)malloc(length * sizeof(double));
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    double _b = opr * b;
    const double inv_f_2 = 1 / (a + _b);

    // result = a * x
    cblas_dcopy(length, x, 1, result, 1);      // result = x
    cblas_dscal(length, inv_f_2 * a, result, 1);         // result = inv_f_2 * a * x

    // result = result + b * y
    cblas_daxpy(length, inv_f_2 * _b, y, 1, result, 1);   // result = inv_f_2 * a * x + inv_f_2 * b * y

    return result;
}

// Compute wide-lane phase combination in meters
MWPIRComb precompute_combinations(Vector* l1_phase, Vector* l2_phase, Vector* l1_psr, Vector* l2_psr) { // This should receive L1, L2, and NOT carrier phase in cycles.
    const size_t length = l1_phase->size;

    double* wlp = linear_combination(l1_phase->data, l2_phase->data, FREQ_L1, FREQ_L2, length, -1); // wide_lane_phase
    double* wlr = linear_combination(l1_psr->data, l2_psr->data, FREQ_L1, FREQ_L2, length, 1); // wide_lane_psr
    double* ip = vector_sum(l1_phase->data, l2_phase->data, -1, length); // iono phase
    double* ir = vector_sum(l2_psr->data, l1_psr->data, -1, length); // iono pseudorange

    double* b_delta = vector_sum(wlp, wlr, -1, length);

    free(wlp);
    free(wlr);

    cblas_dscal(length, LAMBDA_WL_INV, b_delta, 1);

    MWPIRComb result = { ip, ir, b_delta };

    return result;
}

Vector* widelane_slip_detection(MWPIRComb wlio_comb, size_t length) {
    double* b_delta = copy_array(wlio_comb.b_delta, length);

    Vector* ms = create_vector();

    // Calculate running mean.
    double running_mean = *b_delta; // first item
    double running_std2 = 0.25; // 0.5^2
    bool prev_outlier = false;
    
    for (int i = 1; i < length; i++) {
        double b_w = *(b_delta + i);

        if ((b_w - running_mean) * (b_w - running_mean) > 16 * running_std2) { // Outlier
            if(prev_outlier) { // any two consecutive outliers lying within 1 cycle
                push_back(ms, i - 1); // Store the mean of last epoch. This is the mean for last arc.

                running_mean = b_w; // Start new arc.
                running_std2 = 0.25;
                prev_outlier = false;
                continue;
            }

            prev_outlier = true;
            continue;
        }

        double inv_i = (1 / i);
        double b_w_prev = *(b_delta + i - 1);
        double b_dff = b_w - running_mean;

        running_mean = running_mean + inv_i * b_dff;
        running_std2 = running_std2 + inv_i * (b_dff * b_dff - running_std2); // Avoiding sqrt
        
        prev_outlier = false;
    }

    free(b_delta);

    return ms;
}

// Horner's method (https://en.wikipedia.org/wiki/Horner%27s_method)
double eval_poly(double* coeffs, int t, int order) {
    double result = coeffs[order - 1];
    for (int i = order - 2; i >= 0; i--) {
        result = result * t + coeffs[i];
    }
    return result;
}

Vector* ionospheric_splip_detection(MWPIRComb wlio_comb, size_t length) {
    double* ip = copy_array(wlio_comb.ip, length);
    double* ir = copy_array(wlio_comb.ir, length);

    const int degree = min(floor((length / 100 + 1)), 6); // in the original paper there is no floor() here, but it feels like there should be?
    const int n = degree + 1;

    double* coeffs = polynomial_fit(ir, n, length);

    Vector* slips = create_vector();

    for(int i = 1; i < length - 1; i++) { // Start from second observation
        static double Q_i_prev = 0.0;
        static double Q_i = 0.0;
        static double Q_i_next = 0.0;

        double L_i = *(ip + i);
        double L_i_prev = *(ip + i - 1);
        double L_i_next = *(ip + i + 1);

        if(i == 1) {
            Q_i_prev = eval_poly(coeffs, i, n);
            Q_i = eval_poly(coeffs, i + 1, n);
            Q_i_next = eval_poly(coeffs, i + 2, n);
        } else {
            Q_i_prev = Q_i;
            Q_i = Q_i_next;
            Q_i_next = eval_poly(coeffs, i + 2, n);
        }

        if((L_i - Q_i) - (L_i_prev - Q_i_prev) > 6 && (L_i_next - Q_i_next) - (L_i - Q_i) < 1) {
            push_back(slips, i - 1);
        }
    }

    free(ip); free(ir);

    return slips;
}

void* find_cycle_slips(double* l1_p, double* l2_p, double* l1_r, double* l2_r, size_t length) {
    Vector* l1_phase = create_vector();
    Vector* l2_phase = create_vector();
    Vector* l1_psr = create_vector();
    Vector* l2_psr = create_vector();

    copy_from_array(l1_phase, l1_p, length);
    copy_from_array(l2_phase, l2_p, length);
    copy_from_array(l1_psr, l1_r, length);
    copy_from_array(l2_psr, l2_r, length);

    clock_t start = clock();
    MWPIRComb obs = precompute_combinations(l1_phase, l2_phase, l1_psr, l2_psr);
    clock_t end = clock();

    printf("precompute_combinations took %.2f sec\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    Vector* widelane_slips = widelane_slip_detection(obs, length);
    end = clock();

    printf("widelane_slip_detection took %.2f sec\n", (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    Vector* iono_slips = ionospheric_splip_detection(obs, length);
    end = clock();

    printf("ionospheric_splip_detection took %.2f sec\n", (double)(end - start) / CLOCKS_PER_SEC);

    destroy(l1_phase); destroy(l2_phase); destroy(l1_psr); destroy(l2_psr); free(obs.b_delta); free(obs.ip); free(obs.ir);
}