
add_library(stfuzz_adapters STATIC
  # Source files
  CommandManager.cpp
  CommandFeedbackManager.cpp
  TransportManager.cpp
  TransportFeedbackManager.cpp
  TelemetryProcessor.cpp
  HealthMonitor.cpp
  FeedbackPublisher.cpp

  # Header files
  ${STFUZZ_INC}/adapters/STFuzzTypes.hpp
  ${STFUZZ_INC}/adapters/ITelemetryDecoder.hpp
  ${STFUZZ_INC}/adapters/CommandManager.hpp
  ${STFUZZ_INC}/adapters/CommandFeedbackManager.hpp
  ${STFUZZ_INC}/adapters/TransportManager.hpp
  ${STFUZZ_INC}/adapters/TransportFeedbackManager.hpp
  ${STFUZZ_INC}/adapters/TelemetryProcessor.hpp
  ${STFUZZ_INC}/adapters/HealthMonitor.hpp
  ${STFUZZ_INC}/adapters/FeedbackPublisher.hpp
)

# Enable Qt Meta-Object Compiler (MOC) for Q_OBJECT signals and slots
set_target_properties(stfuzz_adapters PROPERTIES AUTOMOC ON)

target_include_directories(stfuzz_adapters PUBLIC ${STFUZZ_INC})

# Link stfuzz_utils along with Qt6::Core for QObject, QByteArray, QTimer, etc.
target_link_libraries(stfuzz_adapters 
  PUBLIC 
    stfuzz_utils
    Qt6::Core
)
