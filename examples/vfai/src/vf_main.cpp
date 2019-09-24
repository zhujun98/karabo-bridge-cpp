/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#include <QApplication>

#include <vf_mainwindow.hpp>


int main (int argc, char* argv[])
{
  QApplication app(argc, argv);
  app.setStyleSheet(
    "QTabWidget::pane { border: 0; }"
  );
  vf::MainWindow window;

  window.show();

  return app.exec();
}
