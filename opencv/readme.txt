*** Notice for ImagingTool solution build ***

If you want to build ImagingTool solution, place the OpenCV installation set this directory.

The corrshd.exe depends on following static library files.
----------------------------+----------------------------
    On Release build        |   On Debug build
----------------------------+----------------------------
    ippicvmt.lib            |   (Same as left)
    IlmImf.lib              |   IlmImfd.lib
    ippiw.lib               |   ippiwd.lib
    ittnotify.lib           |   ittnotifyd.lib
    libjpeg-turbo.lib       |   libjpeg-turbod.lib
    libopenjp2.lib          |   libopenjp2d.lib
    libpng.lib              |   libpngd.lib
    libtiff.lib             |   libtiffd.lib
    libwebp.lib             |   libwebpd.lib
    opencv_core480.lib      |   opencv_core480d.lib
    opencv_highgui480.lib   |   opencv_highgui480d.lib
    opencv_imgcodecs480.lib |   opencv_imgcodecs480d.lib
    opencv_imgproc480.lib   |   opencv_imgproc480d.lib
    zlib.lib                |   zlibd.lib
----------------------------+----------------------------

The expected file placement is as follows:

$(SolutionDir)opencv
|   readme.txt                          // This file
|
\---install
    |   LICENSE
    |   OpenCVConfig-version.cmake
    |   OpenCVConfig.cmake
    |   setup_vars_opencv4.cmd
    |
    +---include
    |   \---opencv2
    |       |   calib3d.hpp
    |       |   core.hpp
    |       |   cvconfig.h
    |       |   ...
    |       |
    |       +---calib3d
    |       |       ...
    |       |
    |       ... (OpenCV header files and sub directories) ...
    |
    \---x64
        \---vc16
            \---staticlib
                    IlmImf.lib
                    IlmImfd.lib
                    ippicvmt.lib
                    ippiw.lib
                    ippiwd.lib
                    ittnotify.lib
                    ittnotifyd.lib
                    libjpeg-turbo.lib
                    libjpeg-turbod.lib
                    libopenjp2.lib
                    libopenjp2d.lib
                    libpng.lib
                    libpngd.lib
                    libtiff.lib
                    libtiffd.lib
                    libwebp.lib
                    libwebpd.lib
                    OpenCVConfig-version.cmake
                    OpenCVConfig.cmake
                    OpenCVModules-debug.cmake
                    OpenCVModules-release.cmake
                    OpenCVModules.cmake
                    opencv_core480.lib
                    opencv_core480d.lib
                    opencv_highgui480.lib
                    opencv_highgui480d.lib
                    opencv_imgcodecs480.lib
                    opencv_imgcodecs480d.lib
                    opencv_imgproc480.lib
                    opencv_imgproc480d.lib
                    zlib.lib
                    zlibd.lib
                    ...

---
readme.txt [END]
