# Build the echo server
add_executable(echo_server)
target_sources(echo_server PRIVATE echo_server.cpp)
target_link_libraries(echo_server PRIVATE core)
target_compile_options(echo_server PRIVATE ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(echo_server PRIVATE ${LONGLP_PROJECT_SRC_DIR})

# Build the echo client
add_executable(echo_client)
target_sources(echo_client PRIVATE echo_client.cpp)
target_link_libraries(echo_client PRIVATE core)
target_compile_options(echo_client PRIVATE ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(echo_client PRIVATE ${LONGLP_PROJECT_SRC_DIR})
