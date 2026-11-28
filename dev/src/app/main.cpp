#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qdialog.h>
#include <QtWidgets/qlabel.h>

#include "gui/MainWindow.hpp"
#include "gui/WindowController.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  stfuzz::gui::main_window::MainWindow window;
  stfuzz::gui::window_controller::WindowController controller(&window);
  static_cast<void>(controller.OpenMainWindow());

  QObject::connect(&window, &stfuzz::gui::main_window::MainWindow::NewRunRequested, [&controller] {
    auto* dialog = new QDialog;
    dialog->setWindowTitle("Run name and path (demo)");
    auto* layout = new QVBoxLayout(dialog);
    layout->addWidget(new QLabel("Run name and path"));
    static_cast<void>(controller.ShowWindow(dialog));
  });

  return QApplication::exec();
}
