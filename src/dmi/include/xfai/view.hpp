/*
    Copyright (c) 2019, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/
#ifndef XFAI_VIEW_H
#define XFAI_VIEW_H


#include <opencv2/core.hpp>


template<typename T>
class VfView
{

  VfView() = default;

  cv::Mat m_;

public:

  ~VfView() = default;

  template <typename ...Args>
  VfView create(Args&& ... args)
  {
    m_.create(std::forward<Args>(args) ... );
  }

};


#endif //XFAI_VIEW_H
