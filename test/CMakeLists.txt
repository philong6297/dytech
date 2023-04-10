list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# Core module
set(CORE_TARGETS
    acceptor_test
    buffer_test
    cache_test
    connection_test
    looper_test
    net_address_test
    poller_test
    socket_test
    thread_pool_test
)
foreach(target ${CORE_TARGETS})
  add_executable(${target} core/${target}.cc)
  target_link_libraries(${target} PRIVATE core Catch2::Catch2WithMain fmt::fmt)
  target_compile_options(${target} PRIVATE ${LONGLP_DESIRED_COMPILE_OPTIONS})
  target_include_directories(${target} PRIVATE ${LONGLP_PROJECT_SRC_DIR})
  catch_discover_tests(${target})
endforeach()

# HTTP module
add_executable(http_test)
target_sources(http_test PRIVATE http/header_test.cc http/request_test.cc http/response_test.cc)
target_link_libraries(http_test PRIVATE core http Catch2::Catch2WithMain)
target_compile_options(http_test PRIVATE ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(http_test PRIVATE ${LONGLP_PROJECT_SRC_DIR})
catch_discover_tests(http_test)
