/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#include <Qt>
#include <QLayout>
#include <QValidator>
#include <QLabel>
#include <QSizePolicy>

#include "imageanalysis_widget.hpp"


dmi::ImageAnalysisWidget::ImageAnalysisWidget(QWidget *parent) :
  QSplitter(Qt::Horizontal, parent)
{
  initUI();
  initConnections();
}

void dmi::ImageAnalysisWidget::initUI()
{
  image_scene_ = new QGraphicsScene();
  image_view_ = new QGraphicsView(image_scene_);

  image_view_->scene()->addItem(&pixmap_);

  initCtrlUI();

  addWidget(image_view_);
  addWidget(image_ctrl_);
}

void dmi::ImageAnalysisWidget::initConnections()
{
}

void dmi::ImageAnalysisWidget::updateImage(QPixmap pix)
{
  pixmap_.setPixmap(pix);
}

void dmi::ImageAnalysisWidget::initCtrlUI()
{
  image_ctrl_ = new QGroupBox("Image process");

  auto thresh_validator = new QDoubleValidator(image_ctrl_);
  auto thresh_lower_lb = new QLabel("Threshold (lower): ");
  thresh_lower_le_ = new QLineEdit("-100000", image_ctrl_);
  thresh_lower_le_->setValidator(thresh_validator);
  thresh_lower_le_->setAlignment(Qt::AlignRight);
  thresh_lower_le_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  auto thresh_upper_lb = new QLabel("Threshold (upper): ");
  thresh_upper_le_ = new QLineEdit("100000", image_ctrl_);
  thresh_upper_le_->setValidator(thresh_validator);
  thresh_upper_le_->setAlignment(Qt::AlignRight);
  thresh_upper_le_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

  auto layout = new QGridLayout();
  layout->addWidget(thresh_lower_lb, 0, 0);
  layout->addWidget(thresh_lower_le_, 0, 1);
  layout->addWidget(thresh_upper_lb, 1, 0);
  layout->addWidget(thresh_upper_le_, 1, 1);
  layout->setRowStretch(2, 1);

  image_ctrl_->setLayout(layout);
  image_ctrl_->setFixedWidth(image_ctrl_->minimumSizeHint().width());
}

void dmi::ImageAnalysisWidget::addProcessor(dmi::ImageProcessor *ptr)
{
  img_procs_.insert(ptr);

  connect(thresh_lower_le_, &QLineEdit::returnPressed, [ptr, this]()
  {
    ptr->setThreshLower(thresh_lower_le_->text().toDouble());
  });
  connect(thresh_upper_le_, &QLineEdit::returnPressed, [ptr, this]()
  {
    ptr->setThreshUpper(thresh_upper_le_->text().toDouble());
  });
}