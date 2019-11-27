/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#ifndef KBCPP_DMI_TREEMODEL_HPP
#define KBCPP_DMI_TREEMODEL_HPP

#include <map>

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QComboBox>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QVector>

#include "pipeline/sourceitem.hpp"


namespace dmi
{

inline QString getOutputChannel(const QString& src_name)
{
  QString chn;

  auto idx = src_name.indexOf(':');
  if (idx > 0) chn  = src_name.right(src_name.size() - idx);
  return chn;
}


class DSFilterRangeDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  explicit DSFilterRangeDelegate(QObject *parent = nullptr);

  ~DSFilterRangeDelegate() override = default;

  QWidget *createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override;

  void updateEditorGeometry(QWidget* editor,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override;
};


class DSPropertyDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  explicit DSPropertyDelegate(QObject *parent = nullptr);

  ~DSPropertyDelegate() override = default;

  QWidget *createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override;

  void updateEditorGeometry(QWidget* editor,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override;
};


class DataSourceTreeItem
{

public:

  using value_type = QVector<QVariant>;

  explicit DataSourceTreeItem(const value_type& data,
                              bool exclusive = false,
                              DataSourceTreeItem* parent = nullptr);

  virtual ~DataSourceTreeItem();

  // Return the child at an index position.
  //
  // If the index is out of bounds, the function returns a default-construct
  // value, which is a nullptr in this case.
  DataSourceTreeItem* child(int number);

  // Append a child item.
  void appendChild(DataSourceTreeItem* item);

  // Return the number of rows (children) associated with this item.
  int rowCount() const;

  // Return the number of columns associated with this item.
  int columnCount() const;

  // Determine the index of child in its parents' list of children.
  int row() const;

  // Get data by column index.
  QVariant data(int column) const;

  void setData(const QVariant& value, int column);

  // Return the parent item.
  DataSourceTreeItem* parent();

  int rank() const;

  bool isChecked() const;
  void setChecked(bool checked);

  bool isExclusive() const;

private:

  QList<DataSourceTreeItem*> child_items_;
  value_type item_data_;

  bool exclusive_; // whether the item is an exclusive child of its parent
  DataSourceTreeItem* parent_item_;

  int rank_; // rank of the item in the tree
  bool checked_;
};


class DataSourceTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  using map_type = SourceItem::map_type;

  explicit DataSourceTreeModel(QObject* parent = nullptr);

  explicit DataSourceTreeModel(const map_type& sources, QObject* parent = nullptr);

  ~DataSourceTreeModel() override;

  // Returns the data stored under the given role for the item referred to by the index.
  QVariant data(const QModelIndex& index, int role) const override;

  // Sets the role data for the item at index to value.
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

  // Returns the item flags for the given index.
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  // Returns the data for the given role and section in the header with the
  // specified orientation.
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  // Returns the index of the item in the model specified by the given row,
  // column and parent index.
  QModelIndex index(int row, int column, const QModelIndex& parent) const override;

  QModelIndex parent(const QModelIndex& index) const override;

  // Returns the number of rows (children) under the given parent.
  int rowCount(const QModelIndex& parent) const override;

  // Returns the number of columns for the children of the given parent.
  int columnCount(const QModelIndex& parent) const override;

  void setupModelData(const map_type& sources, DataSourceTreeItem *parent);

signals:
  // emitted after the check state of a data source changed
  void sourceToggled(const SourceItem& item, bool checked);
  // emitted after the check states of data source modules changed
  void moduleSourcesToggled(const QVector<SourceItem>& items, bool checked);

private:
  DataSourceTreeItem* root_item_;
};


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


class CheckBoxDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  explicit CheckBoxDelegate(QObject *parent = nullptr);

  ~CheckBoxDelegate() override = default;

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

} //dmi


#endif //KBCPP_DMI_TREEMODEL_HPP
