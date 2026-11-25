#pragma once

#include <QtCore/qobject.h>

#include "utils/ErrorCode.hpp"

class QDialog;
class QWidget;

namespace stfuzz {
namespace gui {
namespace window_controller {

class WindowController : public QObject {
  Q_OBJECT

 public:
  explicit WindowController(QWidget* main_window, QObject* parent = nullptr);

  [[nodiscard]] stfuzz::utils::ErrorCode ShowWindow(QDialog* window);

  void OpenMainWindow();

  QWidget* main_window() const { return main_window_; }
  QDialog* current_window() const { return current_window_; }

 private:
  QWidget* main_window_;
  QDialog* current_window_ = nullptr;
};

}  // namespace window_controller
}  // namespace gui
}  // namespace stfuzz
