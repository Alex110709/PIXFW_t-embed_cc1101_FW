# LilyGO T-Embed CC1101 JavaScript App Firmware
cmake_minimum_required(VERSION 3.16)

# 프로젝트 설정
set(COMPONENTS "main" CACHE STRING "List of components to build")
set(EXTRA_COMPONENT_DIRS components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(t-embed-cc1101-fw)

# ESP32-S3 타겟 설정
set(IDF_TARGET esp32s3)