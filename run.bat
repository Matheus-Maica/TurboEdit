@echo off
set INCLUDE_PATH=.\openblas\include
set LIB_PATH=.\lib

echo Compiling...
REM gcc -O3 -march=native -funroll-loops -fopenmp -fPIC -I%INCLUDE_PATH% -c src\main.c src\dvector.c src\svector.c src\ivector.c --- -march=native causes unexpected crashes
gcc -O3 -funroll-loops -fopenmp -fPIC -I%INCLUDE_PATH% -c src\main.c src\dvector.c src\svector.c src\ivector.c
gcc -shared -o lib\libnvec.dll main.o dvector.o svector.o ivector.o -L%LIB_PATH% -lopenblas

del *.o
echo Done.

set OMP_NUM_THREADS=20
python python/cycle_slip.py