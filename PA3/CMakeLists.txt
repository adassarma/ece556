cmake_minimum_required(VERSION 3.22)

project(Steiner VERSION 1.2 LANGUAGES CXX)

MESSAGE(STATUS "ROOT: " ${CMAKE_ROOT})
MESSAGE(STATUS "SOURCE_DIR: " ${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-std=c++20 -O2")
set (SOURCES ${PROJECT_SOURCE_DIR}/src/main.cpp ${PROJECT_SOURCE_DIR}/src/parser.cpp ${PROJECT_SOURCE_DIR}/src/steiner.hpp)

add_executable(steiner ${SOURCES})

target_include_directories(steiner PRIVATE ${PROJECT_SOURCE_DIR}/src) 
target_include_directories(steiner PRIVATE ${PROJECT_SOURCE_DIR}/ThirdParty/PEGTL/include)