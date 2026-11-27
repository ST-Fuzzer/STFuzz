#include "gui/MainWindow.hpp"

#include <QtCore/qcontainerfwd.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qprogressbar.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qstatusbar.h>
#include <QtWidgets/qtreewidget.h>
#include <QtWidgets/qwidget.h>

#include <initializer_list>

namespace stfuzz {
namespace gui {
namespace main_window {

namespace {

constexpr const char* kStyleSheet = R"(
QMainWindow, QStatusBar { background: #eef2f7; color: #1b2a41; }
QLabel { background: transparent; }
QFrame#panel { background: white; border: 1px solid #c9d3e0; }
QFrame#panel_header { background: #0b2a5b; border: none; }
QFrame#panel_header QLabel { color: white; }
QLabel#panel_title { font-weight: bold; }
QFrame#separator { color: #c9d3e0; }
QLabel#muted { color: #8a97a8; }
QLabel#state_badge { background: white; border: 1px solid #c9d3e0; padding: 4px 14px; }
QPushButton { background: white; border: 1px solid #c9d3e0; padding: 5px 12px; }
QPushButton:disabled { color: #a9b4c3; background: #f6f8fb; }
QPushButton#step { text-align: left; padding: 6px 10px; }
QComboBox { background: white; color: #1b2a41; border: 1px solid #c9d3e0; padding: 1px 6px; }
QTreeWidget { border: none; background: white; }
QHeaderView::section { background: #f3f6fa; border: none; padding: 3px 6px; color: #5b6b82; }
QProgressBar { border: 1px solid #c9d3e0; background: #eef2f7; max-height: 10px; }
QProgressBar::chunk { background: #3aa6e0; }
QStatusBar { background: #0b2a5b; }
QStatusBar QLabel { color: white; }
)";

constexpr int kLeftColumnWidth = 340;
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
constexpr int kMargin = 8;
constexpr int kDevicesTreeHeight = 120;

QString Text(const char* text) { return QString::fromUtf8(text); }

QStringList Texts(std::initializer_list<const char*> texts) {
  QStringList list;
  for (const char* text : texts) {
    list.append(Text(text));
  }
  return list;
}

QLabel* MakeLabel(const char* text, const char* object_name = "") {
  auto* label = new QLabel(Text(text));
  label->setObjectName(Text(object_name));
  return label;
}

QPushButton* MakeButton(const char* text, bool enabled,
                        const char* object_name = "") {
  auto* button = new QPushButton(Text(text));
  button->setObjectName(Text(object_name));
  button->setEnabled(enabled);
  return button;
}

QFrame* MakeSeparator() {
  auto* line = new QFrame;
  line->setObjectName(Text("separator"));
  line->setFrameShape(QFrame::VLine);
  line->setFrameShadow(QFrame::Plain);
  return line;
}

QComboBox* MakeVerbosity(std::initializer_list<const char*> items) {
  auto* combo = new QComboBox;
  combo->addItems(Texts(items));
  return combo;
}

QTreeWidget* MakeTree(const QStringList& headers,
                      const QList<QStringList>& rows) {
  auto* tree = new QTreeWidget;
  tree->setHeaderLabels(headers);
  tree->setRootIsDecorated(false);
  for (const QStringList& row : rows) {
    tree->addTopLevelItem(new QTreeWidgetItem(row));
  }
  return tree;
}

QFrame* MakePanel(const char* title, QWidget* body,
                  const QList<QWidget*>& left_widgets = {},
                  const QList<QWidget*>& right_widgets = {}) {
  auto* panel = new QFrame;
  panel->setObjectName(Text("panel"));
  auto* layout = new QVBoxLayout(panel);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  auto* header = new QFrame;
  header->setObjectName(Text("panel_header"));
  auto* header_layout = new QHBoxLayout(header);
  header_layout->setContentsMargins(kMargin, 4, kMargin, 4);
  header_layout->addWidget(MakeLabel(title, "panel_title"));
  for (QWidget* widget : left_widgets) {
    header_layout->addWidget(widget);
  }
  header_layout->addStretch();
  for (QWidget* widget : right_widgets) {
    header_layout->addWidget(widget);
  }

  layout->addWidget(header);
  layout->addWidget(body, 1);
  return panel;
}

QWidget* MakeKeyValueBody(const QStringList& keys,
                          Qt::AlignmentFlag value_alignment) {
  auto* body = new QWidget;
  auto* grid = new QGridLayout(body);
  grid->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
  grid->setColumnStretch(1, 1);
  int row = 0;
  for (const QString& key : keys) {
    auto* key_label = new QLabel(key);
    key_label->setObjectName(Text("muted"));
    grid->addWidget(key_label, row, 0);
    grid->addWidget(new QLabel(Text("\u2014")), row, 1, value_alignment);
    ++row;
  }
  grid->setRowStretch(row, 1);
  return body;
}

QWidget* MakeToolbar() {
  auto* toolbar = new QWidget;
  auto* layout = new QHBoxLayout(toolbar);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(MakeButton("New run", true, "new_run"));
  layout->addWidget(MakeButton("Open run", true, "open_run"));
  layout->addWidget(MakeSeparator());
  layout->addWidget(MakeButton("Start", false));
  layout->addWidget(MakeButton("Pause", false));
  layout->addWidget(MakeButton("Resume", false));
  layout->addWidget(MakeButton("Stop", false));
  layout->addWidget(MakeSeparator());
  layout->addWidget(MakeButton("Save run", false));
  layout->addWidget(MakeButton("Exit run", false));
  layout->addStretch();
  layout->addWidget(MakeLabel("Create or open a run to continue.", "muted"));
  layout->addWidget(MakeLabel("No run", "state_badge"));
  return toolbar;
}

QWidget* MakeSteps() {
  const QStringList titles = Texts({
      "Run name and path",
      "Devices configuration",
      "Protocol and layer",
      "Network settings",
      "Packets configuration",
  });
  const QStringList states =
      Texts({"Not done", "Locked", "Locked", "Locked", "Locked"});

  auto* steps = new QWidget;
  auto* layout = new QHBoxLayout(steps);
  layout->setContentsMargins(0, 0, 0, 0);
  for (qsizetype i = 0; i < titles.size(); ++i) {
    auto* button = new QPushButton(QString::number(i + 1) + Text("  ") +
                                   titles[i] + Text("\n") + states[i]);
    button->setObjectName(Text("step"));
    button->setEnabled(i == 0);
    layout->addWidget(button, 1);
  }
  return steps;
}

QWidget* MakeLeftColumn() {
  auto* column = new QWidget;
  column->setFixedWidth(kLeftColumnWidth);
  auto* layout = new QVBoxLayout(column);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(kMargin);

  QList<QStringList> devices;
  devices.append(Texts({"Injector", "Not selected", "\u2014", "\u2014"}));
  devices.append(Texts({"DUT", "Not selected", "\u2014", "\u2014"}));
  devices.append(Texts({"Sniffer", "Not used", "\u2014", "\u2014"}));
  QTreeWidget* devices_tree =
      MakeTree(Texts({"Role", "Port", "Link", "Health"}), devices);
  devices_tree->header()->setSectionResizeMode(QHeaderView::Stretch);
  devices_tree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  devices_tree->setFixedHeight(kDevicesTreeHeight);
  layout->addWidget(MakePanel("Devices and health", devices_tree));

  layout->addWidget(MakePanel("Results", MakeKeyValueBody(Texts({
                                                              "Tests done",
                                                              "Responses",
                                                              "Errors",
                                                              "Timeouts",
                                                              "Crashes",
                                                          }),
                                                          Qt::AlignRight)));

  layout->addWidget(
      MakePanel("Run", MakeKeyValueBody(Texts({
                                            "Name",
                                            "Folder",
                                            "Protocol and layer",
                                            "Packets configuration",
                                            "Elapsed",
                                        }),
                                        Qt::AlignLeft)),
      1);
  return column;
}

QWidget* MakeProgressPanel() {
  auto* body = new QWidget;
  auto* layout = new QHBoxLayout(body);
  layout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
  auto* bar = new QProgressBar;
  bar->setRange(0, 100);
  bar->setValue(0);
  bar->setTextVisible(false);
  layout->addWidget(bar, 1);
  layout->addWidget(MakeLabel("No run", "muted"));
  return MakePanel("Progress", body);
}

QWidget* MakeRunLogPanel() {
  auto* body = new QWidget;
  auto* grid = new QGridLayout(body);
  grid->setContentsMargins(0, 0, 0, 0);
  grid->addWidget(
      MakeTree(Texts({"Time", "Test number", "Status", "Elapsed"}), {}), 0, 0);
  auto* empty = MakeLabel("No tests yet.", "muted");
  empty->setAttribute(Qt::WA_TransparentForMouseEvents);
  grid->addWidget(empty, 0, 0, Qt::AlignCenter);

  return MakePanel("Run log", body, {MakeLabel("Click a test to open it", "")},
                   {
                       MakeLabel("Verbosity", ""),
                       MakeVerbosity({"All tests", "Problems only"}),
                       MakeLabel("0 entries", ""),
                   });
}

QWidget* MakeEventLogPanel() {
  const QString now = QTime::currentTime().toString(Text("HH:mm:ss"));
  QList<QStringList> events;
  events.append({now, Text("INFO"), Text("App"), Text("Application started")});
  events.append({now, Text("INFO"), Text("App"), Text("Session ready")});

  return MakePanel(
      "Event log",
      MakeTree(Texts({"Time", "Level", "Source", "Message"}), events), {},
      {
          MakeLabel("Verbosity", ""),
          MakeVerbosity({"All events", "Problems only"}),
          MakeLabel("2 entries", ""),
      });
}

QWidget* MakeRightColumn() {
  auto* column = new QWidget;
  auto* layout = new QVBoxLayout(column);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(kMargin);
  layout->addWidget(MakeProgressPanel());
  layout->addWidget(MakeRunLogPanel(), 3);
  layout->addWidget(MakeEventLogPanel(), 2);
  return column;
}

void FillStatusBar(QStatusBar* bar) {
  bar->setSizeGripEnabled(false);
  bar->addWidget(MakeLabel("State: No run", ""));
  bar->addWidget(MakeLabel("Elapsed: \u2014", ""));
  bar->addWidget(MakeLabel("Rate: \u2014", ""));
  bar->addPermanentWidget(MakeLabel("Run: \u2014", ""));
}

}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  setWindowTitle(Text("ST.fuzz"));
  setStyleSheet(Text(kStyleSheet));

  auto* central = new QWidget;
  auto* layout = new QVBoxLayout(central);
  layout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
  layout->setSpacing(kMargin);
  layout->addWidget(MakeToolbar());
  layout->addWidget(MakeSteps());

  auto* body = new QHBoxLayout;
  body->setSpacing(kMargin);
  body->addWidget(MakeLeftColumn());
  body->addWidget(MakeRightColumn(), 1);
  layout->addLayout(body, 1);

  setCentralWidget(central);
  connect(findChild<QPushButton*>(Text("new_run")), &QPushButton::clicked, this,
          &MainWindow::NewRunRequested);
  connect(findChild<QPushButton*>(Text("open_run")), &QPushButton::clicked,
          this, &MainWindow::OpenRunRequested);
  FillStatusBar(statusBar());
  resize(kWindowWidth, kWindowHeight);
}

}  // namespace main_window
}  // namespace gui
}  // namespace stfuzz
