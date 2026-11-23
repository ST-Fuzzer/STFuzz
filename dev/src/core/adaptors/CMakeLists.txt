add_library(stfuzz_adaptors STATIC
  adapters_placeholder.cpp
  ${STFUZZ_INC}/core/adaptors/adapters_placeholder.hpp
  ${STFUZZ_INC}/core/adaptors/AdaptorInterface.hpp
  ${STFUZZ_INC}/core/adaptors/ConfigInterface.hpp
  ${STFUZZ_INC}/core/adaptors/SerialInterface.hpp
  ${STFUZZ_INC}/core/adaptors/FrameInterface.hpp
  ${STFUZZ_INC}/core/adaptors/FrameSubscriber.hpp
  ${STFUZZ_INC}/core/adaptors/StatusInterface.hpp
  ${STFUZZ_INC}/core/adaptors/StatusSubscriber.hpp
  ${STFUZZ_INC}/core/adaptors/FeedbackInterface.hpp
  ${STFUZZ_INC}/core/adaptors/FeedbackSubscriber.hpp
)
target_include_directories(stfuzz_adaptors PUBLIC ${STFUZZ_INC})
target_link_libraries(stfuzz_adaptors PUBLIC stfuzz_utils)
