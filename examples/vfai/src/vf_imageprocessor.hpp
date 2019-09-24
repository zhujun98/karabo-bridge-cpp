/*
    Copyright (c) 2019, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_VF_IMAGEPROCESSOR_HPP
#define KARABO_BRIDGE_VF_IMAGEPROCESSOR_HPP

#include <tbb/concurrent_queue.h>

#include <QThread>
#include <QPixmap>

#include <vf_pipeline_data.hpp>

namespace vf
{

class ImageProcessor : public QThread
{
  Q_OBJECT

  void run() override;

public:
  explicit ImageProcessor(QObject* parent = nullptr);

  void connect(const std::shared_ptr<PipeLineQueue>& output);

signals:
  // emitted when a new frame is ready
  void newFrame(QPixmap pix);
  // emitted after processing an image data
  void imageProcessed();

public slots:
  // set the image lower threshold
  void setThreshLower(double v);
  // set the image upper threshold
  void setThreshUpper(double v);

private:
  std::shared_ptr<PipeLineQueue> queue_;

  double thresh_lb_; // image threshold lower bound
  double thresh_ub_; // image threshold upper bound
};

}

#endif //KARABO_BRIDGE_VF_IMAGEPROCESSOR_HPP
