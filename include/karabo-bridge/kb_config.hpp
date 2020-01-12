/*
    Karabo bridge client.

    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_KB_CONFIG_H
#define KARABO_BRIDGE_KB_CONFIG_H

#ifdef __GNUC__
#define DEPRECATED __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#define DEPRECATED
#pragma message("DEPRECATED is not defined for this compiler")
#endif

#define KARABO_BRIDGE_VERSION_MAJOR 0
#define KARABO_BRIDGE_VERSION_MINOR 2
#define KARABO_BRIDGE_VERSION_PATCH 0

#endif //KARABO_BRIDGE_KB_CONFIG_H
