cmake_minimum_required(VERSION 3.10.2)

project("opencv_plugin")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(libopencv_java4 SHARED IMPORTED)

set_target_properties(libopencv_java4 PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libopencv_java4.so)

add_library(
        native-lib
        SHARED
        native-lib.cpp)

target_link_libraries(
        native-lib
        libopencv_java4
        -landroid
        -llog)
