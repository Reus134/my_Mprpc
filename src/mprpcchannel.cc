#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include <string>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<error.h>
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"
/*
header_size+service_name+method_name arg_size+args 放在一个字符串
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response, 
                          google::protobuf::Closure* done) 
{
    //描述服务的名字和服务的方法
    const google::protobuf::ServiceDescriptor* sd=method->service();
    std::string service_name=sd->name();//service_name服务的名字是什么 ->UserServiceRpc
    std::string method_name=method->name();//method_name
 
    //获取参数的序列化字符串长度 args_size
    std::string args_str;
    uint32_t args_size=0;
    if(request->SerializeToString(&args_str))//request是请求 现在是将请求序列化给到args_str
    {
        args_size=args_str.size();
    }
    else{
        
        controller->SetFailed("serialize request error");
        return;
    }
    //定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);
    
    uint32_t header_size=0;
    std::string rpc_header_str;
    if(rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size=rpc_header_str.size();
    }
    else{
        controller->SetFailed("serialize rpc header error! ");
        return;
    }
    //组织待发送的rpc请求的
    //header_size+service_name+method_name arg_size+args 
    //放在一个字符串
    std::string send_rpc_str;
    //header_size是一个4字节的整数 
    //&header_size是header_size的首地址 用一个char*强制转化 4字节全部转换
    send_rpc_str.insert(0,std::string((char*)&header_size,4));
    send_rpc_str+=rpc_header_str;//rpcheader
    //rpc_header_str包括了service_name+method_name arg_size
    send_rpc_str+=args_str;
    //打印调试信息
    std::cout<<"==================="<<std::endl;
    std::cout<<"header_size:"<<header_size<<std::endl;
    std::cout<<"rpc_header_str:"<<rpc_header_str<<std::endl;
    std::cout<<"service_name:"<<service_name<<std::endl;
    std::cout<<"method_name:"<<method_name<<std::endl;
    std::cout<<"args_size:"<<args_size<<std::endl;
    std::cout<<"args_str:"<<args_str<<std::endl;
    std::cout<<"==================="<<std::endl;
    //上面是序列化

    //下面使用TCP编程 完成远程调用
    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1 == clientfd)
    {
        char errtxt[512]={0};
        sprintf(errtxt,"crete socket error:%d",errno);
        //携带控制信息
        controller->SetFailed(errtxt);
        return;
    }
    //读取配置文件rpcserver的信息 有了zookeeper就不需要了
    //std::string ip=Mprpcapplication::GetInstance().GetConfig().Load("rpcserverip");
    //注意atoi还需要.c_str()方法才能使用
    //uint16_t port=atoi(Mprpcapplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    ZkClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login  往zookeeper查找一下
    std::string method_path = "/" + service_name + "/" + method_name;
    // 127.0.0.1:8000  hostdate存储的是这个服务-》方法存在的 IP地址和端口号
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);//[0,:)
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str());//把string转换为int  注意.c_str`
   //之后用muduo去
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr=inet_addr(ip.c_str());
    //连接rpc服务节点
    if(-1==connect(clientfd,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        
        close(clientfd);
        char errtxt[512]={0};
        sprintf(errtxt,"connect error! erro:%d",errno);
        //携带控制信息
        controller->SetFailed(errtxt);
        return;
    }
   if(-1==send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0))
   {
       close(clientfd);
       char errtxt[512]={0};
       sprintf(errtxt,"send error! erro:%d",errno);
       //携带控制信息
       controller->SetFailed(errtxt);
        return;//结束这一次的调用
   }
   //接收rpc请求的响应
   char recv_buf[1024]={0};
   int recv_size=0;
   if(-1==(recv_size=recv(clientfd,recv_buf,1024,0)))
   {
      
       close(clientfd);
       char errtxt[512]={0};
       sprintf(errtxt,"recv error! erro:%d",errno);
       //携带控制信息
       controller->SetFailed(errtxt);
       return;
   }
   //字符流写到response 反序列化rpc调用的响应数据
   //特别注意这边 string的构造函数有问题 他遇到\0会停下来
  // std::string response_str(recv_buf,0,recv_size);
  // if(!response->ParseFromString(response_str))
   if(!response->ParseFromArray(recv_buf,recv_size))//直接从数组来
   {
      
       close(clientfd);
       char errtxt[512]={0};
       sprintf(errtxt,"parse error! response_str:%s",recv_buf);
       //携带控制信息
       controller->SetFailed(errtxt);
       return;
   }
   close(clientfd);
}