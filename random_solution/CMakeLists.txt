cmake_minimum_required(VERSION 2.8)

file(GLOB HEADERS *.h)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -O0 -Wall -Werror -Wextra -Wno-unused-parameter -O2 -g")

add_executable(random_solution main.cpp ${HEADERS})
