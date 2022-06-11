#pragma once
#include <queue>
#include <thread>
#include <mutex>//pthread_mutex_t
#include <condition_variable>//pthread_condition_t
//异步写日志

//模板代码只能写在头文件
template <typename T>
class LockQueue 
{
public:
    //多个worker线程都会写日志queue
    void push(const T &data)
    {   
        //智能锁 能自动析构
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        //  .notify_one()：一次只能通知一个的线程，
        //唤醒这一个线程中的condition_variable类的.wait()函数！
        //也即（2个线程中）唤醒另一个线程中的wait()函数。
        m_condvariable.notify_one();//只有一个线程在写日志文件
    }
    //只有一个线程来读日志 写日志文件
    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty())//为空就不要往下面去了
        {
            //日志队列为空，线程进入wait状态
            m_condvariable.wait(lock);//进入wait并把锁释放 
            //入队和出队用的是一把锁 所以把锁释放

        }
        //不能返回局部变量的引用 刚才返回的T& data是一个局部变量
        T data=m_queue.front();
        m_queue.pop();
        return data;

    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    //condition_variable->条件变量
    //有 notify_one 和notify_all线程 通知一个线程和通知所有线程
    std::condition_variable m_condvariable;
};
