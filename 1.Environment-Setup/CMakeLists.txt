file(GLOB_RECURSE srcs CONFIGURE_DEPENDS *.cpp *.h)
set(name Environment)
add_executable(${name} ${srcs})
target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})