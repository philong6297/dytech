# 'make benchmark' will run http_server
set(WEBBENCH_DIR ${LONGLP_PROJECT_BENCHMARK_DIR})

set(CONCURRENCY 10500) # concurrent client number
set(DURATION 5) # stress test duration

add_custom_target(
  benchmark
  WORKING_DIRECTORY ${WEBBENCH_DIR}
  COMMAND sh ./benchmark.sh ${CONCURRENCY} ${DURATION}
  DEPENDS http_server
)
