#########################################################
# CMake example to configure your project to use vk_sdk.
#########################################################
cmake_minimum_required(VERSION 3.16)
project(yolo_request VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV 4 REQUIRED)

# Set the directory to find the necessary libraries.
# Alternatively, you may specify these when you run `cmake` command instead. To do so, run
# `cmake -DCapnProto_DIR="..." -Dvk_sdk_DIR="..." "<source_directory>"` in your build directory.

set(CapnProto_DIR "/opt/vilota/lib/cmake/CapnProto")
set(vk_sdk_DIR "/opt/vilota/lib/cmake/vk_sdk")

find_package(vk_sdk CONFIG REQUIRED)

add_executable(yolo_request CustomYolo.cpp)
target_link_libraries(yolo_request PUBLIC CapnProto::capnp PRIVATE vilota::vk_sdk)

add_executable(image_yolo_viewer CustomImageYolo.cpp)
target_link_libraries(image_yolo_viewer PUBLIC CapnProto::capnp PRIVATE vilota::vk_sdk  ${OpenCV_LIBS})
