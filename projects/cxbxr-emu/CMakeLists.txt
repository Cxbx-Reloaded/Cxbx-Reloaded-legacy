cmake_minimum_required (VERSION 3.12)
project(cxbxr-emu)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

# Suppress extra stuff from generated solution
set(CMAKE_SUPPRESS_REGENERATION true)

# Force exclude default libraries being included
#set(CMAKE_CXX_STANDARD_LIBRARIES "")

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

  # Use inline XXHash version
  XXH_INLINE_ALL
 )
 add_compile_options(
  /EHs
  /MP
  /GF
  /arch:SSE2
 )
endif()

add_compile_definitions(
 NOMINMAX
 CXBXR_EMU
 CXBXR_EMU_EXPORTS
)

file (GLOB RESOURCES
 "${CXBXR_ROOT_DIR}/CONTRIBUTORS"
 "${CXBXR_ROOT_DIR}/COPYING"
 "${CXBXR_ROOT_DIR}/README.md"
)

source_group(TREE ${CXBXR_ROOT_DIR}/import PREFIX import FILES
 ${CXBXR_HEADER_EMU_IMPORT}
)

source_group(TREE ${CXBXR_ROOT_DIR}/src PREFIX header FILES
 ${CXBXR_HEADER_GUIv1}
 ${CXBXR_HEADER_COMMON}
 ${CXBXR_HEADER_EMU}
 "${CXBXR_ROOT_DIR}/src/emulator/targetver.h"
)

source_group(TREE ${CXBXR_ROOT_DIR}/import PREFIX import FILES
 ${CXBXR_SOURCE_EMU_IMPORT}
)

source_group(TREE ${CXBXR_ROOT_DIR}/src PREFIX source FILES
 ${CXBXR_SOURCE_GUIv1}
 ${CXBXR_SOURCE_COMMON}
 ${CXBXR_SOURCE_EMU}
 "${CXBXR_ROOT_DIR}/src/emulator/cxbxr-emu.cpp"
 "${CXBXR_ROOT_DIR}/src/emulator/dllmain.cpp"
)

source_group(TREE ${CXBXR_ROOT_DIR} FILES ${RESOURCES})

add_library(${PROJECT_NAME} SHARED ${RESOURCES}
 ${CXBXR_HEADER_COMMON}
 ${CXBXR_HEADER_EMU_IMPORT}
 ${CXBXR_HEADER_EMU}
 "${CXBXR_ROOT_DIR}/src/emulator/targetver.h"
 ${CXBXR_SOURCE_COMMON}
 ${CXBXR_SOURCE_EMU_IMPORT}
 ${CXBXR_SOURCE_EMU}
 ${CXBXR_GIT_VERSION_H}
 "${CXBXR_ROOT_DIR}/src/emulator/cxbxr-emu.cpp"
 "${CXBXR_ROOT_DIR}/src/emulator/dllmain.cpp"
)

# Link and compile flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

 set_target_properties(${PROJECT_NAME} PROPERTIES
  LINK_FLAGS "
  /INCREMENTAL:NO \
  /LARGEADDRESSAWARE \
  /SAFESEH:NO \
  /STACK:65536,65536 \
  /NODEFAULTLIB:libcmt \
  /DELAYLOAD:wpcap.dll \
  "
  LINK_FLAGS_RELEASE "
  /LTCG \
  /DEBUG \
  "
 )

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
 wpcap
)

target_link_libraries(${PROJECT_NAME}
 PUBLIC libXbSymbolDatabase
 subhook
 libtomcrypt
 SDL2
 imgui
 libusb
 nv2a_vsh_emulator
 mio::mio_min_winapi

 ${WINS_LIB}
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
)

add_dependencies(${PROJECT_NAME} cxbxr-ldr misc-batch)

# Try to stop cmake from building hlsl files
# Which are all currently loaded at runtime only
set_source_files_properties(
  ${CXBXR_HEADER_HLSL}
  PROPERTIES
  HEADER_FILE_ONLY TRUE
  VS_TOOL_OVERRIDE "None"
)
