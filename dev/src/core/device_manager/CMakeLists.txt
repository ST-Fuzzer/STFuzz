add_library(stfuzz_device_manager STATIC
  DeviceManager.cpp
  ${STFUZZ_INC}/core/device_manager/DeviceManager.hpp
  ${STFUZZ_INC}/core/device_manager/DeviceSubscriber.hpp
  ${STFUZZ_INC}/core/device_manager/DeviceTypes.hpp
)
target_include_directories(stfuzz_device_manager PUBLIC ${STFUZZ_INC})
target_link_libraries(stfuzz_device_manager
  PUBLIC stfuzz_utils
  PRIVATE setupapi advapi32
)
