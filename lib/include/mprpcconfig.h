#pragma once
#include<unordered_map>
#include<string>
//框架读取配置文件类

class MprpcConfig
{
public:
    //负责解析加载配置文件
    void LoadConfigFile(const char* config_file);
    //查询配置信息
    std::string Load(const std::string& key);
private:
//线程安全问题 unordered_map不是线程安全的 但是只需要初始化一次 所以不需要考虑线程安全问题
    std::unordered_map<std::string,std::string> m_configMap;
    //去掉字符串前后的空格
    void Trim(std::string &src_buf); 
};