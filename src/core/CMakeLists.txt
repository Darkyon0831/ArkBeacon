cmake_minimum_required(VERSION 3.24)

project(ArkBeacon_Core)

add_library(ArkBeacon_Core STATIC
    src/ParameterLoader.cpp
)

find_package(Python3 REQUIRED COMPONENTS Development)
find_package(OpenSSL REQUIRED)

target_include_directories(ArkBeacon_Core PRIVATE
    include
    ../backend/include
    ../../external/pybind11/include
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(ArkBeacon_Core PRIVATE
    OpenSSL::SSL 
    OpenSSL::Crypto
    ${Python3_LIBRARIES}
)

set_property(TARGET ArkBeacon_Core PROPERTY CXX_STANDARD 20)
target_compile_features(ArkBeacon_Core PRIVATE cxx_std_20)
