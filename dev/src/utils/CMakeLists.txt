add_library(stfuzz_utils STATIC
  ${STFUZZ_INC}/utils/ErrorCode.hpp
  ${STFUZZ_INC}/utils/SerDes.hpp
)
target_include_directories(stfuzz_utils PUBLIC ${STFUZZ_INC})
