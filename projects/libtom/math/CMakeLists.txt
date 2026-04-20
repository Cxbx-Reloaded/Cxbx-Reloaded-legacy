cmake_minimum_required (VERSION 3.12)
project(libtommath)

# Suppress extra stuff from generated solution
set(CMAKE_SUPPRESS_REGENERATION true)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
 add_compile_definitions(
 _CRT_SECURE_NO_WARNINGS
 _CRT_NONSTDC_NO_DEPRECATE
 )
 # Suppress warnings in external code
 add_compile_options(
 /wd4146 # unary minus on unsigned type
 )
 # s_mp_rand_platform.c relies on dead code elimination to avoid linking
 # Unix-only random sources (arc4random, getrandom, urandom) on Windows.
 # MSVC /Od (Debug) disables all optimization including DCE, causing LNK2019.
 # Strip /RTC1 (incompatible with /O1) and enable minimal optimization for
 # just this file so DCE works in Debug builds.
 string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
endif()

file (GLOB HEADERS
 "${CXBXR_ROOT_DIR}/import/libtommath/*.h"
)

file (GLOB SOURCES
 "${CXBXR_ROOT_DIR}/import/libtommath/*.c"
)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
 set_source_files_properties(
  "${CXBXR_ROOT_DIR}/import/libtommath/s_mp_rand_platform.c"
  PROPERTIES COMPILE_FLAGS "$<$<CONFIG:Debug>:/O1>"
 )
endif()

source_group(TREE ${CXBXR_ROOT_DIR}/import/libtommath PREFIX header FILES ${HEADERS})

source_group(TREE ${CXBXR_ROOT_DIR}/import/libtommath PREFIX source FILES ${SOURCES})

add_library(libtommath ${HEADERS} ${SOURCES})

target_include_directories(libtommath
 PUBLIC "${CXBXR_ROOT_DIR}/import/libtommath"
)
