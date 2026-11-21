add_library(stfuzz_utils STATIC
  frame_serializer.cpp
  ieee802154_protocol.cpp
  ${STFUZZ_INC}/utils/frame_serializer.hpp
  ${STFUZZ_INC}/utils/ieee802154_protocol.hpp
)
target_include_directories(stfuzz_utils PUBLIC ${STFUZZ_INC})
