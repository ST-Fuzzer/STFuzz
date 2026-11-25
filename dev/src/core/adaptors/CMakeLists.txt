add_library(stfuzz_adaptors STATIC
  ${STFUZZ_INC}/core/adaptors/AdaptorConfigInterface.hpp
  ${STFUZZ_INC}/core/adaptors/AdaptorInterface.hpp
)
target_include_directories(stfuzz_adaptors PUBLIC ${STFUZZ_INC})
target_link_libraries(stfuzz_adaptors PUBLIC stfuzz_utils)
