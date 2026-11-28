add_library(stfuzz_main_controller STATIC
  ${STFUZZ_INC}/core/main_controller/MainController.hpp
  MainController.cpp
)
target_include_directories(stfuzz_main_controller PUBLIC ${STFUZZ_INC})
target_link_libraries(stfuzz_main_controller PUBLIC stfuzz_adaptor)
