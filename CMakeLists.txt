cmake_minimum_required(VERSION 3.10)
project(OpenBLASExample C)

set(CMAKE_C_STANDARD 11)

# Use vcpkg integration
find_package(OpenBLAS REQUIRED)

add_executable(main main.c)
target_link_libraries(main PRIVATE OpenBLAS::OpenBLAS)