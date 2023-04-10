# Build the logging library
add_library(log STATIC)
target_sources(log PRIVATE logger.cc logger.h)
target_link_libraries(log PUBLIC Threads::Threads fmt::fmt base)
target_compile_options(log PUBLIC ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(log PUBLIC ${LONGLP_PROJECT_SRC_DIR})
