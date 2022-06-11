#include<iostream>
#include<string>
#include "../friend.pb.h"//注意这个user.pb.h在上一层目录的example直属目录喜爱
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include<vector>
#include<logger.h>

class FriendService:public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string>GetFriendList(uint32_t userid)
    {
        std::cout << "do GetFriendLists service! userid: " << userid << std::endl;
        std::vector<std::string> vec;
        vec.push_back("Eric");
        vec.push_back("George");
        vec.push_back("Sun");
        return vec;
    }
    //重写基类方法
     void GetFriendsList(google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRequest* request,
                       ::fixbug::GetFriendsListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid=request->userid();
        //调用本地方法
        std::vector<std::string>friendList=GetFriendList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        //把本地方法调用得到的朋友列表
        for(std::string &name : friendList)
        {
            std::string *p=response->add_friends();//p是一个指针
            *p=name;//这个指针指向的值改为本地方法中的调用得到的vector
        }
        done->Run();
    }
};
int main(int argc,char **argv) 
{
    LOG_INFO("FIRST LOG MESSAGE");
    LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);
    //框架的初始化操作 任何的初始化  provider -i config.conf 两个参数
    Mprpcapplication::Init(argc,argv);

    //provider 是一个网络服务对象 把UserService对象发布到rpc节点上
    RpcProvider provider;//框架上发布服务
    //NotifyService就是识别你要的服务和方法 
    provider.NotifyService(new FriendService());
    //启动一个rpc服务发布节点 Run以后 进程进入阻塞状态 等待远程的RPC请求调用 
    provider.Run();//相当于启动muduo库里面epoll的服务器
    return 0;
}