/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#include <QDebug>

#include "vf_listmodel.hpp"


vf::DataSourceListModel::DataSourceListModel(QObject* parent)
  : QAbstractListModel(parent)
{
}

int vf::DataSourceListModel::rowCount(const QModelIndex& parent) const
{
  return devices_.count();
}

QVariant vf::DataSourceListModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() >= devices_.count()) return {};

  if (role == Qt::DisplayRole) return devices_.at(index.row());

  return {};
}

void vf::DataSourceListModel::setSourceList(const QStringList& devices)
{
  if (devices != devices_)
  {
    beginResetModel();
    devices_ = devices;
    endResetModel();
  }
}
