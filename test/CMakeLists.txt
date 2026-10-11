cmake_minimum_required(VERSION 3.22)
project(ref VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ref
    ref_test.cpp
    catch_amalgamated.cpp
)

target_include_directories(ref PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../IVX/src"
)

# ref_test.cpp asserts intrusive-implementation behaviour (count layout,
# non-copyable RefCounted, ref_count()), so this target pins the intrusive mode.
target_compile_definitions(ref PRIVATE IVX_INTRUSIVE_REF)