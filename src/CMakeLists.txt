# Build core library
add_library(core STATIC)
target_sources(
  core
  PRIVATE core/acceptor.cpp
          core/acceptor.h
          core/buffer.h
          core/cache.h
          core/connection.h
          core/looper.h
          core/net_address.h
          core/poller.h
          core/socket.h
          core/thread_pool.h
          core/server.h
          core/utils.h
          core/buffer.cpp
          core/cache.cpp
          core/connection.cpp
          core/looper.cpp
          core/net_address.cpp
          core/poller.cpp
          core/socket.cpp
          core/thread_pool.cpp
          core/server.cpp
)
target_link_libraries(core PUBLIC log Threads::Threads)
target_compile_options(core PUBLIC ${LONGLP_DESIRED_COMPILE_OPTIONS} -Wno-unsafe-buffer-usage)
target_include_directories(core PUBLIC ${LONGLP_PROJECT_SRC_DIR})

# Build the turtle http library
add_library(http STATIC)
target_sources(
  http
  PRIVATE http/header.h
          http/http_utils.h
          http/request.h
          http/response.h
          http/header.cpp
          http/http_utils.cpp
          http/request.cpp
          http/response.cpp
          http/cgier.h
          http/cgier.cpp
)
target_link_libraries(http PUBLIC log)
target_compile_options(http PUBLIC ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(http PUBLIC ${LONGLP_PROJECT_SRC_DIR})

# Build the turtle logging library
add_library(log STATIC)
target_sources(log PRIVATE log/logger.cpp log/logger.h)
target_link_libraries(log PUBLIC Threads::Threads fmt::fmt)
target_compile_options(log PUBLIC ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(log PUBLIC ${LONGLP_PROJECT_SRC_DIR})
