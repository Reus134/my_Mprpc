#pragma once 
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    //zkclient 启动连接 zkserver
    void Start();
    //zkserve上指定的path 创建znode节点 
    void Create(const char* path,const char* data,int datalen,int state=0);
    //根据参数指定的znode路径  获得存的数据
    std::string GetData(const char* path);
private:
    //zk的客户端句柄  标识了客户端的句柄 通过这个可以操作
    zhandle_t *m_zhandle;
};