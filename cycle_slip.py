import numpy as np

C = 299792458.0
FREQ_L1 = 1575.42e6
FREQ_L2 = 1227.60e6

LAMBDA_WL = C / (FREQ_L1 - FREQ_L2)
LAMBDA_L1 = C / FREQ_L1
LAMBDA_L2 = C / FREQ_L2

FREQ1_RATIO = FREQ_L1 / (FREQ_L1 - FREQ_L2)

def precompute_combinations(L1: np.ndarray, L2: np.ndarray, P1: np.ndarray, P2: np.ndarray):
    wide_lane_phase = (L1 - L2) * LAMBDA_WL
    wide_lane_psr = P1 - P2 * FREQ1_RATIO

    iono_phase = (L1 - L2) * (LAMBDA_L1 - LAMBDA_L2)
    iono_psr = P2 - P1

    return wide_lane_phase, wide_lane_psr, iono_phase, iono_psr

def phase_connection(wide_lane_phase, wide_lane_psr, iono_phase, iono_psr):
    running_mean = wide_lane_phase[0] - wide_lane_psr[0]
    running_std = 0
    k = 1

    outliers = np.array([])
    arc_means = np.array([])
    last_residual = 0

    for i in range(1, len(wide_lane_phase)):
        r_diff = wide_lane_phase[i] - wide_lane_psr[i]
        residual = r_diff - running_mean

        old_mean = running_mean
        running_mean += residual / (k + 1)
        running_std = np.sqrt((k - 1) / k * (running_std ** 2) + (residual ** 2) / (k + 1))

        # outlier test
        if abs(residual) > 4 * running_std:           
            if len(outliers) and outliers[-1] == i - 1 and residual - last_residual < LAMBDA_WL: # Arc breakup
                running_mean = r_diff
                k = 1
                running_std = 0

                arc_means.append(old_mean)


            outliers.append(i)

        k += 1
        last_residual = residual