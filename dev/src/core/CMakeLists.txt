add_library(stfuzz_core STATIC
  core_placeholder.cpp
  ${STFUZZ_INC}/core/core_placeholder.hpp
)
target_include_directories(stfuzz_core PUBLIC ${STFUZZ_INC})
target_link_libraries(stfuzz_core PUBLIC stfuzz_adapters stfuzz_utils Qt6::Core)
