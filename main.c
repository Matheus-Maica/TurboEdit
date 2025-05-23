#include <stdio.h>
#include <cblas.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "nvector.h"

#define C 299792458.0 // Speed of light
#define FREQ_L1 1575.42e6 // L1 Frequency in Hz
#define FREQ_L2 1227.60e6 // L2 Frequency in Hz

// Wide-lane wavelength
const double LAMBDA_WL = C / (FREQ_L1 - FREQ_L2);

// L1 and L2 wavelengths
const double LAMBDA_L1 = C / FREQ_L1;
const double LAMBDA_L2 = C / FREQ_L2;
const double FREQ1_RATIO = FREQ_L1 / (FREQ_L1 - FREQ_L2);

typedef struct {
    double* wlp;
    double* wlr;
    double* ip;
    double* ir;
} MWPIRComb;

double* vector_sum(const Vector* a, const Vector* b, unsigned char coeff) {
    const size_t length = a->size;
    // coeff = -1 => a - b
    // coeff = 1 => a + b
    double* result = (double*)malloc(length * sizeof(double));
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // result = a (copy a into result)
    cblas_dcopy(length, a->data, 1, result, 1);

    // result = result - b → result = a - b
    // This is done by result += -1.0 * b
    cblas_daxpy(length, coeff, b->data, 1, result, 1);

    return result;
}

double* vector_mult(const Vector* a, double alpha) {
    const size_t length = a->size;
    double* result = (double*)malloc(length * sizeof(double));
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    cblas_scopy(length, a, 1, result, 1);
    cblas_dscal(length, alpha, a->data, 1);

    return result;
}

double* linear_combination(const double* x, const double* y, double a, double b, int length, int opr) { // (a * x + opr * b * y) / (a + opr * b)
    double* result = (double*)malloc(length * sizeof(double));
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    double _b = opr * b;
    const inv_f_2 = 1 / (a + _b);

    // result = a * x
    cblas_dcopy(length, x, 1, result, 1);      // result = x
    cblas_dscal(length, a, result, 1);         // result = a * x

    // result = result + b * y
    cblas_daxpy(length, _b, y, 1, result, 1);   // result = a * x + b * y

    // result = result * inv_f_2
    cblas_dscal(length, inv_f_2, result, 1);         // result = a * x

    return result;
}

// Compute wide-lane phase combination in meters
MWPIRComb precompute_combinations(Vector* l1_phase, Vector* l2_phase, Vector* l1_psr, Vector* l2_psr) { // This should receive L1, L2, and NOT carrier phase in cycles.
    const size_t length = l1_phase->size;

    double* wlp = linear_combination(l1_phase->data, l2_phase->data, FREQ_L1, FREQ_L2, length, -1); // wide_lane_phase
    double* wlr = linear_combination(l1_psr->data, l2_psr->data, FREQ_L1, FREQ_L2, length, 1); // wide_lane_psr
    double* ip = vector_sum(l1_phase, l2_phase, -1); // iono phase
    double* ir = vector_sum(l2_psr, l1_psr, -1); // iono pseudorange

    MWPIRComb result = { wlp, wlr, ip, ir };

    return result;
}

Vector* cycle_slip_detection(Vector* l1_phase, Vector* l2_phase, Vector* l1_psr, Vector* l2_psr) {
    int k = 1;
    double running_rms = 0.0; // Actually, sigma squared.
    double running_mean = compute_wide_lane_phase(*l1_phase->data, *l2_phase->data); // Pass first element of vectors.
    Vector* outliers = create_vector();


    int rl1_size = size(l1_phase);
    int i, residual;
    bool was_last_epoch_outlier = false;

    for(i = 1; i < rl1_size; i++) {
        double wide_lane_phase = compute_wide_lane_phase(*(l1_phase->data + i), *(l2_phase->data + i));
        double wide_lane_psr = compute_wide_lane_psr(*(l1_psr->data + i), *(l2_psr->data + i));
        double old_mean = running_mean;
        residual = wide_lane_phase - wide_lane_psr - running_mean;

        running_mean = (k * running_mean + wide_lane_phase - wide_lane_psr) / (k + 1);
        running_rms = sqrt((k - 1) / k * pow(running_rms, 2) + pow(wide_lane_phase - wide_lane_psr - running_mean, 2) / (k + 1)); // Again, updating sigma squared

        if(abs(residual) > 4 * running_rms) {
            push_back(outliers, i); // Mark the epoch as an outlier.
            was_last_epoch_outlier = true;
        }

        if(was_last_epoch_outlier) {
            k = 1; // Start new phase-connected arc
        } else {
            k += 1;
            was_last_epoch_outlier = false;
        }
    }

    return outliers;
}

int main() {
    // Example observation (just a dummy value)
    GpsObservation obs = {
        .time = 100000.0,
        .L1_phase = 123456.789,
        .L2_phase = 123450.123
    };

    // double wl_phase = compute_wide_lane_phase(obs);

    printf("Epoch time: %.2f sec\n", obs.time);

    return 0;
}