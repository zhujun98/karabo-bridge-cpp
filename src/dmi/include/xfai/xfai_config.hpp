/*
    Copyright (c) 2019, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef XFAI_CONFIG_HPP
#define XFAI_CONFIG_HPP

#ifndef XFAI_DEFAULT_SOURCE_TYPE
#define XFAI_DEFAULT_SOURCE_TYPE ::dmi::ImageDataType::cal
#endif

namespace xfai
{

enum class DataSourceType
{
  zmq = 0x00, // real-time data via ZeroMQ bridge
  file = 0x01, // data streamed from files
};

} //xfai

#endif //XFAI_CONFIG_HPP
