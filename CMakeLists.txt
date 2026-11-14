cmake_minimum_required(VERSION 3.10)
project(udemy-rpc-from-scratch C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_subdirectory(serdes)
add_subdirectory(networking)
