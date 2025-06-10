from ctypes import *
import numpy as np
import os

np.random.seed(42)

C = 299792458.0

def match_sampling_rate(l1, l2, p1, p2, F1, F2, method='mean'):
    def downsample(arr, factor, agg='mean'):
        arr = arr[:len(arr) - len(arr) % factor]  # trim excess if not divisible
        reshaped = arr.reshape(-1, factor)
        if agg == 'mean':
            return reshaped.mean(axis=1)
        elif agg == 'first':
            return reshaped[:, 0]
        elif callable(agg):
            return agg(reshaped, axis=1)
        else:
            raise ValueError("Unsupported method: choose 'mean', 'first', or provide a function")

    if F1 == F2:
        return l1, l2, p1, p2

    elif F1 > F2:
        factor = F1 // F2
        if F1 % F2 != 0:
            raise ValueError("F1 must be an integer multiple of F2 to downsample cleanly.")
        l1_down = downsample(l1, factor, method)
        l2_down = downsample(l2, factor, method)
        return l1_down, l2_down, p1, p2

    else:
        factor = F2 // F1
        if F2 % F1 != 0:
            raise ValueError("F2 must be an integer multiple of F1 to downsample cleanly.")
        p1_down = downsample(p1, factor, method)
        p2_down = downsample(p2, factor, method)
        return l1, l2, p1_down, p2_down

def remove_gaps_from_original_l_series(l1, l2, F_high, F_low, gap_mask_1hz):
    """
    Masks (with NaN) the regions in l1 and l2 corresponding to gaps in the 1 Hz data.
    The original length is preserved.

    Parameters:
        l1, l2          : original high-frequency arrays (e.g., 100 Hz)
        F_high          : original sampling rate (e.g., 100)
        F_low           : low sampling rate used for gap detection (e.g., 1)
        gap_mask_1hz    : boolean array (True = valid, False = gap) of length (duration in seconds)

    Returns:
        l1_masked, l2_masked : same shape as l1 and l2, with NaNs in gap regions
    """
    if F_high % F_low != 0:
        raise ValueError("F_high must be an integer multiple of F_low.")

    factor = F_high // F_low
    expected_len = len(gap_mask_1hz) * factor

    if len(l1) < expected_len:
        raise ValueError("Original high-frequency signals are shorter than expected from gap_mask.")

    # Repeat each mask value for its corresponding high-frequency chunk
    high_freq_mask = np.repeat(gap_mask_1hz, factor)

    # Make sure the mask matches the signal length exactly
    high_freq_mask = np.pad(high_freq_mask, (0, len(l1) - len(high_freq_mask)), constant_values=True)

    l1_masked = l1.copy()
    l2_masked = l2.copy()
    l1_masked[~high_freq_mask] = np.nan
    l2_masked[~high_freq_mask] = np.nan

    return l1_masked, l2_masked

class Slip(Structure):
    _fields_ = [("index", c_int), ("mean_bw", c_double), ("stdev", c_double), ("delta_N_w", c_int), ("nPoints", c_int), ("isPhaseConnected", c_char)]
    
class SlipVector(Structure):
    _fields_ = [("data", POINTER(Slip)), ("size", c_size_t), ("capacity", c_size_t)]

class Results(Structure):
    _fields_ = [("arcs", POINTER(SlipVector)), ("widelane_arcs_length", c_size_t), ("ionospheric", POINTER(c_double)), ("ionospheric_slips_length", c_int), ("outliers", POINTER(c_int)), ("outliers_length", c_int)]
    
    def get_outliers(self):
        return np.array([self.outliers[j] for j in range(self.outliers_length)])
    
    def get_arcs(self):
        def get_slip(slip):
            return { "index": slip.index, "mean_bw": slip.mean_bw, "stdev": slip.stdev, "delta_N_w": slip.delta_N_w, "nPoints": slip.nPoints, "isPhaseConnected": slip.isPhaseConnected }

        return [get_slip(self.arcs.contents.data[j]) for j in range(self.widelane_arcs_length)]


def get_dll_path(dll_name="libnvec.dll"):
    base_path = os.path.dirname(__file__)
    dll_path = os.path.join(base_path, "./lib", dll_name)
    return dll_path

lib = CDLL(get_dll_path())
lib.find_cycle_slips.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_size_t]
lib.find_cycle_slips.restype = Results

def correct_cycle_slips(
    l1_phase: np.ndarray,
    l2_phase: np.ndarray,
    l1_psr: np.ndarray,
    l2_psr: np.ndarray,
    FREQ_L1: float = 1575.42e6,
    FREQ_L2: float = 1227.60e6,
    **kwargs
):
    l_rate = kwargs.get('l_rate', 1)
    p_rate = kwargs.get('p_rate', 1)

    l1p, l2p, l1r, l2r = match_sampling_rate(l1_phase, l2_phase, l1_psr, l2_psr, l_rate, p_rate) # Downsample

    l1p *= (-C / FREQ_L1)
    l2p *= (-C / FREQ_L2)

    if len({len(l1p), len(l2p), len(l1r), len(l2r)}) != 1:
        raise Exception("Failed to match sampling rate.")

    slip_data = lib.find_cycle_slips(l1p.ctypes.data_as(POINTER(c_double)), l2p.ctypes.data_as(POINTER(c_double)), l1r.ctypes.data_as(POINTER(c_double)), l2r.ctypes.data_as(POINTER(c_double)), len(l1p))

    arcs = slip_data.get_arcs()
    outliers = slip_data.get_outliers()

    l1p[outliers] = np.nan
    l2p[outliers] = np.nan

    prev = 0
    for i in arcs:
        if bool(i['isPhaseConnected'][0]):
            prev = i['index']
            continue

        l1p[prev-1:i['index']] += i['delta_N_w']
        l2p[prev-1:i['index']] += i['delta_N_w']

        prev = i['index']

    l1p *= (-FREQ_L1 / C)
    l2p *= (-FREQ_L2 / C)

    return remove_gaps_from_original_l_series(l1_phase, l2_phase, l_rate, p_rate, ~np.isnan(l2p))