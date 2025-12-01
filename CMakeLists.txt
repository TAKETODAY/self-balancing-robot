# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# In this example, sdkconfig file is placed into the build directory.
# This allows building development and production configs side by side,
# without having them influence each other.
set(SDKCONFIG "${CMAKE_BINARY_DIR}/sdkconfig")

set(COMPONENTS cxx src) # 设置 组件
set(EXTRA_COMPONENT_DIRS src) # 组件重命名


include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
#idf_build_set_property(MINIMAL_BUILD ON)

project(self-balancing-robot)

