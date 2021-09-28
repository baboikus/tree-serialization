cmake_minimum_required(VERSION 3.10)

# set the project name
project(tree)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
add_compile_options(
  $<$<CONFIG:DEBUG>:-g>
  $<$<CONFIG:DEBUG>:-O0>
  $<$<CONFIG:RELEASE>:-O2>
)

# add the executable
add_executable(tree main.cpp)
