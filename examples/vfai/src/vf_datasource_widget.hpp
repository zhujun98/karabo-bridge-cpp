/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_VF_DATASOURCE_WIDGET_H
#define KARABO_BRIDGE_VF_DATASOURCE_WIDGET_H

#include <QString>
#include <QLineEdit>
#include <QListView>
#include <QComboBox>
#include <QGroupBox>

#include "vf_treeview.hpp"
#include "vf_treemodel.hpp"
#include "vf_listmodel.hpp"
#include "vf_delegate.hpp"
#include "vfai/vf_config.hpp"


namespace vf
{

class DataSourceWidget : public QWidget
{
  Q_OBJECT

public:
  explicit DataSourceWidget(QWidget* parent = nullptr);

  ~DataSourceWidget() override = default;

  std::string endpoint() const;

  DataSourceType sourceType() const;

  void updateSourceList(const QStringList& srcs);

  static constexpr int SPLITTER_HANDLE_WIDTH = 9;
  static constexpr char HOSTNAME[] = "localhost";
  static constexpr char PORT[] = "45454";

private:
  void initUI();
  void initConnections();

public slots:
  void onStart();
  void onStop();

signals:
  // forwarded by DataSourceTreeModel::sourceToggled
  void sourceToggled(const SourceItem& item, bool checked);

private:
  QComboBox* source_type_cb_;
  QLineEdit* hostname_le_;
  QLineEdit* port_le_;

  DSPropertyDelegate* ppt_delegate_;
//  DSFilterRangeDelegate* fr_delegate_;
  DeviceTreeView* tree_view_;
  DataSourceTreeModel* tree_model_;

  QListView* list_view_;
  DataSourceListModel* list_model_;
};

} //vf

#endif //KARABO_BRIDGE_VF_DATASOURCE_WIDGET_H