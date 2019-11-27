/*
    Copyright (c) 2019, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef XFAI_AREA_DETECTOR_H
#define XFAI_AREA_DETECTOR_H

#include <type_traits>

#include "xfai_config.hpp"


namespace xfai
{

enum class ImageDataType
{
  raw = 0x00, // raw data
  cal = 0x01, // calibrated data
};

/**
 * ImageDetector base class.
 *
 * @tparam N: number of modules.
 * @tparam W: module width.
 * @tparam H: module height.
 * @tparam S: source type (calibrated or raw).
 * @tparam D: derived detector type.
 */
template<std::size_t N, std::size_t W, std::size_t H, ImageDataType S, typename D>
class ImageDetector
{
public:
  static constexpr std::size_t n_modules = N;
  static constexpr std::size_t width = W;
  static constexpr std::size_t height = H;
  static constexpr ImageDataType data_type = S;
  using value_type = typename std::conditional<S == ImageDataType::raw, uint16_t, float>::type;
  static constexpr int mat_type = S == ImageDataType::raw ? CV_16UC1 : CV_32FC1; // not used for 1D detector
  static constexpr int display_type = CV_8UC1;

protected:

  std::vector<cv::Mat> orig_; // original image data
  std::vector<cv::Mat> proc_; // processed image data

public:

  ImageDetector()
  {
    for (size_t i = 0; i < n_modules; ++i)
    {
      orig_.emplace_back(cv::Mat(height, width, mat_type, cv::Scalar(0)));
      proc_.emplace_back(cv::Mat(height, width, display_type, cv::Scalar(0)));
    }
  }

  ~ImageDetector() = default;

  /**
   * Return the assembled image data for multi-module detectors. It returns
   * a copy of the image data for single-module detectors.
   */
  cv::Mat assembled()
  {
    return static_cast<D*>(this)->assembleModules();
  }

  void update(const std::vector<void*>& data)
  {
    if (data.size() != n_modules)
      throw std::invalid_argument("Source size is different from the number of modules!");

    for (size_t i = 0; i < n_modules; ++i)
    {
      if (data[i] != nullptr) {
        auto module = cv::Mat(height, width, mat_type, data[i]);
        module.copyTo(orig_[i]);
      } else {
        orig_[i].setTo(cv::Scalar(0));
      }
    }
  }

  /**
   * Process the original image data and store the processed data.
   *
   * @param threshold_range: (min, max) of the threshold-to-zero range.
   */
  void process(const std::pair<double, double>& threshold_range)
  {
    cv::Mat thresh;
    for (size_t i=0; i < n_modules; ++i)
    {
      cv::threshold(orig_[i], thresh, threshold_range.first, 0, cv::THRESH_TOZERO);
      cv::threshold(thresh, thresh, threshold_range.second, 0, cv::THRESH_TOZERO_INV);
      cv::normalize(thresh, proc_[i], 0., 255., cv::NORM_MINMAX, display_type);
    }
  }

};

/**
 * JungFrau-1M detector.
 */
template<ImageDataType S>
class JungFrau1M : public ImageDetector<1, 1024, 512, S, JungFrau1M<S>>
{
  friend ImageDetector<1, 1024, 512, S, JungFrau1M<S>>;

  cv::Mat assembleModules()
  {
    return this->proc_[0];
  }
public:
  ~JungFrau1M() = default;

  JungFrau1M() = default;
};

/**
 * DSSC-1M detector consists of 16 modules of 128×512 pixels each. Each
 * module is further subdivided into 2 sensor tiles.
 *
 */
template<ImageDataType S>
class DSSC1M : public ImageDetector<16, 512, 128, S, DSSC1M<S>>
{
  void assembleQuad1(cv::Mat& assembled)
  {
    // each quadrant contains four modules
    for (int m = 0; m < 4; ++m)
    {
      cv::Mat roi = assembled(cv::Rect(0,
                                       this->height * (4 + m),
                                       this->width,
                                       this->height));
      cv::Mat flipped;
      cv::flip(this->proc_[m], flipped, 1); // flipping around y axis
      flipped.copyTo(roi);
    }
  }

  void assembleQuad2(cv::Mat& assembled)
  {
    for (int m = 0; m < 4; ++m)
    {
      cv::Mat roi = assembled(cv::Rect(0,
                                       this->height * m,
                                       this->width,
                                       this->height));
      cv::Mat flipped;
      cv::flip(this->proc_[4 + m], flipped, 1); // flipping around y axis
      flipped.copyTo(roi);
    }
  }

  void assembleQuad3(cv::Mat& assembled)
  {
    for (int m = 0; m < 4; ++m)
    {
      cv::Mat roi = assembled(cv::Rect(this->width,
                                       this->height * (3 - m),
                                       this->width,
                                       this->height));
      cv::Mat flipped;
      cv::flip(this->proc_[8 + m], flipped, 0); // flipping around x axis
      flipped.copyTo(roi);
    }
  }

  void assembleQuad4(cv::Mat& assembled)
  {
    for (int m = 0; m < 4; ++m)
    {
      cv::Mat roi = assembled(cv::Rect(this->width,
                                       this->height * (7 - m),
                                       this->width,
                                       this->height));
      cv::Mat flipped;
      cv::flip(this->proc_[12 + m], flipped, 0); // flipping around x axis
      flipped.copyTo(roi);
    }
  }

  cv::Mat assembleModules()
  {
    cv::Mat assembled(1024, 1024, this->display_type, cv::Scalar(0));

    // loop over quadrants
    assembleQuad1(assembled);
    assembleQuad2(assembled);
    assembleQuad3(assembled);
    assembleQuad4(assembled);

    return assembled;
  }

  friend ImageDetector<16, 512, 128, S, DSSC1M<S>>;

public:

  ~DSSC1M() = default;

  DSSC1M() = default;

};

/**
 * LPD-1M detector consists of 16 supermodules of 256×256 pixels each.
 * Each supermodule is further subdivided into 16 sensor tiles.
 */
template<ImageDataType S>
class LPD1M : public ImageDetector<16, 256, 256, S, LPD1M<S>>
{
  void assembleQuad1(cv::Mat& assembled)
  {
    // each quadrant contains four modules
    for (int m = 0; m < 4; ++m)
    {
      cv::Mat roi = assembled(cv::Rect(this->width * (m/2),
                                       this->height * (2 + (m ==0 || m == 3)),
                                       this->width,
                                       this->height));
      this->proc_[m].copyTo(roi);
    }
  }

  void assembleQuad2(cv::Mat& assembled)
  {
    for (int m = 0; m < 4; ++m)
    {
      cv::Mat roi = assembled(cv::Rect(this->width * (m/2),
                                       this->height * (m ==0 || m == 3),
                                       this->width,
                                       this->height));
      this->proc_[4 + m].copyTo(roi);
    }
  }

  void assembleQuad3(cv::Mat& assembled)
  {
    for (int m = 0; m < 4; ++m)
    {
      cv::Mat roi = assembled(cv::Rect(this->width * (2 + m/2),
                                       this->height * (m ==0 || m == 3),
                                       this->width,
                                       this->height));
      this->proc_[8 + m].copyTo(roi);
    }
  }

  void assembleQuad4(cv::Mat& assembled)
  {
    for (int m = 0; m < 4; ++m)
    {
      cv::Mat roi = assembled(cv::Rect(this->width * (2 + m/2),
                                       this->height * (2 + (m ==0 || m == 3)),
                                       this->width,
                                       this->height));
      this->proc_[12 + m].copyTo(roi);
    }
  }

  cv::Mat assembleModules()
  {
    cv::Mat assembled(1024, 1024, this->display_type, cv::Scalar(0));

    // loop over quadrants
    assembleQuad1(assembled);
    assembleQuad2(assembled);
    assembleQuad3(assembled);
    assembleQuad4(assembled);

    return assembled;
  }

  friend ImageDetector<16, 256, 256, S, LPD1M<S>>;

public:

  ~LPD1M() = default;

  LPD1M() = default;
};

}; //xfai

#endif //XFAI_AREA_DETECTOR_H
