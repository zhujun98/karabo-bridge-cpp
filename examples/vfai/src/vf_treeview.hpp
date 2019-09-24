/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_VF_TREEVIEW_HPP
#define KARABO_BRIDGE_VF_TREEVIEW_HPP

#include <QTreeView>

#include <QStandardItem>


namespace vf
{

class DeviceTreeView : public QTreeView
{
  Q_OBJECT

public:

  explicit DeviceTreeView(QWidget* parent = nullptr);

  ~DeviceTreeView() override = default;
};

}

#endif //KARABO_BRIDGE_VF_TREEVIEW_HPP
