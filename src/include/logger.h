#pragma once
#include "lockqueue.h"
#include <string>

enum Loglevel
{
    INFO,//普通信息
    ERROR//错误信息
};
// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \
    
//Mprpc框架提供的日志系统
class Logger
{
public:
    //设置日志的级别
    void SetLogLevel(Loglevel level);
    //写日志
    void Log(std::string msg);
    //获取日志的单例
    static Logger& GetInstance();
private:
    int m_logLevel;//记录日志的级别
    LockQueue<std::string> m_lckQue;//日志缓冲队列

    Logger();
    Logger(const Logger&)=delete;
    Logger(Logger&&)=delete;
    //Logger& operator=(const Logger&)=delete;
};
//定义宏  logmsgformat, ... 和可变参数
//log_info
// #define LOG_INFO(logmsgformat, ...)\
//     do\
//     { \
//         Logger &logger=Logger::GetInstance(); \
//         logger.SetLogLevel(INFO); \
//         char c[1024]={0};\
//         snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
//         logger.Log(c); \
//     }while(0);\


// #define LOG_ERR(logmsgformat, ...)\ 
//     do\
//     { \
//         Logger &logger=Logger::GetInstance(); \
//         logger.SetLogLevel(ERROR); \
//         char c[1024]={0};\
//         snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
//         logger.Log(c); \
//     }while(0)\