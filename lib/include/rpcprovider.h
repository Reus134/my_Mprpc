#pragma once
#include "google/protobuf/service.h"

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>

#include<string>
#include<functional>
#include<google/protobuf/descriptor.h>
#include <unordered_map>
//框架提供的专门服务rpc服务的网络对象类
class RpcProvider
{
public:
    //这里是框架提供给外部使用的，可以注册rpc方法 service可以任意的接收
    void NotifyService(google::protobuf::Service *service);
    
    //启动RPC服务节点，开始提供rpc远程网络用
    void Run();
private:
    //组合EventLoop
    muduo::net::EventLoop m_eventLoop;
    //服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;//保存服务对象
        //保存服务
       // 方法名字-方法信息
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*>m_methodMap;
    };
    //存储注册成功的服务对象和所有信息
    //服务名字-服务信息
    std::unordered_map<std::string,ServiceInfo>m_serviceMap;
    //新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);
    //读写已经建立连接用户的读写回调
    void onMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* buffer,muduo::Timestamp);
    //Closure的回调操作，用于序列化rpc的响应和网络的发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);
};