/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_VF_LISTMODEL_HPP
#define KARABO_BRIDGE_VF_LISTMODEL_HPP

#include <QAbstractListModel>
#include <QStringList>

namespace vf
{

/**
 * Used for displaying the device IDs found in the received data.
 */

// TODO: provide more customized visualization. For example, when a new source
//       appears or an old source disappeared.

class DataSourceListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  explicit DataSourceListModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent) const override;

  QVariant data(const QModelIndex& index, int role) const override;

  void setSourceList(const QStringList& devices);

private:

  QStringList devices_;
};

} // vf

#endif //KARABO_BRIDGE_VF_LISTMODEL_HPP
