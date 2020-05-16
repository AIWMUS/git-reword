cmake_minimum_required(VERSION 3.5.1)

project(git-reword)

find_library(LIBGIT2_LIBRARY git2)

message(STATUS "libgit2: " ${LIBGIT2_LIBRARY})

#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_EXTENSIONS ON)

add_executable(git-reword src/main.cpp)

target_link_libraries(git-reword ${LIBGIT2_LIBRARY})
