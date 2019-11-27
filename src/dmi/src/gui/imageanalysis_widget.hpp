/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#ifndef KBCPP_DMI_IMAGEANALYSIS_WIDGET_H
#define KBCPP_DMI_IMAGEANALYSIS_WIDGET_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGroupBox>
#include <QLineEdit>
#include <QSplitter>
#include <QSet>

#include "pipeline/imageprocessor.hpp"


namespace dmi
{

class ImageAnalysisWidget : public QSplitter
{
  Q_OBJECT

public:

  explicit ImageAnalysisWidget(QWidget* parent = nullptr);

  ~ImageAnalysisWidget() override = default;

  void addProcessor(ImageProcessor* ptr);

private:

  void initUI();
  void initCtrlUI();
  void initConnections();

public slots:

  // set the new processed image received from the data processor
  void updateImage(QPixmap pix);

private:

  QGraphicsScene* image_scene_;
  QGraphicsView* image_view_;

  QGroupBox* image_ctrl_;
  QLineEdit* thresh_lower_le_;
  QLineEdit* thresh_upper_le_;

  QGraphicsPixmapItem pixmap_;

  QSet<ImageProcessor*> img_procs_;
};

}

#endif //KBCPP_DMI_IMAGEANALYSIS_WIDGET_H
