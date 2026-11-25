add_library(stfuzz_adaptor STATIC
  ${STFUZZ_INC}/core/adaptor/AdaptorConfigInterface.hpp
  ${STFUZZ_INC}/core/adaptor/AdaptorInterface.hpp
  ${STFUZZ_INC}/core/adaptor/AdaptorSubscriber.hpp
)
target_include_directories(stfuzz_adaptor PUBLIC ${STFUZZ_INC})
target_link_libraries(stfuzz_adaptor PUBLIC stfuzz_utils)
