project(benchmark_succinct)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

add_executable(rank_sel rank_sel.cpp)
target_link_libraries(rank_sel PRIVATE sdsl)