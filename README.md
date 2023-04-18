## DLP-ALC-LIGHTCRAFTER-SDK for Ubuntu

This project is based on the officially released code of TI [1], with some configurations and bugs modified to be suitable for Linux platform.

##### Tested environment:

- Ubuntu 16.04
- OpenCV 3.4.11
- CMake 3.21.4
- TI DLP lightcrafter 4500 EVM

(Unlike most DLP 4500 related projects, this project does not require OpenGL or QT to project customized images)

##### Added Demos:

- examples/projector_demo_sequence.cpp
  It utilizes pattern sequence mode to project sequence with customized configurations.
- examples/projector_demo_video.cpp
  It streams video from HDMI interface via pattern sequence mode. The output trigger signal has been validated to be functional. To test your own camera, you can add code starting with line 318.

##### Please note:

1. Given that the demos work in pattern sequence mode, it is necessary to adjust the output resolution of the projector in computer settings to 912x1140 before running.

2. Do not change the configurations in projector_demo_video.cpp. Otherwise, configuration validation errors or image flickering may occur.

3. Please change file path to the absolute path of the file on your computer before running the code (Including the image paths and proj_param_file path).

4. All modified parts are marked with "new added" comments.

5. This project fixes USB connection and image initialization bugs on Ubuntu, it should also be functional on other platforms (Windows, MacOS and other Linux platforms).

6. The current code has only been tested on DLP 4500, configurations for other projectors are commented out in the CMakeLists.txt. If you want to try other devices, please modify CMakeLists.txt first.

7. Set up and compile steps are the same as the official code.

##### References:

[1]https://www.ti.com.cn/tool/zh-cn/DLP-ALC-LIGHTCRAFTER-SDK?keyMatch=&tisearch=search-everything&usecase=software

[2]https://www.ti.com/lit/ug/dlpu010g/dlpu010g.pdf

[3]https://www.ti.com/lit/ug/dlpu011f/dlpu011f.pdf

[4]https://github.com/alexandre-bernier/coro_eyes_sdk

[5]https://e2e.ti.com/support/dlp-products-group/dlp/f/dlp-products-forum/635435/linux-dlp4500-sdk-generates-wrong-patterns-on-ubuntu-16-04-64bit

[6]https://github.com/libusb/hidapi/issues/58

[7]https://www.ti2k.com/35471.html



