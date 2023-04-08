# Build base library
add_library(base STATIC)
target_sources(
  base
  PRIVATE pointers.h
          pointers.cc
          utils.cc
          utils.h
          no_destructor.h
          no_destructor.cc
          chrono.cc
          chrono.h
          macros.h
)
target_link_libraries(base PRIVATE Microsoft.GSL::GSL chromium)
target_compile_options(base PUBLIC ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(base PUBLIC ${LONGLP_PROJECT_SRC_DIR})
