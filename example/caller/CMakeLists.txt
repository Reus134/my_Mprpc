#set(SRC_LIST calluserservice.cc ../user.pb.cc)
set(SRC_LIST callfriendservice.cc ../friend.pb.cc)
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lprotobuf  -std=c++11  -lpthread ")  
#设置编译带的参数

find_package(Protobuf REQUIRED)  #查找protobuf包

#include_directories(${Protobuf_INCLUDE_DIRS})  #添加头文件
#include_directories(${CMAKE_CURRENT_BINARY_#

#include_directories(/usr/local/include)  
#link_directories(/usr/local/lib)    #链接库文件

add_executable(consumer ${SRC_LIST})
#连接框架的动态库
target_link_libraries(consumer mprpc ${PROTOBUF_LIBRARIES})