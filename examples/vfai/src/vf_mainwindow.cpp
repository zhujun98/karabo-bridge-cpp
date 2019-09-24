/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#include <iostream>

#include <QThread>
#include <QStatusBar>
#include <QLayout>
#include <Qt>
#include <QDebug>

#include "vf_mainwindow.hpp"


vf::MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  initUI();
  initConnections();

  img_proc_->connect(broker_->outputChannel());

  image_analysis_->addProcessor(img_proc_);
}

vf::MainWindow::~MainWindow()
{
  broker_->requestInterruption();
  img_proc_->requestInterruption();
  qDebug() << "Requested interruptions of QThreads!";
  broker_->quit();
  img_proc_->quit();
  qDebug() << "Waiting for QThreads to join ...";
  broker_->wait();
  img_proc_->wait();
  qDebug() << "QThreads terminated!";
}

void vf::MainWindow::initUI()
{
  this->setWindowTitle("VFAI");
  this->resize(width_, height_);

  initToolbar();
  initStatusbar();

  cw_ = new QSplitter(this);
  cw_->setChildrenCollapsible(false);
  cw_->setHandleWidth(SPLITTER_HANDLE_WIDTH);
  l_panel_ = new QTabWidget();
  c_panel_ = new QSplitter(Qt::Vertical);
  c_panel_->setChildrenCollapsible(false);
  c_panel_->setHandleWidth(SPLITTER_HANDLE_WIDTH);
  cw_->addWidget(l_panel_);
  cw_->addWidget(c_panel_);
  setCentralWidget(cw_);

  initLeftUI();
  initCentralUI();

  setMinimumSize(640, 480);
}

void vf::MainWindow::initLeftUI()
{
  ds_widget_ = new DataSourceWidget();
  l_panel_->setTabPosition(QTabWidget::West);
  l_panel_->addTab(ds_widget_, "Data source");
}

void vf::MainWindow::initCentralUI()
{
  ct_panel_ = new QTabWidget();
  cb_panel_ = new QTabWidget();
  c_panel_->addWidget(ct_panel_);
  c_panel_->addWidget(cb_panel_);

  // top
  image_analysis_ = new ImageAnalysisWidget(ct_panel_);
  line_analysis_ = new LineAnalysisWidget(ct_panel_);
  ct_panel_->addTab(image_analysis_, "Image Analysis");
  ct_panel_->addTab(line_analysis_, "Line analysis");

  // bottom
  logger_ = new QWidget();
  cb_panel_->setTabPosition(QTabWidget::South);
  cb_panel_->addTab(logger_, "Logger");
}

void vf::MainWindow::initToolbar()
{
  tool_bar_ = addToolBar("View");

  start_act_ = new QAction("&Start", this);
  stop_act_ = new QAction("&Stop", this);
  stop_act_->setEnabled(false);

  tool_bar_->addAction(start_act_);
  tool_bar_->addAction(stop_act_);
}

void vf::MainWindow::initStatusbar()
{
  status_label_ = new QLabel();
  this->statusBar()->addPermanentWidget(status_label_);
  status_label_->setText("Ready");
}

void vf::MainWindow::initConnections()
{
  connect(start_act_, &QAction::triggered, this, &vf::MainWindow::onStart);
  connect(start_act_, &QAction::triggered, ds_widget_, &vf::DataSourceWidget::onStart);
  connect(stop_act_, &QAction::triggered, this, &vf::MainWindow::onStop);
  connect(stop_act_, &QAction::triggered, ds_widget_, &vf::DataSourceWidget::onStop);

  // data broker
  broker_ = new DataBroker(this);
  connect(this, &MainWindow::daqStarted, broker_, [this]()
  {
    this->broker_->setEndpoint(this->ds_widget_->endpoint());
    this->broker_->setSourceType(this->ds_widget_->sourceType());
    this->broker_->start();
  });
  connect(this, &MainWindow::daqStopped, broker_, &DataBroker::stop);

  // image processor (run forever)
  img_proc_ = new ImageProcessor(this);
  img_proc_->start();

  connect(img_proc_, &vf::ImageProcessor::newFrame, image_analysis_, &ImageAnalysisWidget::updateImage);
  connect(img_proc_, &vf::ImageProcessor::imageProcessed, broker_, &vf::DataBroker::dataProcessed);

  connect(broker_, &vf::DataBroker::newSources, [this](const QStringList& srcs)
  {
    this->ds_widget_->updateSourceList(srcs);
  });

  connect(ds_widget_, &DataSourceWidget::sourceToggled, [this](SourceItem item, bool checked)
  {
    this->broker_->updateSources(item, checked);
  });

  connect(broker_, &DataBroker::newLine, line_analysis_, &LineAnalysisWidget::updateChart);
}

void vf::MainWindow::onStart()
{
  start_act_->setEnabled(false);
  stop_act_->setEnabled(true);

  emit daqStarted();
  status_label_->setText("Acquiring ...");
}

void vf::MainWindow::onStop()
{
  emit daqStopped();
  broker_->wait();
  status_label_->setText("Ready");

  stop_act_->setEnabled(false);
  start_act_->setEnabled(true);
}
