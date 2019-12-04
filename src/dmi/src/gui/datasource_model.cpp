/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#include <QCheckBox>
#include <QString>
#include <QTreeWidget>
#include <QDebug>
#include <QSet>

#include "datasource_model.hpp"


/**
 * DSPropertyDelegate
 */

dmi::DSPropertyDelegate::DSPropertyDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* dmi::DSPropertyDelegate::createEditor(QWidget* parent,
                                               const QStyleOptionViewItem& option,
                                               const QModelIndex& index) const
{
  auto cb = new QComboBox(parent);

  cb->addItems(SourceItem::properties[index.parent().data(Qt::DisplayRole).toString()
               + getOutputChannel(index.sibling(index.row(), 0).data(Qt::DisplayRole).toString())]);
  return cb;
}

void dmi::DSPropertyDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cb = qobject_cast<QComboBox*>(editor);
    const QString value = index.data(Qt::EditRole).toString();
    cb->setCurrentText(value);
}

void dmi::DSPropertyDelegate::setModelData(QWidget* editor,
                                           QAbstractItemModel* model,
                                           const QModelIndex& index) const
{
  auto cb = qobject_cast<QComboBox*>(editor);
  model->setData(index, cb->currentText(), Qt::EditRole);
}

void dmi::DSPropertyDelegate::updateEditorGeometry(QWidget* editor,
                                                   const QStyleOptionViewItem& option,
                                                   const QModelIndex& index) const
{
  Q_UNUSED(index)
  editor->setGeometry(option.rect);
}

/**
 * DSVRangeDelegate
 */

dmi::DSVRangeDelegate::DSVRangeDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QWidget* dmi::DSVRangeDelegate::createEditor(QWidget* parent,
                                             const QStyleOptionViewItem& option,
                                             const QModelIndex& index) const
{
  Q_UNUSED(option)
  Q_UNUSED(index)

  auto le = new QLineEdit(parent);
  return le;
}

void dmi::DSVRangeDelegate::setEditorData(QWidget* editor,
                                          const QModelIndex& index) const
{
    auto le = qobject_cast<QLineEdit*>(editor);
    const QString value = index.data(Qt::DisplayRole).toString();
    le->setText(value);
}

void dmi::DSVRangeDelegate::setModelData(QWidget* editor,
                                         QAbstractItemModel* model,
                                         const QModelIndex& index) const
{
  auto le = qobject_cast<QLineEdit*>(editor);
  model->setData(index, le->text(), Qt::EditRole);
}

void dmi::DSVRangeDelegate::updateEditorGeometry(QWidget* editor,
                                                 const QStyleOptionViewItem& option,
                                                 const QModelIndex& index) const
{
  Q_UNUSED(index)
  editor->setGeometry(option.rect);
}


/**
 * DataSourceTreeItem
 */

dmi::DataSourceTreeItem::DataSourceTreeItem(const value_type& data,
                                            bool exclusive,
                                            DataSourceTreeItem* parent)
  : item_data_(data),
    exclusive_(exclusive),
    parent_item_(parent),
    rank_([parent]() { return parent == nullptr ? 0 : parent->rank() + 1; }()),
    checked_(false)
{
}

dmi::DataSourceTreeItem::~DataSourceTreeItem()
{
  qDeleteAll(child_items_);
}

dmi::DataSourceTreeItem* dmi::DataSourceTreeItem::child(int number)
{
  return child_items_.value(number);
}

void dmi::DataSourceTreeItem::appendChild(DataSourceTreeItem* item)
{
  // TODO: check parent?
  child_items_.append(item);
}

int dmi::DataSourceTreeItem::rowCount() const { return child_items_.count(); }

int dmi::DataSourceTreeItem::columnCount() const { return item_data_.count(); }

int dmi::DataSourceTreeItem::row() const
{
  if (parent_item_)
    return parent_item_->child_items_.indexOf(const_cast<DataSourceTreeItem*>(this));

  return 0;
}

QVariant dmi::DataSourceTreeItem::data(int column) const
{
  if (column < 0 || column >= item_data_.size())
    return QVariant();
  return item_data_.at(column);
}

void dmi::DataSourceTreeItem::setData(const QVariant& value, int column)
{
  if (column >= 0 && column < item_data_.size())
    item_data_[column] = value;
}


dmi::DataSourceTreeItem* dmi::DataSourceTreeItem::parent() { return parent_item_; }

int dmi::DataSourceTreeItem::rank() const { return rank_; }

bool dmi::DataSourceTreeItem::isChecked() const { return checked_; }
void dmi::DataSourceTreeItem::setChecked(bool checked) { checked_ = checked; }

bool dmi::DataSourceTreeItem::isExclusive() const { return exclusive_; }


/**
 * DataSourceTreeModel
 */

dmi::DataSourceTreeModel::DataSourceTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
  root_item_ = new DataSourceTreeItem({tr("Source name"),
                                       tr("Property"),
                                       tr("Pulse slicer"),
                                       tr("Value range")});
  setupModelData(SourceItem::categories, root_item_);
}

dmi::DataSourceTreeModel::DataSourceTreeModel(const map_type& sources, QObject *parent)
    : QAbstractItemModel(parent)
{
  root_item_ = new DataSourceTreeItem({tr("Source name"),
                                       tr("Property"),
                                       tr("Pulse slicer"),
                                       tr("Value range")});
  setupModelData(sources, root_item_);
}

dmi::DataSourceTreeModel::~DataSourceTreeModel()
{
  delete root_item_;
}

QVariant dmi::DataSourceTreeModel::data(const QModelIndex& index, int role) const
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

bool dmi::DataSourceTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
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
               item_sb->data(1).toString(),
               item_sb->data(2).toString(),
               item_sb->data(3).toString()}, false
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
      auto old_ppt = item->data(1).toString();
      auto old_slicer = item->data(2).toString();
      auto old_vrange = item->data(3).toString();
      item->setData(value, index.column());

      if (index.column() >= 1)
      {
        // remove registered item with the old property
        emit sourceToggled(
          {item->parent()->data(0).toString(),
           item->data(0).toString(),
           old_ppt,
           old_slicer,
           old_vrange}, false
        );
      }
    }

    // notify new source being checked
    emit sourceToggled({item->parent()->data(0).toString(),
                        item->data(0).toString(),
                        item->data(1).toString(),
                        item->data(2).toString(),
                        item->data(3).toString()}, item->isChecked());

    return true;
  }

  return false;
}

Qt::ItemFlags dmi::DataSourceTreeModel::flags(const QModelIndex& index) const
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

QVariant dmi::DataSourceTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    return root_item_->data(section);
  return {};
}

QModelIndex dmi::DataSourceTreeModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex dmi::DataSourceTreeModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) return {};

  auto* child_item = static_cast<DataSourceTreeItem*>(index.internalPointer());
  DataSourceTreeItem* parent_item = child_item->parent();

  if (parent_item == root_item_) return {};
  return createIndex(parent_item->row(), 0, parent_item);
}

int dmi::DataSourceTreeModel::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0) return 0;

  DataSourceTreeItem* parent_item;
  if (!parent.isValid())
    parent_item = root_item_;
  else
    parent_item = static_cast<DataSourceTreeItem*>(parent.internalPointer());

  return parent_item->rowCount();
}

int dmi::DataSourceTreeModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return static_cast<DataSourceTreeItem*>(parent.internalPointer())->columnCount();
  return root_item_->columnCount();
}

void dmi::DataSourceTreeModel::setupModelData(const map_type& sources, DataSourceTreeItem* parent)
{
  auto it = sources.cbegin();
  while (it != sources.end())
  {
    // category
    QString ctg = it.key();
    auto ctg_item = new DataSourceTreeItem({ctg, {}, {}, {}}, false, parent);
    parent->appendChild(ctg_item);

    bool exclusive = false;
    if (SourceItem::exclusive_categories.find(ctg) != SourceItem::exclusive_categories.end())
      exclusive = true;

    // loop over sources in the category
    auto v = it.value();
    for(int i = 0; i < v.size(); ++i)
    {
      auto device_item = new DataSourceTreeItem(
        {v[i], SourceItem::properties[ctg + getOutputChannel(v[i])][0], ":", "-inf, inf"}, exclusive, ctg_item);
      ctg_item->appendChild(device_item);
    }

    ++it;
  }
}


/**
 * DataSourceListModel
 */

dmi::DataSourceListModel::DataSourceListModel(QObject* parent)
  : QAbstractListModel(parent)
{
}

int dmi::DataSourceListModel::rowCount(const QModelIndex& parent) const
{
  return devices_.count();
}

QVariant dmi::DataSourceListModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() >= devices_.count()) return {};

  if (role == Qt::DisplayRole) return devices_.at(index.row());

  return {};
}

void dmi::DataSourceListModel::setSourceList(const QStringList& devices)
{
  if (devices != devices_)
  {
    beginResetModel();
    devices_ = devices;
    endResetModel();
  }
}
