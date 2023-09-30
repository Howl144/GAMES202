file(GLOB_RECURSE srcs CONFIGURE_DEPENDS *.cpp)
set(name Pipeline-and-Shading)
add_executable(${name} ${srcs})
target_link_libraries(${name} ${OpenCV_LIBS})
target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(${name} PUBLIC -Wall)
add_custom_target(copy-files4 ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/models 
        ${CMAKE_BINARY_DIR}/models)
