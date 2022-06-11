cmake_minimum_required(VERSION 3.0)
project(mprpc)
#设置可debug
set(CMAKE_BUILD_TYPE,"Debug")
#设置项目可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置项目库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#设置项目编译头文件的搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
#设置项目库文件的搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)
#src下继续去寻找
add_subdirectory(src) #src包含框架所有的相关代码
add_subdirectory(example)
