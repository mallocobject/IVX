cmake_minimum_required(VERSION 3.22)
project(test VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Needs a real GL context, so it is a program rather than a unit test: it tone maps a known
# HDR radiance, then checks the encoded pixels, the exposure curve, the picking ID and that the
# pass restores the pipeline state it found.
add_executable(hdr_smoke hdr_smoke.cpp)
target_link_libraries(hdr_smoke PRIVATE IVX)

# ref_test.cpp, pre_process_test.cpp and scoped_timer_test.cpp predate the current headers
# (ivx/core/scoped_timer.h no longer exists), so they are kept for reference but not built.
