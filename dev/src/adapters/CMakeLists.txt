add_library(stfuzz_adapters STATIC
  adapters_placeholder.cpp
  ${STFUZZ_INC}/adapters/adapters_placeholder.hpp
)
target_include_directories(stfuzz_adapters PUBLIC ${STFUZZ_INC})
target_link_libraries(stfuzz_adapters PUBLIC stfuzz_utils)
