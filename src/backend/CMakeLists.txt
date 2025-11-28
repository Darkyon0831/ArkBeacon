cmake_minimum_required(VERSION 3.24)

project(ArkBeacon_Backend)

add_library(ArkBeacon_Backend INTERFACE)

find_package(Python3 REQUIRED COMPONENTS Development)

target_include_directories(ArkBeacon_Backend INTERFACE
    include
    ../../external/pybind11/include
    ../../external/websocketpp
    ../../external/json/single_include
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(ArkBeacon_Backend INTERFACE 
    ${Python3_LIBRARIES}
)

target_compile_features(ArkBeacon_Backend INTERFACE cxx_std_20)