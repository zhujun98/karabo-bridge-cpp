/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#include <QCheckBox>

#include "vf_delegate.hpp"


vf::CheckBoxDelegate::CheckBoxDelegate(QObject* parent) : QStyledItemDelegate(parent)
{

}

QWidget* vf::CheckBoxDelegate::createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
  auto* editor = new QCheckBox(parent);

  return editor;
}

void vf::CheckBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  auto check_box = static_cast<QCheckBox*>(editor);
  check_box->setChecked(index.model()->data(index, Qt::EditRole).toBool());
}

void vf::CheckBoxDelegate::setModelData(QWidget* editor,
                                        QAbstractItemModel* model,
                                        const QModelIndex& index) const
{
  auto* check_box = static_cast<QCheckBox*>(editor);
  int value = check_box->checkState() == Qt::Checked ? 1 : 0;
  model->setData(index, value, Qt::EditRole);
}

void vf::CheckBoxDelegate::updateEditorGeometry(QWidget* editor,
                                                const QStyleOptionViewItem& option,
                                                const QModelIndex& index) const
{
  editor->setGeometry(option.rect);
}