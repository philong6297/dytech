# Build base library
add_library(base STATIC)
target_sources(base PRIVATE pointers.h pointers.cc)
target_link_libraries(base PUBLIC Microsoft.GSL::GSL)
target_compile_options(base PUBLIC ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(base PUBLIC ${LONGLP_PROJECT_SRC_DIR})
