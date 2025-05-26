import ctypes
import numpy as np
import time

lib = ctypes.CDLL('./lib/libnvec.dll')

lib.find_cycle_slips.argtypes = [
    ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double),
    ctypes.c_size_t
]
lib.find_cycle_slips.restype = None

def cycle_slip_correction():
    n = 50000000
    a = np.random.rand(n)
    b = np.random.rand(n)
    c = np.random.rand(n)
    d = np.random.rand(n)

    start = time.time()
    lib.find_cycle_slips(
        a.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
        b.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
        c.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
        d.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
        n
    )
    end = time.time()

    print(f"Time elapsed: {end - start}")

cycle_slip_correction()