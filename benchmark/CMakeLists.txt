# 'make benchmark' will run http_server
set(WEBBENCH_DIR ${PROJECT_SOURCE_DIR}/webbench)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CONCURRENCY 10500) # concurrent client number
  set(DURATION 5) # stress test duration
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CONCURRENCY 105)
  set(DURATION 10)
endif()

add_custom_target(
  benchmark
  WORKING_DIRECTORY ${WEBBENCH_DIR}
  COMMAND sh ./benchmark.sh ${CONCURRENCY} ${DURATION}
  DEPENDS http_server
)
