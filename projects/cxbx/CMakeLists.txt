cmake_minimum_required (VERSION 3.12)
project(cxbx)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

# Suppress extra stuff from generated solution
set(CMAKE_SUPPRESS_REGENERATION true)

include_directories(
 "${CXBXR_ROOT_DIR}/src"
 "${CXBXR_ROOT_DIR}/src/common"
 "${CXBXR_ROOT_DIR}/src/common/Win32"
 "${CXBXR_ROOT_DIR}/import/OpenXDK/include"
 "${CXBXR_ROOT_DIR}/import/DirectX9/include"
 "${CXBXR_ROOT_DIR}/import/distorm/include"
 "${CXBXR_ROOT_DIR}/import/glew-2.0.0/include"
 "${CXBXR_ROOT_DIR}/import/libusb/libusb"
 "${CXBXR_ROOT_DIR}/import/simpleini"
 "${CXBXR_ROOT_DIR}/import/winpcap/Include"
 "${CXBXR_ROOT_DIR}/import/xxHash"
)

link_directories(
 "${CXBXR_ROOT_DIR}/import/distorm/lib/Win32"
 "${CXBXR_ROOT_DIR}/import/glew-2.0.0/lib/Release/Win32"
 "${CXBXR_ROOT_DIR}/import/DirectX9/lib"
 "${CXBXR_ROOT_DIR}/import/winpcap/Lib"
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
 add_compile_definitions(
  _CRT_SECURE_NO_WARNINGS
  # Windows 7 minimum requirement
  _WIN32_WINNT=0x0601
  LTM_DESC
  USE_LTM
  LTC_NO_TEST
  LTC_NO_CIPHERS
  LTC_NO_HASHES
  LTC_NO_MACS
  LTC_NO_PRNGS
  LTC_HMAC
  LTC_HASH_HELPERS
  LTC_NO_MISC
  LTC_NO_PROTOTYPES

  # Enable Chihiro work
  CHIHIRO_WORK
 )

 # Reference: https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically
 add_compile_options(
  # Catch synchronous (C++) exceptions only
  # plus assume extern "C" may throw exception
  /EHs
  # Compile multiple source files by using multiple processes.
  /MP
  # Enable string pooling (reduce duplicate strings)
  /GF
  # Enable SSE2 code generation
  /arch:SSE2
 )
endif()

add_compile_definitions(NOMINMAX

# Use inline XXHash version
XXH_INLINE_ALL
)

file (GLOB RESOURCES

 "${CXBXR_ROOT_DIR}/CONTRIBUTORS"
 "${CXBXR_ROOT_DIR}/COPYING"
 "${CXBXR_ROOT_DIR}/README.md"
 "${CXBXR_ROOT_DIR}/src/gui/resource/.editorconfig"
 "${CXBXR_ROOT_DIR}/src/gui/resource/Cxbx.rc"
 "${CXBXR_ROOT_DIR}/src/gui/resource/Cxbx-R.ico"
 "${CXBXR_ROOT_DIR}/src/gui/resource/Logo.bmp"
 "${CXBXR_ROOT_DIR}/src/gui/resource/Logo-License-CC4.bmp"
 "${CXBXR_ROOT_DIR}/src/gui/resource/ResCxbx.h"
 "${CXBXR_ROOT_DIR}/src/.editorconfig"
)

source_group(TREE ${CXBXR_ROOT_DIR}/src PREFIX header FILES
 ${CXBXR_HEADER_GUIv1}
 ${CXBXR_HEADER_COMMON}
)

source_group(TREE ${CXBXR_ROOT_DIR}/import PREFIX import FILES
 ${CXBXR_SOURCE_EMU_IMPORT}
)

source_group(TREE ${CXBXR_ROOT_DIR}/src PREFIX source FILES
 ${CXBXR_SOURCE_GUIv1}
 ${CXBXR_SOURCE_COMMON}
)

source_group(TREE ${CXBXR_ROOT_DIR} FILES ${RESOURCES})

add_executable(${PROJECT_NAME} WIN32 ${RESOURCES}
 ${CXBXR_HEADER_GUIv1}
 ${CXBXR_HEADER_COMMON}
 ${CXBXR_SOURCE_GUIv1}
 ${CXBXR_SOURCE_COMMON}
 ${CXBXR_GIT_VERSION_H}
)

# Link and compile flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

 # Reference: https://docs.microsoft.com/en-us/cpp/build/reference/linker-options
 set_target_properties(${PROJECT_NAME} PROPERTIES
  LINK_FLAGS "
  /NODEFAULTLIB:libcmt \
  "
  LINK_FLAGS_RELEASE "
  /LTCG \
  /DEBUG \
  "
 )

 # Reference: https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically
 # /Zi   = create a PDB file without affecting optimization
 # /Ob3  = Controls inline expansion of functions.
 # /Oi   = Generate intrinsic functions
 # /Ot   = In favor of using fast code than small code
 # /GL   = Whole program optimization
 # /GS-  = Remove buffer security check
 # /Gy   = Enable function-level linking
 # /Qpar = Enable automatic parallelize loops in the code

 # Set optimization options for release build
 # Replace CMake's default /Ob2 with /Ob3 for aggressive inlining
 string(REPLACE "/Ob2" "/Ob3" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
 set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} \
  /Zi \
  /Oi \
  /Ot \
  /GL \
  \
  /GS- \
  /Gy \
  /Qpar \
  "
 )

 # disable optimization for CxbxKrnl.cpp file
 set_source_files_properties(
  ${CXBXR_KRNL_CPP} PROPERTIES COMPILE_FLAGS "/Od /GL-"
 )
endif()

# Windows libraries
set(WINS_LIB
 legacy_stdio_definitions
 d3d9
 d3dcompiler
 delayimp
 dinput8
 dxguid
 odbc32
 odbccp32
 Shlwapi
 dxerr9
 ws2_32
 dsound
 winmm
 ddraw
 d3dx9
 dbghelp
 comctl32
 XINPUT9_1_0
 Iphlpapi
 Dwmapi
)

target_link_libraries(${PROJECT_NAME}
 PUBLIC libXbSymbolDatabase
 subhook
 libtomcrypt
 SDL2
 imgui
 libusb
 mio::mio_min_winapi

 ${WINS_LIB}
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
)

if(${CMAKE_GENERATOR} MATCHES "Visual Studio ([^9]|[9][0-9])" AND BUILD_CXBXR_DEBUGGER)
  add_dependencies(${PROJECT_NAME} cxbxr-debugger)
endif()

add_dependencies(${PROJECT_NAME} cxbxr-ldr cxbxr-emu misc-batch)

# Try to stop cmake from building hlsl files
# Which are all currently loaded at runtime only
set_source_files_properties(
  ${CXBXR_HEADER_HLSL}
  PROPERTIES
  HEADER_FILE_ONLY TRUE
  VS_TOOL_OVERRIDE "None"
)
