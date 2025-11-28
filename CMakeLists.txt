cmake_minimum_required(VERSION 3.24)

project(ArkBeacon)

add_compile_definitions($<$<CONFIG:DEBUG>:DEBUG>)

if (WIN32)
    add_definitions(-D__WIN32__)
endif (WIN32)

add_subdirectory(src/backend)
add_subdirectory(src/utils)
add_subdirectory(src/core)
add_executable(ArkBeacon
    src/ArkBeacon.cpp
)

set_property(TARGET ArkBeacon PROPERTY CXX_STANDARD 20)

target_include_directories(ArkBeacon PRIVATE
    src/backend/include
    src/utils/include
    src/core/include
    external/tomlplusplus/include
)

target_link_libraries(ArkBeacon PRIVATE ArkBeacon_Backend)
target_link_libraries(ArkBeacon PRIVATE ArkBeacon_Utils)
target_link_libraries(ArkBeacon PRIVATE ArkBeacon_Core)

target_compile_definitions(ArkBeacon PRIVATE -DARKBEACON_DIR="${CMAKE_CURRENT_SOURCE_DIR}")