add_library(stfuzz_core STATIC
  device_manager.cpp
  ${STFUZZ_INC}/core/device_manager.hpp
)
target_include_directories(stfuzz_core PUBLIC ${STFUZZ_INC})
target_link_libraries(stfuzz_core
  PUBLIC stfuzz_adapters stfuzz_utils Qt6::Core
  PRIVATE Qt6::SerialPort
)
