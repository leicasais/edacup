cmake_minimum_required(VERSION 3.25)   

project(move_sample)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB SAMPLE_SOURCE "*.cpp")

add_executable(move_sample ${SAMPLE_SOURCE})
target_link_libraries(move_sample PRIVATE nlohmann_json::nlohmann_json)
