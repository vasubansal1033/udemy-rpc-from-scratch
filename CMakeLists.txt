cmake_minimum_required(VERSION 3.10)
project(udemy-rpc-from-scratch C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# Shared serialization stream library
add_library(stream STATIC
  stream.c
  stream.h
)
target_include_directories(stream PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Demo apps: each has main() and links against stream
add_executable(primitive_ds_ser_deser primitive_ds_ser_deser.c)
target_link_libraries(primitive_ds_ser_deser PRIVATE stream)

add_executable(struct_ds_ser_deser struct_ds_ser_deser.c)
target_link_libraries(struct_ds_ser_deser PRIVATE stream)

add_executable(nested_struct_ds_ser_deser nested_struct_ds_ser_deser.c)
target_link_libraries(nested_struct_ds_ser_deser PRIVATE stream)

add_executable(nested_pointer_struct_ds_ser_deser nested_pointer_struct_ds_ser_deser.c)
target_link_libraries(nested_pointer_struct_ds_ser_deser PRIVATE stream)
