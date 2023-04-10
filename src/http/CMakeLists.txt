# Build the http library
add_library(http STATIC)
target_sources(
  http
  PRIVATE header.h
          http_utils.h
          request.h
          response.h
          header.cc
          http_utils.cc
          request.cc
          response.cc
          cgi_runner.h
          cgi_runner.cc
          constants.h
          constants.cc
)
target_link_libraries(http PUBLIC log core)
target_compile_options(http PUBLIC ${LONGLP_DESIRED_COMPILE_OPTIONS} -Wno-unsafe-buffer-usage)
target_include_directories(http PUBLIC ${LONGLP_PROJECT_SRC_DIR})
