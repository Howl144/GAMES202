file(GLOB_RECURSE srcs CONFIGURE_DEPENDS *.cpp *.h *.hpp)
set(name Path-Tracing)
add_executable(${name} ${srcs})
target_link_libraries(${name} PUBLIC ${OpenCV_LIBS})
target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(copy-files8 ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/models 
        ${CMAKE_BINARY_DIR}/models)