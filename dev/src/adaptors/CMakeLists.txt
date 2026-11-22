add_library(stfuzz_adaptors STATIC
  adapters_placeholder.cpp
  ${STFUZZ_INC}/adaptors/adapters_placeholder.hpp
  ${STFUZZ_INC}/adaptors/AdaptorInterface.hpp
  ${STFUZZ_INC}/adaptors/ConfigInterface.hpp
  ${STFUZZ_INC}/adaptors/SerialInterface.hpp
  ${STFUZZ_INC}/adaptors/FrameInterface.hpp
  ${STFUZZ_INC}/adaptors/FeedbackInterface.hpp
  ${STFUZZ_INC}/adaptors/StatusInterface.hpp
)
target_include_directories(stfuzz_adaptors PUBLIC ${STFUZZ_INC})
target_link_libraries(stfuzz_adaptors PUBLIC stfuzz_utils)
