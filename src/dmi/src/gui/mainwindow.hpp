/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#ifndef KBCPP_DMI_MAINWINDOW_H
#define KBCPP_DMI_MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QComboBox>
#include <QTabWidget>
#include <QSplitter>
#include <QTreeWidget>

#include "pipeline/databroker.hpp"
#include "pipeline/imageprocessor.hpp"
#include "datasource_widget.hpp"
#include "lineanalysis_widget.hpp"
#include "imageanalysis_widget.hpp"


namespace dmi
{

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

  ~MainWindow() override;

  static constexpr int SPLITTER_HANDLE_WIDTH = 9;

private:
  void initUI();
  void initLeftUI();
  void initCentralUI();
  void initConnections();
  void initToolbar();
  void initStatusbar();

private slots:
  void onStart();
  void onStop();

signals:
  // The signal is emitted when the "start" button is pressed.
  void daqStarted();
  // The signal is emitted when the "stop" button is pressed.
  void daqStopped();

private:
  static constexpr size_t width_ = 1600;
  static constexpr size_t height_ = 1400;

  QToolBar* tool_bar_;

  QSplitter* cw_; // central widget

  // left central widgets
  QTabWidget* l_panel_;
  DataSourceWidget* ds_widget_;

  // middle central widgets
  QSplitter* c_panel_;
  QTabWidget* ct_panel_;
  QTabWidget* cb_panel_;
  ImageAnalysisWidget* image_analysis_;
  LineAnalysisWidget* line_analysis_;
  QWidget* logger_; // placeholder for now

  QAction* start_act_;
  QAction* stop_act_;

  QLabel* status_label_;

  DataBroker* broker_;
  ImageProcessor* img_proc_;
};

} //dmi

#endif //KBCPP_DMI_MAINWINDOW_H
