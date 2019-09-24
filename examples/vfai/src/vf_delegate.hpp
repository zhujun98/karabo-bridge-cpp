/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_VF_DELEGATE_H
#define KARABO_BRIDGE_VF_DELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>


namespace vf
{

class CheckBoxDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  CheckBoxDelegate(QObject *parent = nullptr);

  ~CheckBoxDelegate() = default;

  QWidget *createEditor(QWidget *parent,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index) const override;

  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  void setModelData(QWidget *editor,
                    QAbstractItemModel *model,
                    const QModelIndex &index) const override;

  void updateEditorGeometry(QWidget *editor,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const override;
};

} // vf

#endif //KARABO_BRIDGE_VF_DELEGATE_H
