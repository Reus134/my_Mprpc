#pragma once
//mprap框架的初始化 单例模式
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
class Mprpcapplication
{
public:
    static void Init(int argc, char **argv);  
    static Mprpcapplication& GetInstance();
    static  MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config; //

    Mprpcapplication(){}
    Mprpcapplication(const Mprpcapplication&) = delete;
    Mprpcapplication(Mprpcapplication&&) = delete;
    //Mprpcapplication& operator=(const Mprpcapplication&) = delete;
  
};


