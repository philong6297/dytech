# Build the http server
add_executable(http_server)
target_sources(http_server PRIVATE http_server.cc)
target_link_libraries(http_server PRIVATE core http)
target_compile_options(http_server PRIVATE ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(http_server PRIVATE ${LONGLP_PROJECT_SRC_DIR})
add_dependencies(http_server copy_data)

add_custom_target(
  copy_data ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory ./pages ${CMAKE_BINARY_DIR}/demo/http-server/pages
  VERBATIM
  WORKING_DIRECTORY ${LONGLP_PROJECT_DIR}/demo/http-server/
  USES_TERMINAL
  COMMENT
    "copy ${LONGLP_PROJECT_DIR}/demo/http-server/pages to ${CMAKE_BINARY_DIR}/demo/http-server/pages"
)
