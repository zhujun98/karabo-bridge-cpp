# VFAI (View - Fast Analysis Interface)

This project aims to provide a visualization tool to help users check whether the 
detector data received from the C++ bridge client is correct.

## Dependencies

#### Install Qt5

```shell script
conda install -c conda-forge qt=5.12.5
```

#### Install TBB

For runtime dynamic linking.

```shell script
conda install -c conda-forge tbb
```

#### Build and install OpenCV with Qt5 support

```shell script
cd thirdparty
git clone --branch 4.1.1 https://github.com/opencv/opencv.git
cd opencv
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release 
      -DWITH_QT=ON 
      -DCMAKE_INSTALL_PREFIX=../../opencv_install/ ../
make -j8
make install
```

In case of *fatal error: Eigen/Core: No such file or directory*,
change the line "# include <Eigen/Core>" to "# include <eigen3/Eigen/Core>" in 
`opencv/modules/core/include/opencv2/core/private.hpp`.

In case of * ... undefined reference to 'TIFFReadRGBAStrip@LIBTIFF_4.0' ... *, add "-DBUILD_TIFF=ON".
