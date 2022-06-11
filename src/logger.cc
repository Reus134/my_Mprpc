#include "logger.h"
#include <time.h>
#include <iostream>
//设置日志的级别
void Logger::SetLogLevel(Loglevel level)
{
    m_logLevel = level;
}
//写日志
void  Logger::Log(std::string msg)
{
    //日志信息写入lockqueue缓存区中
    m_lckQue.push(msg);
}
//获取日志的单例
Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}
Logger::Logger()
{
    //启动专门的写日志线程 后面是一个lambda表达式
    //这个线程是一直做写日志的
    std::thread writeLogTask([&]()
    {   for(;;)
        {
            //获取当前的日期，然后取日志信息，写入相应的日志文件
            time_t now = time(nullptr);
            tm *nowtm=localtime(&now);

            char file_name[128];
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);
            /*"a+"以“追加/更新”方式打开文件，相当于a和r+叠加的效果。既
            可以读取也可以写入，也就是随意更新文件。如果文件不存在，那
            么创建一个新文件；如果文件存在，那么将写入的数据追加到文件
            的末尾（文件原有的内容保留）*/
            FILE *pf=fopen(file_name,"a+");
            if(pf==nullptr)
            {
                std::cout << "logger file:" << file_name << "open_error" << std::endl;
                exit(EXIT_FAILURE);
            }
            std::string msg=m_lckQue.pop();
            
            char time_buf[128]={0};
            sprintf(time_buf,"%d:%d:%d =>[%s]",
                    nowtm->tm_hour,
                    nowtm->tm_min,
                    nowtm->tm_sec,
                    (m_logLevel==INFO?"INFO":"ERROR"));
            
            msg.insert(0,time_buf);//从首位插 插一个十分秒
            msg.append("\n");
            //msg.c_str()写入到pf文件
            fputs(msg.c_str(),pf);
            fclose(pf);
        }
    });
    //设置分离线程，守护线程
    writeLogTask.detach();
}