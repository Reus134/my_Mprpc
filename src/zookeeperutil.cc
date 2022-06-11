#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include<semaphore.h>
#include<iostream>

//全局的watcher观察类

// 全局的watcher观察器   zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type,
                   int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  // 回调的消息类型是和会话相关的消息类型  会话连接 或者会话超时断开连接
	{
		if (state == ZOO_CONNECTED_STATE)  // zkclient和zkserver连接成功
		{
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            //sem_post( sem_t *sem )用来增加信号量的值。
            //当有线程阻塞在这个信号量上时，调用这个函数会使
            //其中的一个线程不在阻塞，选择机制同样是由线程的调度策略决定的
            sem_post(sem);//信号量资源+1 就会让下面Start函数内的sem等待阻辙取消
            //上面这个是通过 state和type一起来判断表示连接成功了
		}
	}
}


ZkClient::ZkClient():m_zhandle(nullptr)
{

}

ZkClient::~ZkClient()
{
    if(m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);//不是空就关闭这个句柄释放资源
    }
}

//连接zkserver
void ZkClient::Start()
{
    std::string host=Mprpcapplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port=Mprpcapplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr=host+":"+port;//zk的init函数的第一个参数的格式是127.0.0.1:80这种样子的
    //返回一个句柄  
    /*
    zookeeper_mt:多线程版本
    zookeeper的API客户端程序提供了三个线程
    API调用线程
    网络IO线程 pthread_create poll
    watcher回调线程

    */
   //连接zksever 是一个异步的方法 所以一开始并不会说连接成功与否
    m_zhandle=zookeeper_init(connstr.c_str(),global_watcher,30000,nullptr,nullptr,0);
    if (nullptr == m_zhandle) //内存开辟失败
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    /*
    int sem_init(sem_t *sem, int pshared, unsigned int value);
    其中sem是要初始化的信号量，pshared表示此信号量是在进程间共享还是线程间共享，value是信号量的初始值。
    */
    sem_init(&sem,0,0);
    zoo_set_context(m_zhandle,&sem);//上下文就是给观察器传参数  给指定的句柄设置了一个信号量

    sem_wait(&sem);//主线程阻塞 
    //走到下面来说明连接成功了 开始上面的sempost发消息了
    std::cout << "zookeeper_init success!" << std::endl;
}

void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	flag = zoo_exists(m_zhandle, path, 0, nullptr);
	if (ZNONODE == flag) // 表示path的znode节点不存在 不存在才应该判断
	{
		// 创建指定path的znode节点了
		flag = zoo_create(m_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
            //上面这个state 是生成临时的还是永久性节点
            // 用get/.../..某个节点可以看到 ephemeralOwner 为0就是临时的
		if (flag == ZOK)
		{
			std::cout << "znode create success... path:" << path << std::endl;
		}
		else
		{
			std::cout << "flag:" << flag << std::endl;
			std::cout << "znode create error... path:" << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}
// 根据指定的path，获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);//获取数据到bufferlen里面
	if (flag != ZOK)
	{
		std::cout << "get znode error... path:" << path << std::endl;
		return "";
	}
	else
	{
		return buffer;
	}
}
