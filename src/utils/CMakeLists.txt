cmake_minimum_required(VERSION 3.24)

project(ArkBeacon_Utils)

add_library(ArkBeacon_Utils STATIC
    src/ArgumentProcessor.cpp
    src/InputHandler.cpp
    src/ConfigHandler.cpp
    src/Logger.cpp
)

target_include_directories(ArkBeacon_Utils PRIVATE
    include
    ../../external/tomlplusplus/include
)

set_property(TARGET ArkBeacon_Utils PROPERTY CXX_STANDARD 20)
target_compile_features(ArkBeacon_Utils PRIVATE cxx_std_20)