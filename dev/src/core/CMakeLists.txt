add_subdirectory(adaptor)
add_subdirectory(main_controller)

add_library(stfuzz_core INTERFACE)
target_include_directories(stfuzz_core INTERFACE ${STFUZZ_INC})
target_link_libraries(stfuzz_core INTERFACE stfuzz_adaptor stfuzz_utils)
