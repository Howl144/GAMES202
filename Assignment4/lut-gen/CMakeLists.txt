cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
project(lut-gen)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext)

add_executable(lut-Emu-MC Emu_MC.cpp)
add_executable(lut-Eavg-MC Eavg_MC.cpp)
add_executable(lut-Emu-IS Emu_IS.cpp)
add_executable(lut-Eavg-IS Eavg_IS.cpp)
