#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"
//这个类的主要功能是集合了muduo库的网络功能 然后解析rpc请求并且返回
/*
service_name =>service描述  
                    =》service* 服务对象
                    method_name => method方法
json protobuf 
*/

    //框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_Info;
    //获取了服务对象的描述信息GetDescriptor() 是一个常指针 得用const指针来接收他
    const google::protobuf::ServiceDescriptor *pserviceDesc=service->GetDescriptor();
    //获取服务的名字
    std::string service_name = pserviceDesc->name();
   //std::cout << "service_name: " << service_name << std::endl;
   LOG_INFO("service_name:%s",service_name.c_str());
    //获取服务对象的方法和数量
    int methodCnt = pserviceDesc->method_count();
    for(int i=0; i<methodCnt; i++)
    {
        //获取了服务对象指定下表的服务方法的描述
        //他可能有很多个方法 然后这个循环是每一个的名字和具体的方法都存储在哈系表里面
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_Info.m_methodMap.insert({method_name,pmethodDesc});
        //std::cout << "method_name: " << method_name << std::endl;
        LOG_INFO("method_name:%s",method_name.c_str());
    }
     service_Info.m_service=service;//相当于我解释里面的那张表
     m_serviceMap.insert({service_name,service_Info});
}
  //启动rpc服务节点，开始提供rpc远程网络调用服务  
void RpcProvider::Run()
{
    //读取配置文件rpcserver的信息
    std::string ip=Mprpcapplication::GetInstance().GetConfig().Load("rpcserverip");
    //注意atoi还需要.c_str()方法才能使用
    uint16_t port=atoi(Mprpcapplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);

    //启动TcPServer
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    //绑定连接回调和消息读写回调方法 muduo库分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    //回调函数 是对象里的方法 必须得绑定
    server.setMessageCallback(std::bind(&RpcProvider::onMessage,this,std::placeholders::_1,
                            std::placeholders::_2,
                            std::placeholders::_3));
    //设置muduo库的线程数量
    server.setThreadNum(4);

    //当前rpc节点上要发布的服务全部注册到zk上区，让rpc client可以从zk上发现服务
    //session timeout 30s zkclient 的网络IO线程 1/3的超时时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    //service_name是永久节点 method_name是临时节点
    for(auto &sp:m_serviceMap)//创建map里面所有的service_name
    {
        //service_name    
        std::string service_path="/"+sp.first;//节点的格式是"/zookeeper"
        zkCli.Create(service_path.c_str(),nullptr,0);
        for(auto &mp:sp.second.m_methodMap)
        {
        // 格式/service_name/method_name
        std::string method_path=service_path+"/"+mp.first;
        char method_path_data[128]={0}; //这是本方法存的数据 里面的内容是在什么ip地址下的多少端口下有这个服务
        sprintf(method_path_data,"%s:%d",ip.c_str(),port);
        //ZOO_EPHEMERAL表示znode是一个临时性节点
        zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    //启动服务
    std::cout << "RpcProvider start service at ip:" << ip<<" port:" << port<<std::endl;
    LOG_INFO("RpcProvider start service at ip:%s,port:%d",ip.c_str(),port);
    server.start(); 
    m_eventLoop.loop();

}
//新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        //和rpc的client连接断开了
        //关闭
        conn->shutdown();
    }
}
/*
在框架内部 RpcProvider和rpcConsumer协商好通信的protobuf数据类型
数据包含者三部分service_name method_name args 定义proto的message类型进行数据头的序列化和反序列化
                   为了防止粘包在method后面要 arg_size
比如UserServiceLoginZhang san123456
约定存成header_size(4个字节）+header_str+args_str这个样子
这里的4个字节是二进制存储的 二进制存储整数是不会超过4个字节的 
2^32次方吗 如果用string “11111”就5个字节了

*/
//以建立连接用户的读写事件回调  如果远端有一个rpc服务的调用请求 那么OnMessage就会响应
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr& conn,
                          muduo::net::Buffer* buffer,
                          muduo::Timestamp)
{
    //网络上结合的元从rpc调用请求的字符流 ，Login args 
    std::string recv_buf=buffer->retrieveAllAsString();
    //读取前4个字节的内容
    //4个字节34位的整数
    uint32_t header_size=0;
    //首先recv_buf前32位都是二进制存储的 
    //&head_size代表的是head_size这个32位整数的首地址
    //(char*)&head_size char* 是因为copy方法的第一个参数是char*
    //(char*)&head_size 就是强制转换成char指针
    //从recv_buf的0开始4个字节的二进制位会复制给head_size
    recv_buf.copy((char*)&header_size,4,0);//从0开始的4个字节
    //根据header_size读取数据的原始字符流 然后反序列化数据
    std::string rpc_header_str=recv_buf.substr(4,header_size);//从第四个字节开始读取header_size个字符 
    //mprpc是刚刚在rpcheader.proto里面定义的命名空间 这个里面的RpcHeader
    mprpc::RpcHeader rpcHeader;

    std::string service_name;
    std::string method_name;
    uint32_t args_size=0;
    //反序列化
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列成功
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();
    }
    else
    {
        //数据头的反序列化失败
        LOG_ERR("rpc_header_str:%s,parse error",rpc_header_str.c_str());
        std::cout << "rpc_header_str:"<<rpc_header_str <<"parse error" << std::endl;
        return;
    }
    //获取rpc方法的参数方法
    std::string args_str=recv_buf.substr(4+header_size,args_size);
    //打印调试信息
    std::cout<<"==================="<<std::endl;
    std::cout<<"header_size:"<<header_size<<std::endl;
    std::cout<<"rpc_head_str:"<<rpc_header_str<<std::endl;
    std::cout<<"service_name:"<<service_name<<std::endl;
    std::cout<<"method_name:"<<method_name<<std::endl;
    std::cout<<"args_size:"<<args_size<<std::endl;
    std::cout<<"args_str:"<<args_str<<std::endl;
    std::cout<<"==================="<<std::endl;

    //获取serevice对象和method对象
    auto it=m_serviceMap.find(service_name);
    if(it==m_serviceMap.end())//请求了一个没有的
    {
        std::cout<<service_name<<"is not exist"<<std::endl;
        LOG_ERR("%s is not exist",service_name.c_str());
        return ;
    }
   
    //取方法
    auto mit=it->second.m_methodMap.find(method_name);
    if(mit==it->second.m_methodMap.end())
    {
        std::cout<<service_name<<":"<<method_name<<"is not exist"<<std::endl;
         //LOG_ERR("%s is not exist",service_name.c_str());
        return;
    }
    google::protobuf::Service *service = it->second.m_service;//获取service对象 UserService
    const google::protobuf::MethodDescriptor *method = mit->second;//获取method对象 Login方法
    //生成rpc方法调用的请求request和响应response参数 
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();//生成一个Loginrequest对象
    if(!request->ParseFromString(args_str))//参数的反序列化
    {
        std::cout <<"request parse error,content: " << args_str<<std::endl;
        return;
    }
    google::protobuf::Message *response =service->GetResponsePrototype(method).New();//生成响应对象
//给下面的method方法的俄调用，绑定一个Closure的回调函数
/*
inline Closure* NewCallback(Class* object, void (Class::*method)(Arg1, Arg2),
                            Arg1 arg1, Arg2 arg2) */
    google::protobuf::Closure *done=google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>
    (this,&RpcProvider::SendRpcResponse,conn,response);
    //在框架上根据远端的rpc请求，调用但当前RPC节点上发布的方法
    //new UserService().Login(constroller,request,response,done)
    
    service->CallMethod(method,nullptr,request,response,done);
}
//Closure的回调操作，用于序列化rpc的响应和网络的发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn,google::protobuf::Message* response)
{
    std::string response_str;
    if(response->SerializeToString(&response_str))//结果进行一个序列化 
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送给rpc的调用方法
        conn->send(response_str);
        
    }
    else
    {
        std::cout << "serialize response_str error,"<<std::endl;
    
    }
    conn->shutdown();//模拟http的短连接服务，由rpcprovider主动断开连接
}
