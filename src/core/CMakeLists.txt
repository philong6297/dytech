# Build core library
add_library(core STATIC)
target_sources(
  core
  PRIVATE acceptor.cc
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
          buffer.cc
          cache.cc
          connection.cc
          looper.cc
          net_address.cc
          poller.cc
          socket.cc
          thread_pool.cc
          server.cc
          typedefs.h
          distribution_agent.cc
          distribution_agent.h
)
target_link_libraries(core PUBLIC log Threads::Threads base)
target_compile_options(core PUBLIC ${LONGLP_DESIRED_COMPILE_OPTIONS})
target_include_directories(core PUBLIC ${LONGLP_PROJECT_SRC_DIR})
