@echo off
setlocal
cd /d %~dp0

set INCLUDE=../openblas/include
set SRC=../src
set LIB=../lib

gcc -O3 -pg -I%INCLUDE% -I%SRC% profiling.c %SRC%\main.c %SRC%\nvector.c -L%LIB% -lopenblas -o profile.exe

echo Running profile...
profile.exe

echo Generating gprof report...
gprof profile.exe gmon.out > profile.txt
echo Done.