/*
    Copyright (c) 2019, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef XFAI_LINE_DETECTOR_H
#define XFAI_LINE_DETECTOR_H

namespace xfai
{
  /**
   * LineDetector base class.
   *
   * @tparam C: number of channels.
   * @tparam L: channel length.
   * @tparam D: derived detector type.
   */
  template<std::size_t C, std::size_t L, typename D>
  class LineDetector
  {
    public:
      static constexpr std::size_t n_channels = C;
      static constexpr std::size_t length = L;
      using value_type = float;
  };


  class AdqDigitizer : LineDetector<4, 100000, AdqDigitizer>
  {
  };


};

#endif //XFAI_LINE_DETECTOR_H
