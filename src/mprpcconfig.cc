#include "mprpcconfig.h"
#include<iostream>
#include<string>
//负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char* config_file)
{
    FILE *pf=fopen(config_file,"r");//读文件
    if(nullptr == pf)
    {
        std::cout<<config_file<<"is not exists"<<std::endl;
        exit(EXIT_FAILURE);
    }
    /*
    1.注释
    2。正确的配置
    3。去除多余的空格*/
    while (!feof(pf))//没操作完就一直循环读取
    {
        char buf[512]={0};
        /**
        fgets函数
        函数调用形式如：fgets(str,n,fp);
 
    函数作用：
        从fp所指的文件中读出n-1个字符送入字符数
        组str中，因为在最后加一个’\0’*/
        fgets(buf,512,pf);
        std::string read_buf(buf);
        Trim(read_buf);//去掉readbuf前后的空格
        //判断#的注释
        if(read_buf=="#"||read_buf.empty())
        {
            continue;
        }

        //解析配置项
        int idx=read_buf.find('=');
        if(idx==-1)//==-1就是没找到等号
        {
            //配置项不合法
            continue;
        }
        std::string key;
        std::string value;
        key=read_buf.substr(0,idx);//idx是等号前面
        // read_buf.pop_back();
        // value=read_buf.substr(idx+1);//-idx
        Trim(key); //127.0.0.1    \n     不需要全部的
        //举例：rpcserverip=127.0.0.1\n idx此时是等号的位置
        int end_idx=read_buf.find('\n',idx);//从等号的位置继续往后读
        value=read_buf.substr(idx+1,end_idx-idx-1);

        Trim(value);
        m_configMap.insert({key,value}); //不要用[] 因为key 不存在的话会添加东西哦

    }
    
}
    //查询配置信息
std::string MprpcConfig::Load(const std::string& key)
{
    auto it=m_configMap.find(key);
    if(it==m_configMap.end())//如果是没找到 返回空字符串
    {
        return "";
    }
    return it->second;
}

//去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf)
{
int idx=src_buf.find_first_not_of(' ');
        if(idx!=-1)//==-1的话 没找到 就没有空格
        {
            //说明字符串前面有空格
            src_buf=src_buf.substr(idx,src_buf.size()-idx);
        }
        //去掉后面多余的空格
        idx=src_buf.find_last_not_of(' ');
        if(idx!=-1)//==-1
        {
            //说明字符串后面有空格
            src_buf=src_buf.substr(0,idx+1);
        }

}