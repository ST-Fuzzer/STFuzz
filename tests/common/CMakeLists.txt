add_library(stfuzz_test_main STATIC test_main.cpp)
target_link_libraries(stfuzz_test_main PUBLIC
  Qt6::Core Qt6::Test GTest::gtest GTest::gmock)
