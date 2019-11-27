/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KBCPP_DMI_BARCHART_HPP
#define KBCPP_DMI_BARCHART_HPP

#include <QVector>
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>


namespace dmi
{

class LineAnalysisWidget : public QWidget
{
  Q_OBJECT

public:

  explicit LineAnalysisWidget(QWidget* parent = nullptr);

  ~LineAnalysisWidget() override = default;

private:
  void initUI();
  void initConnections();

public slots:

  void updateChart();

private:
  QtCharts::QLineSeries* series_;
  QtCharts::QChartView* view_;

};

}

#endif //KBCPP_DMI_BARCHART_HPP
