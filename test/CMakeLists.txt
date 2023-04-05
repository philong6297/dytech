# Core module
add_executable(core_test)
target_sources(
  core_test
  PRIVATE core/acceptor_test.cpp
          core/buffer_test.cpp
          core/cache_test.cpp
          core/connection_test.cpp
          core/looper_test.cpp
          core/net_address_test.cpp
          core/poller_test.cpp
          core/socket_test.cpp
          core/thread_pool_test.cpp
)
target_link_libraries(core_test PRIVATE core Catch2::Catch2WithMain)
target_compile_options(core_test PRIVATE ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_compile_features(core_test PRIVATE ${LONGLP_DESIRED_COMPILE_FEATURES})
target_include_directories(core_test PRIVATE ${LONGLP_PROJECT_SRC_DIR})

# HTTP module
add_executable(http_test)
target_sources(http_test PRIVATE http/header_test.cpp http/request_test.cpp http/response_test.cpp)
target_link_libraries(http_test PRIVATE core http Catch2::Catch2WithMain)
target_compile_options(http_test PRIVATE ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_compile_features(http_test PRIVATE ${LONGLP_DESIRED_COMPILE_FEATURES})
target_include_directories(http_test PRIVATE ${LONGLP_PROJECT_SRC_DIR})

# ---- Discover tests ----

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

catch_discover_tests(core_test)
catch_discover_tests(http_test)
