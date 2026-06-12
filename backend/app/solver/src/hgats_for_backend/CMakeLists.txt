cmake_minimum_required(VERSION 3.16)
project(HGATS CXX)

# ── Standard ─────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Default build type ───────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ── Sources ──────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/problem.cpp
    src/individual.cpp
    src/mem.cpp
    src/local_search.cpp
    src/gsga.cpp
    src/tabu_search.cpp
    src/hgats.cpp
)

# ── Executable ───────────────────────────────────────────
add_executable(HGATS ${SOURCES})

target_include_directories(HGATS PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# ── Compiler optimizations (safe way) ────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(HGATS PRIVATE -O3)
endif()

# ── Copy data folder automatically ───────────────────────
if(EXISTS "${CMAKE_SOURCE_DIR}/data")
    file(COPY "${CMAKE_SOURCE_DIR}/data"
         DESTINATION "${CMAKE_BINARY_DIR}")
    message(STATUS "Data folder copied to: ${CMAKE_BINARY_DIR}/data")
endif()