from ctypes import *
import numpy as np
import time

np.random.seed(42)

class Slip(Structure):
    _fields_ = [("index", c_int),
                ("mean_bw", c_double),
                ("stdev", c_double),
                ("delta_N_w", c_int),
                ("nPoints", c_int),
                ("isPhaseConnected", c_char)]
    
class SlipVector(Structure):
    _fields_ = [("data", POINTER(Slip)),
                ("size", c_size_t),
                ("capacity", c_size_t)]

class WlData(Structure):
    _fields_ = [("arcs", POINTER(SlipVector)),
                 ("outliers", POINTER(c_int)),
                 ("outliers_length", c_int)]

class Results(Structure):
    _fields_ = [("arcs", POINTER(SlipVector)),
                ("widelane_arcs_length", c_size_t),
                ("ionospheric", POINTER(c_double)),
                ("ionospheric_slips_length", c_int),
                ("outliers", POINTER(c_int)),
                ("outliers_length", c_int)]
    
    def get_outliers(self):
        return np.array([self.outliers[j] for j in range(self.outliers_length)])
    
    def get_arcs(self):
        return [self.arcs[j] for j in range(self.widelane_arcs_length)]

    def __repr__(self):
        return f"<Results widelane_arcs_length={self.widelane_arcs_length}>"

lib = CDLL('./lib/libnvec.dll')

lib.find_cycle_slips.argtypes = [
    POINTER(c_double),
    POINTER(c_double),
    POINTER(c_double),
    POINTER(c_double),
    c_size_t
]
lib.find_cycle_slips.restype = Results

def cycle_slip_correction():
    n = 50000000
    a = np.random.rand(n) * 30
    b = np.random.rand(n) * 30
    c = np.random.rand(n) * 30
    d = np.random.rand(n) * 30

    start = time.time()
    slip_data = lib.find_cycle_slips(
        a.ctypes.data_as(POINTER(c_double)),
        b.ctypes.data_as(POINTER(c_double)),
        c.ctypes.data_as(POINTER(c_double)),
        d.ctypes.data_as(POINTER(c_double)),
        n
    )
    end = time.time()

    print(slip_data)

    print(f"Time elapsed: {end - start}")

cycle_slip_correction()