#pragma once

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qwidget.h>

namespace stfuzz {
namespace gui {
namespace main_window {

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);

 signals:
  void NewRunRequested();
  void OpenRunRequested();
};

}  // namespace main_window
}  // namespace gui
}  // namespace stfuzz
