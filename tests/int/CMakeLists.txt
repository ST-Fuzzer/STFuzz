add_executable(int_test int_placeholder.cpp)
target_link_libraries(int_test PRIVATE stfuzz_test_main)
target_link_libraries(int_test PRIVATE stfuzz_core)
gtest_discover_tests(int_test PROPERTIES LABELS int)
