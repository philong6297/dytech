# Build core library
add_library(core STATIC)
target_sources(
  core
  PRIVATE acceptor.cpp
          acceptor.h
          buffer.h
          cache.h
          connection.h
          looper.h
          net_address.h
          poller.h
          socket.h
          thread_pool.h
          server.h
          utils.h
          buffer.cpp
          cache.cpp
          connection.cpp
          looper.cpp
          net_address.cpp
          poller.cpp
          socket.cpp
          thread_pool.cpp
          server.cpp
)
target_link_libraries(core PUBLIC log Threads::Threads base)
target_compile_options(core PUBLIC ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(core PUBLIC ${LONGLP_PROJECT_SRC_DIR})
