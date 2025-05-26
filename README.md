# TurboEdit Algorithm Implementation

This project provides a C implementation of the [TurboEdit algorithm](https://agupubs.onlinelibrary.wiley.com/doi/10.1029/GL017i003p00199) for detecting potential GPS cycle slips, along with a Python wrapper for identifying and correcting those slips when possible.

## 📖 Reference

- Original paper: [Blewitt, G. (1990)](https://agupubs.onlinelibrary.wiley.com/doi/10.1029/GL017i003p00199) — *An Automatic Editing Algorithm for GPS Data* (Geophysical Research Letters, Vol. 17, No. 3)

## 🧱 Requirements

- [MinGW-w64](https://www.mingw-w64.org/) (for building on Windows)
- [Python 3.x](https://www.python.org/)
- [OpenBLAS](https://github.com/xianyi/OpenBLAS) (included or linked for CBLAS support)

## 🛠️ Build Instructions

1. Clone this repository.
2. Make sure `libopenblas.dll` and `cblas.h` are available in the `lib/` and `include/` folders respectively.
3. From the project root, run:

   ```bat
   run.bat
   ```

   This will build the shared library and run the example script.

---

## ⚙️ C Function Overview

### `main.c` → `find_cycle_slips`

This is the core C function. It is responsible for detecting potential cycle slips in GPS phase and pseudorange data.

#### Function Signature
```c
void find_cycle_slips(
    double* phase_L1,      // e.g., L1 carrier phase
    double* phase_L2,      // e.g., L2 carrier phase
    double* range_L1,      // e.g., L1 pseudorange
    double* range_L2,      // e.g., L2 pseudorange
    size_t n_samples       // number of data points
);
```

- It is assumed that all arrays have the same length.
- The function returns a list containing the indices of all detected slips.

#### Example Usage
```c
int N = 5000;
double phase_L1[N] = {1.0, 2.0, 3.0, ...};
double phase_L2[N] = {5.0, 3.0, 7.0, ...};
double range_L1[N] = {2.0, 5.0, 9.0, ...};
double range_L2[N] = {8.0, 3.0, 6.0, ...};

find_cycle_slips(phase_L1, phase_L2, range_L1, range_L2, N);
```
