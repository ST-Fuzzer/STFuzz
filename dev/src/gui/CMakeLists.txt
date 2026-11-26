add_library(stfuzz_gui STATIC
  WindowController.cpp
  ${STFUZZ_INC}/gui/WindowController.hpp
)
target_include_directories(stfuzz_gui PUBLIC ${STFUZZ_INC})
target_link_libraries(stfuzz_gui PUBLIC stfuzz_core Qt6::Widgets)
