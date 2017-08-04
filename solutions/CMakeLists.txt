cmake_minimum_required(VERSION 2.8)

file(GLOB HEADERS *.h)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -Wall -Werror -Wextra -Wunused-parameter -O2 -g")

add_library(common map.cpp launcher.cpp ${HEADERS})
add_executable(random_solution random_solution.cpp ${HEADERS})
target_link_libraries(random_solution common)
