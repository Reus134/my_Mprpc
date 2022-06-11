#include "mprpcapplication.h"
#include<iostream>
#include<unistd.h>
#include <string>

MprpcConfig Mprpcapplication::m_config;
void ShowArgHelp()
{
    std::cout <<"format:command -i <config>"<<std::endl;
}

void Mprpcapplication::Init(int argc, char **argv)
{
    if(argc < 2)//参数太少了
    {
        ShowArgHelp();
        exit(EXIT_FAILURE);
    }

    int c=0;
    std::string config_file;
    while((c=getopt(argc,argv,"i:="))!=-1)
    {
        switch(c)
        {
            case 'i':
                config_file = optarg;
                break;
            case '?':
                
                ShowArgHelp();
                exit(EXIT_FAILURE);
            case ':':
             
                ShowArgHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }
    //开始加载配置文件 
    //rpcserver_ip= rpcserver_port= zookeeper_ip= zookeeper_port= 
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout << "rpcserver_ip=" << m_config.Load("rpcserverip")<<std::endl;
    // std::cout << "rpcserver_port=" << m_config.Load("rpcserverport")<<std::endl;
    // std::cout << "zookeeper_ip=" << m_config.Load("zookeeperip")<<std::endl;
    // std::cout << "zookeeer_port=" << m_config.Load("zookeeperport")<<std::endl;
}

Mprpcapplication& Mprpcapplication::GetInstance()
{
    static Mprpcapplication app;
    return app;
}
MprpcConfig& Mprpcapplication::GetConfig()
{
    return m_config;
}
