/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#include <QLayout>
#include <QDebug>

#include "lineanalysis_widget.hpp"


dmi::LineAnalysisWidget::LineAnalysisWidget(QWidget *parent) : QWidget(parent)
{
  initUI();
  initConnections();
}

void dmi::LineAnalysisWidget::initUI()
{
  view_ = new QtCharts::QChartView();
  series_ = new QtCharts::QLineSeries(view_);

  auto* chart = new QtCharts::QChart();
  chart->legend()->hide();
  chart->addSeries(series_);
  chart->createDefaultAxes();
  chart->setTitle("Detector name here");

  view_->setChart(chart);
  view_->setRenderHint(QPainter::Antialiasing);

  auto layout = new QVBoxLayout();
  layout->addWidget(view_);
  setLayout(layout);
}

void dmi::LineAnalysisWidget::initConnections()
{
}

void dmi::LineAnalysisWidget::updateChart()
{
  qDebug() << "Update chart";
}
