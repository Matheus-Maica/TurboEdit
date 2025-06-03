from ctypes import *
import numpy as np

C = 299792458.0
FREQ_L1 = 1575.42e6
FREQ_L2 = 1227.60e6

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

lib = CDLL('./lib/libnvec.dll')
lib.find_cycle_slips.argtypes = [POINTER(c_double), POINTER(c_double), POINTER(c_double), POINTER(c_double), c_size_t]
lib.find_cycle_slips.restype = Results

def correct_cycle_slips(
    l1_phase: np.ndarray,
    l2_phase: np.ndarray, 
    l1_psr: np.ndarray, 
    l2_psr: np.ndarray, 
):
    if len({len(l1_phase), len(l2_phase), len(l1_psr), len(l2_psr)}) != 1:
        raise Exception("Phase and pseudorange data must have the same length.")
    
    l1_phase = l1_phase * (-C / FREQ_L1)
    l2_phase = l2_phase * (-C / FREQ_L2)

    slip_data = lib.find_cycle_slips(
        l1_phase.ctypes.data_as(POINTER(c_double)),
        l2_phase.ctypes.data_as(POINTER(c_double)),
        l1_psr.ctypes.data_as(POINTER(c_double)),
        l2_psr.ctypes.data_as(POINTER(c_double)),
        len(l1_phase)
    )

    arcs = slip_data.get_arcs()
    outliers = slip_data.get_outliers()

    l1_phase[outliers] = np.nan
    l2_phase[outliers] = np.nan

    prev = 0
    for i in arcs:
        if not bool(i['isPhaseConnected'][0]):
            prev = i['index']
            continue

        l1_phase[prev-1:i['index']] += i['delta_N_w']
        l2_phase[prev-1:i['index']] += i['delta_N_w']

        prev = i['index']

    return l1_phase, l2_phase, l1_psr, l2_psr

def main():
    n = 50000
    a = np.random.rand(n) * 30
    b = np.random.rand(n) * 30
    c = np.random.rand(n) * 30
    d = np.random.rand(n) * 30

    correct_cycle_slips(a, b, c, d)

if __name__ == '__main__':
    main()