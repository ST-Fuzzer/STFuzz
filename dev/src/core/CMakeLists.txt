add_subdirectory(adaptors)

add_library(stfuzz_core INTERFACE)
target_include_directories(stfuzz_core INTERFACE ${STFUZZ_INC})
target_link_libraries(stfuzz_core INTERFACE stfuzz_adaptors stfuzz_utils Qt6::Core)
