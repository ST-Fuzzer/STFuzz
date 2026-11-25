#include "gui/WindowController.hpp"

#include <QtCore/qobject.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qwidget.h>

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace gui {
namespace window_controller {

WindowController::WindowController(QWidget* main_window, QObject* parent)
    : QObject(parent), main_window_(main_window) {}

stfuzz::utils::ErrorCode WindowController::ShowWindow(QDialog* window) {
  if (window == nullptr) {
    return stfuzz::utils::ErrorCode::kInvalidArgument;
  }
  if (window == current_window_) {
    return stfuzz::utils::ErrorCode::kOk;
  }

  QDialog* previous = current_window_;

  window->setParent(main_window_, window->windowFlags());
  current_window_ = window;

  connect(window, &QDialog::finished, this, [this, window] {
    if (current_window_ == window) {
      current_window_ = nullptr;
      window->deleteLater();
      main_window_->raise();
      main_window_->activateWindow();
    }
  });

  connect(window, &QObject::destroyed, this, [this, window] {
    if (current_window_ == window) {
      current_window_ = nullptr;
    }
  });

  window->open();

  if (previous != nullptr) {
    previous->hide();
    previous->deleteLater();
  }
  return stfuzz::utils::ErrorCode::kOk;
}

void WindowController::OpenMainWindow() {
  if (current_window_ != nullptr) {
    current_window_->hide();
    current_window_->deleteLater();
    current_window_ = nullptr;
  }
  main_window_->show();
  main_window_->raise();
  main_window_->activateWindow();
}

}  // namespace window_controller
}  // namespace gui
}  // namespace stfuzz
