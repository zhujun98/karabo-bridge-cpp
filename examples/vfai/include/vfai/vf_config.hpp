/*
    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_VF_CONFIG_HPP
#define KARABO_BRIDGE_VF_CONFIG_HPP

#ifndef VFAI_DEFAULT_SOURCE_TYPE
#define VFAI_DEFAULT_SOURCE_TYPE ::vf::ImageDataType::cal
#endif

namespace vf
{

enum class DataSourceType
{
  zmq = 0x00, // real-time data via ZeroMQ bridge
  file = 0x01, // data streamed from files
};

} // vf

#endif //KARABO_BRIDGE_VF_CONFIG_HPP
