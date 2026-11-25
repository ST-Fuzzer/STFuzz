add_subdirectory(adaptor)
add_subdirectory(device_manager)

add_library(stfuzz_core INTERFACE)
target_include_directories(stfuzz_core INTERFACE ${STFUZZ_INC})
target_link_libraries(stfuzz_core INTERFACE stfuzz_adaptor stfuzz_utils stfuzz_device_manager)
