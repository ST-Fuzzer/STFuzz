#include <iostream>

#include "core/device_manager/DeviceManager.hpp"

int main() {
  stfuzz::core::device_manager::DeviceManager manager;

  std::cout << "Scanning COM ports...\n";

  manager.ScanPorts();

  const auto devices = manager.ShowDevices();

  std::cout << "Found " << devices.size() << " device(s).\n\n";

  for (const auto& device : devices) {
    std::cout << "Port: " << device.port_name << '\n';
    std::cout << "Device: " << device.device_name << '\n';
    std::cout << "Supported: " << (device.supported ? "Yes" : "No") << '\n';
    std::cout << "-------------------------\n";
  }

  return 0;
}
