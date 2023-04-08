add_library(chromium STATIC)
target_sources(chromium PRIVATE base/no_destructor.h compile.cc)
target_compile_options(chromium PRIVATE ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(chromium PRIVATE ${LONGLP_PROJECT_SRC_DIR}/third_party/chromium)
