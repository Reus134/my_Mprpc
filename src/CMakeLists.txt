#当前目录的所有源文件放入SRC_LIST
aux_source_directory(. SRC_LIST)
#上面这个老出问题
#set(SRC_LIST mprpcapplication.cc mprpcconfig.cc rpcheader.pb.cc rpcprovider.cc)

#find_package(Protobuf REQUIRED) 
#编译成库文件 SHARED代表动态库
add_library(mprpc  ${SRC_LIST})

#连接muduo 注意怎么验证呢 muduo .a是一个静态库
target_link_libraries(mprpc muduo_net muduo_base  pthread zookeeper_mt)#${PROTOBUF_LIBRARIES}先不加
#