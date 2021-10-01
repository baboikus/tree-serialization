cmake_minimum_required(VERSION 3.10)

# set the project name
project(tree)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
add_compile_options(
  $<$<CONFIG:RELEASE>:-O2>

  $<$<CONFIG:VALGRIND>:-O0>
  $<$<CONFIG:${VALGRIND}>:-g>

  $<$<CONFIG:${GPROF}>:-pg>
)

add_link_options(
  $<$<BOOL:${GPROF}>:-pg>
)


# add the executable
add_executable(tree main.cpp)
