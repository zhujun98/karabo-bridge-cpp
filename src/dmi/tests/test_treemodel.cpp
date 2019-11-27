/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#include <iostream>
#include <future>

#include <QtTest/QtTest>
#include <Qt>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "gui/datasource_model.hpp"
#include "pipeline/sourceitem.hpp"


namespace dmi
{

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;

/*
 * test cases
 */

TEST(TestDataSourceTreeModel, TestDataSourceTreeItem)
{
  DataSourceTreeItem item {{"tree"}};
}

class DataSourceTreeModelTestData : public ::testing::Test {

protected:

  void SetUp() override {
    // it is sorted by key
    categories_["Motor"] = dmi::SourceItem::categories["Motor"];
    categories_["XGM"] = dmi::SourceItem::categories["XGM"];
    categories_["JungFrau"] = dmi::SourceItem::categories["JungFrau"];
  }

  dmi::SourceItem::map_type categories_;
};

TEST_F(DataSourceTreeModelTestData, TestModel)
{
  DataSourceTreeModel model(categories_);

  auto root_idx0 = model.index(0, 0, QModelIndex());
  EXPECT_EQ(QString("JungFrau"), model.data(root_idx0, Qt::DisplayRole).toString());
  EXPECT_EQ(categories_["JungFrau"][0], model.data(model.index(0, 0, root_idx0), Qt::DisplayRole).toString());
  EXPECT_EQ(categories_["JungFrau"][3], model.data(model.index(3, 0, root_idx0), Qt::DisplayRole).toString());

  auto root_idx1 = model.index(1, 0, QModelIndex());
  EXPECT_EQ(QString("Motor"), model.data(root_idx1, Qt::DisplayRole).toString());
  EXPECT_EQ(categories_["Motor"][0], model.data(model.index(0, 0, root_idx1), Qt::DisplayRole).toString());

  auto root_idx2 = model.index(2, 0, QModelIndex());
  EXPECT_EQ(QString("XGM"), model.data(root_idx2, Qt::DisplayRole).toString());
  EXPECT_EQ(categories_["XGM"][0], model.data(model.index(0, 0, root_idx2), Qt::DisplayRole).toString());
  EXPECT_EQ(categories_["XGM"][1], model.data(model.index(1, 0, root_idx2), Qt::DisplayRole).toString());

  // test out of bound return ""
  EXPECT_EQ(QString(""), model.data(model.index(3, 0, QModelIndex()), Qt::DisplayRole).toString());
}

} //dmi