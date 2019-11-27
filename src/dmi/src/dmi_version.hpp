/*
    Copyright (c) 2019, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KBCPP_DMI_VERSION_HPP
#define KBCPP_DMI_VERSION_HPP

#include <string>


std::string getDmiVersion();
unsigned getDmiVersionMajor();
unsigned getDmiVersionMinor();
unsigned getDmiVersionPatch();


#endif //KBCPP_DMI_VERSION_HPP
