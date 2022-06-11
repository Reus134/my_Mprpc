#include<iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"
int main(int argc, char **argv)
{
    //整个程序启动后 想要使用mprpc框架来享受rpc服务调用，一定需要调用框架的初始化函数
    Mprpcapplication::Init(argc, argv);//配置信息
    //演示调用远程
    //RpcChannel一个抽象类 框架上来定义一个抽象类来控制
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    //自己设定登陆请求的rpc Login(LoginRequest) returns(LoginResponse);
    //LoginRequest是参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    //响应
    fixbug::LoginResponse response;
    //发起rpc的的调用 同步rpc的调用 MprpcChannel::Callmethod
    stub.Login(nullptr, &request, &response,nullptr);//RpcChannel->RpcChannel::Callmethod 集中来做所有的rpc方法调用的参数序列化和网络发送
    //一次rpc调用完成，读调用的结果
    if(response.result().errcode()==0)//errcode 为0就是成功
    {
        std::cout << "rpc Login response succsess:"<<response.success()<<std::endl;
    }
    else{
        std::cout << "rpc Login response Error :"<<response.result().errmsg()<<std::endl;
    }
    
    //演示调用远程发布rpc方法Register
    fixbug::RegisterRequest req;
    req.set_name("mprpc");
    req.set_pwd("666666");
    req.set_id(20000);
    fixbug::RegisterResponse rsp;
    //以同步的方式发起rpc调用请求
    stub.Register(nullptr,&req,&rsp,nullptr);

    //一次rpc调用完成，读调用的结果
    if(rsp.result().errcode()==0)//errcode 为0就是成功
    {
        std::cout << "rpc register response succsess:"<<rsp.success()<<std::endl;
    }
    else{
        std::cout << "rpc register response Error :"<<rsp.result().errmsg()<<std::endl;
    }
    return 0;
}