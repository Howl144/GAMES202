cmake_minimum_required(VERSION 3.17)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 17)

project(games101)

#加载opencv模块
set(OpenCV_DIR D:/opencv/sources/build)
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
    message(-- add_include : "${OpenCV_INCLUDE_DIRS}")
endif()

#添加头文件
include_directories(${CMAKE_SOURCE_DIR}/include)
message(-- add_include : "${CMAKE_SOURCE_DIR}/include")
#加载子目录下的cmake文件
add_subdirectory(freetype)
add_subdirectory(1.Environment-Setup)
add_subdirectory(2.Simple-Rasterizer)
add_subdirectory(3.Z-buffering)
add_subdirectory(4.Pipeline-and-Shading)
add_subdirectory(5.Bezier-Curves)
add_subdirectory(6.The-light-intersects-the-triangle)
add_subdirectory(7.Whitted-Style-Ray-Tracing-and-Bounding-Volume-Hierarchy)
add_subdirectory(8.Path-Tracing)
add_subdirectory(9.Spring-mass)