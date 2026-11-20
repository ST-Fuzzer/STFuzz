add_library(stfuzz_utils STATIC
  utils_placeholder.cpp
  ${STFUZZ_INC}/utils/utils_placeholder.hpp
)
target_include_directories(stfuzz_utils PUBLIC ${STFUZZ_INC})
