/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_VF_PIPELINE_DATA_HPP
#define KARABO_BRIDGE_VF_PIPELINE_DATA_HPP

#include <iostream>
#include <string>

#include <QDebug>


namespace vf
{

struct MetaData
{
  MetaData() : tid(0) {}
  explicit MetaData(std::size_t tid) : tid(tid) {}
  std::size_t tid;
  std::string source_category;
  std::string source_name;
};

inline QDebug operator<<(QDebug debug, const MetaData &data)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "MetaData(" << "tid=" << data.tid
                                 << ", source_category=" << data.source_category.c_str()
                                 << ", source_name=" << data.source_name.c_str() << ")";
  return debug;
}

using PipeLineData = std::vector<void*>;
using PipeLineQueue = tbb::concurrent_bounded_queue<std::pair<MetaData, PipeLineData>>;

} // vf





#endif //KARABO_BRIDGE_VF_PIPELINE_DATA_HPP
