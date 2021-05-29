# Configure playground
add_executable(
    hyrisePlayground

    playground.cpp
        ../lib/operators/table_scan.cpp)
target_link_libraries(
    hyrisePlayground
    hyrise
)
