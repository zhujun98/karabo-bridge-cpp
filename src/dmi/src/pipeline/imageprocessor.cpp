/*
    Copyright (c) 2019, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

#include <QDebug>

#include <xfai/area_detector.hpp>
#include "imageprocessor.hpp"


dmi::ImageProcessor::ImageProcessor(QObject *parent)
  : QThread(parent), queue_(nullptr), thresh_lb_(-1.e6), thresh_ub_(1.e6)
{
}

void dmi::ImageProcessor::connect(const std::shared_ptr<PipeLineQueue>& output)
{
  queue_ = output;
}


void dmi::ImageProcessor::run()
{
  cv::Mat colored_view;

  xfai::JungFrau1M<xfai::ImageDataType::cal> jf;
  xfai::DSSC1M<xfai::ImageDataType::raw> dssc;
  xfai::LPD1M<xfai::ImageDataType::cal> lpd;

  forever
  {
    if (isInterruptionRequested()) return;

    std::pair<MetaData, PipeLineData > data;
    if (queue_->try_pop(data))
    {
      if (data.first.source_category == "JungFrau")
      {
        jf.update(data.second);
        jf.process({thresh_lb_, thresh_ub_});
        cv::applyColorMap(jf.assembled(), colored_view, cv::COLORMAP_SUMMER);
        cv::cvtColor(colored_view, colored_view, cv::COLOR_BGR2RGB);
        emit newFrame(QPixmap::fromImage(QImage(colored_view.data,
                                                colored_view.cols,
                                                colored_view.rows,
                                                colored_view.step,
                                                QImage::Format_RGB888)));
      } else if (data.first.source_category == "LPD") {
        lpd.update(data.second);
        lpd.process({thresh_lb_, thresh_ub_});
        cv::applyColorMap(lpd.assembled(), colored_view, cv::COLORMAP_SUMMER);
        cv::cvtColor(colored_view, colored_view, cv::COLOR_BGR2RGB);
        emit newFrame(QPixmap::fromImage(QImage(colored_view.data,
                                                colored_view.cols,
                                                colored_view.rows,
                                                colored_view.step,
                                                QImage::Format_RGB888)));
      }

      emit imageProcessed();
    }
    msleep(1);
  }

}

void dmi::ImageProcessor::setThreshLower(double v) { thresh_lb_ = v; }

void dmi::ImageProcessor::setThreshUpper(double v) { thresh_ub_ = v; }
