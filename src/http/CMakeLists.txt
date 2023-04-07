# Build the turtle http library
add_library(http STATIC)
target_sources(
  http
  PRIVATE header.h
          http_utils.h
          request.h
          response.h
          header.cpp
          http_utils.cpp
          request.cpp
          response.cpp
          cgier.h
          cgier.cpp
)
target_link_libraries(http PUBLIC log)
target_compile_options(http PUBLIC ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(http PUBLIC ${LONGLP_PROJECT_SRC_DIR})
