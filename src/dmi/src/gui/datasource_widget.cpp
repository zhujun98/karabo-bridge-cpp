/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#include <QtWidgets>
#include <QLabel>
#include <QTabWidget>
#include <QSplitter>
#include <QMap>

#include "datasource_widget.hpp"
#include "xfai/xfai_config.hpp"


constexpr char dmi::DataSourceWidget::HOSTNAME[];
constexpr char dmi::DataSourceWidget::PORT[];


dmi::DataSourceWidget::DataSourceWidget(QWidget *parent)
    : QWidget(parent)
{
  initUI();
  initConnections();
}

void dmi::DataSourceWidget::initUI()
{
  auto source_type_lb = new QLabel("Data streamed from: ");
  source_type_cb_ = new QComboBox();
  source_type_cb_->addItem("Run directory", static_cast<int>(xfai::DataSourceType::file));
  source_type_cb_->addItem("ZeroMQ bridge", static_cast<int>(xfai::DataSourceType::zmq));

  auto hostname_lb = new QLabel("Hostname: ");
  hostname_le_ = new QLineEdit();
  hostname_le_->setText(HOSTNAME);
  auto port_lb = new QLabel("Port: ");
  port_le_ = new QLineEdit();
  port_le_->setText(PORT);

  tree_view_ = new QTreeView();
  tree_model_ = new DataSourceTreeModel(this);
  tree_view_->setModel(tree_model_);
  ppt_delegate_ = new DSPropertyDelegate(this);
//  fr_delegate_ = new DSFilterRangeDelegate(this);
  tree_view_->setItemDelegateForColumn(1, ppt_delegate_);
//  tree_view_->setItemDelegateForColumn(2, fr_delegate_);
  tree_view_->expandToDepth(1);
  tree_view_->resizeColumnToContents(0);

  list_view_ = new QListView();
  list_model_ = new DataSourceListModel(this);
  list_view_->setModel(list_model_);
  auto list_container = new QTabWidget(this);
  list_container->addTab(list_view_, "Available sources");

  auto splitter = new QSplitter(Qt::Vertical);
  splitter->setChildrenCollapsible(false);
  splitter->setHandleWidth(SPLITTER_HANDLE_WIDTH);
  splitter->addWidget(tree_view_);
  splitter->addWidget(list_container);
  splitter->setStretchFactor(0, 3);
  splitter->setStretchFactor(1, 1);

  auto layout = new QVBoxLayout();

  auto endpoint_layout = new QGridLayout();
  endpoint_layout->addWidget(source_type_lb, 0, 0, Qt::AlignRight);
  endpoint_layout->addWidget(source_type_cb_, 0, 1);
  endpoint_layout->addWidget(hostname_lb, 1, 0, Qt::AlignRight);
  endpoint_layout->addWidget(hostname_le_, 1, 1);
  endpoint_layout->addWidget(port_lb, 2, 0, Qt::AlignRight);
  endpoint_layout->addWidget(port_le_, 2, 1);

  layout->addLayout(endpoint_layout);
  layout->addWidget(splitter);

  setLayout(layout);
}

void dmi::DataSourceWidget::initConnections()
{
  connect(tree_model_, &DataSourceTreeModel::sourceToggled, this, &DataSourceWidget::sourceToggled);
}

std::string dmi::DataSourceWidget::endpoint() const
{
  return "tcp://"
         + hostname_le_->text().toStdString()
         + ":"
         + port_le_->text().toStdString();
}

xfai::DataSourceType dmi::DataSourceWidget::sourceType() const
{
  return static_cast<xfai::DataSourceType>(source_type_cb_->itemData(source_type_cb_->currentIndex()).toInt());
}

void dmi::DataSourceWidget::updateSourceList(const QStringList& srcs)
{
  list_model_->setSourceList(srcs);
}

void dmi::DataSourceWidget::onStart()
{
  source_type_cb_->setEnabled(false);
  hostname_le_->setEnabled(false);
  port_le_->setEnabled(false);
}

void dmi::DataSourceWidget::onStop()
{
  source_type_cb_->setEnabled(true);
  hostname_le_->setEnabled(true);
  port_le_->setEnabled(true);
}