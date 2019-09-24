/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#include <QString>
#include <QTreeWidget>
#include <QDebug>
#include <QSet>

#include "vf_treemodel.hpp"


// ********************************************************************
//                          DSFilterRangeDelegate
// ********************************************************************

vf::DSFilterRangeDelegate::DSFilterRangeDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* vf::DSFilterRangeDelegate::createEditor(QWidget* parent,
                                                 const QStyleOptionViewItem& option,
                                                 const QModelIndex& index) const
{
  Q_UNUSED(option)
  Q_UNUSED(index)

  auto le = new QLineEdit(parent);
  return le;
}

void vf::DSFilterRangeDelegate::setEditorData(QWidget* editor,
                                              const QModelIndex& index) const
{
    auto le = qobject_cast<QLineEdit*>(editor);
    const QString value = index.data(Qt::DisplayRole).toString();
    le->setText(value);
}

void vf::DSFilterRangeDelegate::setModelData(QWidget* editor,
                                             QAbstractItemModel* model,
                                             const QModelIndex& index) const
{
  auto le = qobject_cast<QLineEdit*>(editor);
  model->setData(index, le->text(), Qt::EditRole);
}

void vf::DSFilterRangeDelegate::updateEditorGeometry(QWidget* editor,
                                                     const QStyleOptionViewItem& option,
                                                     const QModelIndex& index) const
{
  Q_UNUSED(index)
  editor->setGeometry(option.rect);
}

// ********************************************************************
//                          DSPropertyDelegate
// ********************************************************************

vf::DSPropertyDelegate::DSPropertyDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* vf::DSPropertyDelegate::createEditor(QWidget* parent,
                                              const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const
{
  auto cb = new QComboBox(parent);

  cb->addItems(SourceItem::properties[index.parent().data(Qt::DisplayRole).toString()
               + getOutputChannel(index.sibling(index.row(), 0).data(Qt::DisplayRole).toString())]);
  return cb;
}

void vf::DSPropertyDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cb = qobject_cast<QComboBox*>(editor);
    const QString value = index.data(Qt::EditRole).toString();
    cb->setCurrentText(value);
}

void vf::DSPropertyDelegate::setModelData(QWidget* editor,
                                          QAbstractItemModel* model,
                                          const QModelIndex& index) const
{
  auto cb = qobject_cast<QComboBox*>(editor);
  model->setData(index, cb->currentText(), Qt::EditRole);
}

void vf::DSPropertyDelegate::updateEditorGeometry(QWidget* editor,
                                                  const QStyleOptionViewItem& option,
                                                  const QModelIndex& index) const
{
  Q_UNUSED(index)
  editor->setGeometry(option.rect);
}

// ********************************************************************
//                          DataSourceTreeItem
// ********************************************************************

vf::DataSourceTreeItem::DataSourceTreeItem(const value_type& data,
                                           bool exclusive,
                                           DataSourceTreeItem* parent)
  : item_data_(data),
    exclusive_(exclusive),
    parent_item_(parent),
    rank_([parent]() { return parent == nullptr ? 0 : parent->rank() + 1; }()),
    checked_(false)
{
}

vf::DataSourceTreeItem::~DataSourceTreeItem()
{
  qDeleteAll(child_items_);
}

vf::DataSourceTreeItem* vf::DataSourceTreeItem::child(int number)
{
  return child_items_.value(number);
}

void vf::DataSourceTreeItem::appendChild(DataSourceTreeItem* item)
{
  // TODO: check parent?
  child_items_.append(item);
}

int vf::DataSourceTreeItem::rowCount() const { return child_items_.count(); }

int vf::DataSourceTreeItem::columnCount() const { return item_data_.count(); }

int vf::DataSourceTreeItem::row() const
{
  if (parent_item_)
    return parent_item_->child_items_.indexOf(const_cast<DataSourceTreeItem*>(this));

  return 0;
}

QVariant vf::DataSourceTreeItem::data(int column) const
{
  if (column < 0 || column >= item_data_.size())
    return QVariant();
  return item_data_.at(column);
}

void vf::DataSourceTreeItem::setData(const QVariant& value, int column)
{
  if (column >= 0 && column < item_data_.size())
    item_data_[column] = value;
}


vf::DataSourceTreeItem* vf::DataSourceTreeItem::parent() { return parent_item_; }

int vf::DataSourceTreeItem::rank() const { return rank_; }

bool vf::DataSourceTreeItem::isChecked() const { return checked_; }
void vf::DataSourceTreeItem::setChecked(bool checked) { checked_ = checked; }

bool vf::DataSourceTreeItem::isExclusive() const { return exclusive_; }

// ********************************************************************
//                         DataSourceTreeModel
// ********************************************************************

vf::DataSourceTreeModel::DataSourceTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
  root_item_ = new DataSourceTreeItem({tr("Source name"), tr("Property")});
  setupModelData(SourceItem::categories, root_item_);
}

vf::DataSourceTreeModel::DataSourceTreeModel(const map_type& sources, QObject *parent)
    : QAbstractItemModel(parent)
{
  root_item_ = new DataSourceTreeItem({tr("Source name"), tr("Property")});
  setupModelData(sources, root_item_);
}

vf::DataSourceTreeModel::~DataSourceTreeModel()
{
  delete root_item_;
}

QVariant vf::DataSourceTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) return {};

  auto* item = static_cast<DataSourceTreeItem*>(index.internalPointer());

  if (role == Qt::DisplayRole)
    return item->data(index.column());

  if (role == Qt::CheckStateRole && index.column() == 0 && item->rank() > 1)
  {
    return static_cast<int>(item->isChecked() ? Qt::Checked : Qt::Unchecked);
  }

  return {};
}

bool vf::DataSourceTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  auto* item = static_cast<DataSourceTreeItem*>(index.internalPointer());

  if (role == Qt::CheckStateRole || role == Qt::EditRole)
  {
    if (role == Qt::CheckStateRole)
    {
      if (item->isExclusive() && not item->isChecked())
      {
        // Loop over its siblings and uncheck the checked one.
        for (int i=0; i < index.model()->rowCount(index.parent()); ++i)
        {
          if (i == index.row()) continue;

          auto item_sb = static_cast<DataSourceTreeItem*>(index.sibling(i, 0).internalPointer());
          if (item_sb->isChecked())
          {
            // found the checked sibling and uncheck it
            item_sb->setChecked(false);

            // notify old source being unchecked
            emit sourceToggled(
              {item_sb->parent()->data(0).toString(),
               item_sb->data(0).toString(),
               item_sb->data(1).toString()},
              false
            );

            // update the tree view
            emit dataChanged(index.sibling(i, 0), index.sibling(i, 0));

            // only one sibling is expected to be checked
            break;
          }
        }
      }

      // check the item it is not exclusive
      item->setChecked(value.toBool());

    } else // if (role == Qt::EditRole)
    {
      item->setData(value, index.column());
    }

    // notify new source being checked
    auto src_item = SourceItem(item->parent()->data(0).toString(),
                               item->data(0).toString(),
                               item->data(1).toString());
    emit sourceToggled(src_item, item->isChecked());

    return true;
  }

  return false;
}

Qt::ItemFlags vf::DataSourceTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  auto* item = static_cast<DataSourceTreeItem*>(index.internalPointer());

  // category is not checkable
  if (item->rank() > 1)
  {
    if (index.column() == 0)
    {
      // first column is the source name
      flags |= Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled;
    } else if (item->isChecked() && index.column() >= 1)
    {
      // disable editing if unchecked
      flags |= Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
    }
  }

  return flags;
}

QVariant vf::DataSourceTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    return root_item_->data(section);
  return {};
}

QModelIndex vf::DataSourceTreeModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) return {};

  DataSourceTreeItem* parent_item;

  if (!parent.isValid())
    parent_item = root_item_;
  else
    parent_item = static_cast<DataSourceTreeItem*>(parent.internalPointer());

  DataSourceTreeItem* child_item = parent_item->child(row);
  if (child_item) return createIndex(row, column, child_item);

  return {};
}

QModelIndex vf::DataSourceTreeModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) return {};

  auto* child_item = static_cast<DataSourceTreeItem*>(index.internalPointer());
  DataSourceTreeItem* parent_item = child_item->parent();

  if (parent_item == root_item_) return {};
  return createIndex(parent_item->row(), 0, parent_item);
}

int vf::DataSourceTreeModel::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0) return 0;

  DataSourceTreeItem* parent_item;
  if (!parent.isValid())
    parent_item = root_item_;
  else
    parent_item = static_cast<DataSourceTreeItem*>(parent.internalPointer());

  return parent_item->rowCount();
}

int vf::DataSourceTreeModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return static_cast<DataSourceTreeItem*>(parent.internalPointer())->columnCount();
  return root_item_->columnCount();
}

void vf::DataSourceTreeModel::setupModelData(const map_type& sources, DataSourceTreeItem* parent)
{
  auto it = sources.cbegin();
  while (it != sources.end())
  {
    // category
    QString ctg = it.key();
    auto ctg_item = new DataSourceTreeItem({ctg, {}}, false, parent);
    parent->appendChild(ctg_item);

    bool exclusive = false;
    if (SourceItem::exclusive_categories.find(ctg) != SourceItem::exclusive_categories.end())
      exclusive = true;

    // loop over sources in the category
    auto v = it.value();
    for(int i = 0; i < v.size(); ++i)
    {
      auto device_item = new DataSourceTreeItem(
        {v[i], SourceItem::properties[ctg + getOutputChannel(v[i])][0]}, exclusive, ctg_item);
      ctg_item->appendChild(device_item);
    }

    ++it;
  }
}
