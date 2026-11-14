add_library(network_common STATIC
  network_common.c
  network_common.h
)
target_include_directories(network_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(client client.c)
target_link_libraries(client PRIVATE network_common)

add_executable(server server.c)
target_link_libraries(server PRIVATE network_common)
