#include<iostream>
#include<string>
#include "../user.pb.h"//注意这个user.pb.h在上一层目录的example直属目录喜爱
#include "mprpcapplication.h"
#include "rpcprovider.h"
/*
Userservice     原来是一个本地服务 
提供了 两个进程内的本地方法 login 和GetFriendList
*/


class Userservice:public fixbug::UserServiceRpc//使用在Rpc服务器的发布端
{
private:
    /* data */
public:
   //专门去做用户的登陆
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service:Login"<<std::endl;
        std::cout <<"name:"<<name<<" pwd:"<<pwd<<std::endl;
        return true;
    }

    bool Register(uint32_t id,std::string name,std::string pwd)
    {
        std::cout << "doing local service:Register"<<std::endl;
        std::cout <<"id"<<id<<"name:"<<name<<" pwd:"<<pwd<<std::endl;
        return true;
    }
    /*
    重写基类的UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    1.caller=>Login(LoginRequest)=muduo=》callee
    2.callee=>Login(LoginRequest)=>交到下面重写的Login方法上 
    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
    //框架给业务上报了请求参数LoginRequest,应用获取相应的数据做本地业务
    std::string name=request->name();//从request中取name
    std::string pwd=request->pwd();//如果没有protobuf我们得自己去解析
    
    bool login_result=Login(name,pwd);//对应着上面第一个

    //把响应写入调用方返回 包括错误马错误消息
    fixbug::ResultCode *code=response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(login_result);

    //执行回调  执行响应对象的数据序列化和网络发送（都是由框架完成的）
    done->Run();
    }
void Register(google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id=request->id();
        std::string name=request->name();
        std::string pwd=request->pwd();
        //直接做本地业务
        bool ret=Register(id,name,pwd);
        //填响应
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        done->Run();
    }                   
};

int main(int argc,char **argv) 
{
    //框架的初始化操作 任何的初始化  provider -i config.conf 两个参数
    Mprpcapplication::Init(argc,argv);

    //provider 是一个网络服务对象 把UserService对象发布到rpc节点上
    RpcProvider provider;//框架上发布服务
    //NotifyService就是识别你要的服务和方法 
    provider.NotifyService(new Userservice());
    //启动一个rpc服务发布节点 Run以后 进程进入阻塞状态 等待远程的RPC请求调用 
    provider.Run();//相当于启动muduo库里面epoll的服务器
    return 0;
}

