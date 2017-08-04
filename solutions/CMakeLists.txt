cmake_minimum_required(VERSION 2.8)

file(GLOB HEADERS *.h)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -Wall -Werror -Wextra -Wunused-parameter -O2 -g")

add_library(common map.cpp launcher.cpp draw.cpp ${HEADERS})
add_executable(random_solution random_solution.cpp ${HEADERS})
target_link_libraries(random_solution common)
add_executable(greed_solution greed_solution.cpp ${HEADERS})
target_link_libraries(greed_solution common)
add_executable(flow_st_solution flow_st_solution.cpp ${HEADERS})
target_link_libraries(flow_st_solution common)
add_executable(bruteforce_solution bruteforce_solution.cpp ${HEADERS})
target_link_libraries(bruteforce_solution common)
add_executable(greed_st_solution greed_st_solution.cpp ${HEADERS})
target_link_libraries(greed_st_solution common)
