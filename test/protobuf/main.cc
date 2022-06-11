#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;
int main()
{
    // LoginResponse rsp;
    // ResultCode *rc=rsp.mutable_result();//这种在一个message里面的message 有这个方法
    // rc->set_errorcode(1);
    // rc->set_errmsg("登陆处理失败")
    GetFriendListResponse rsp;
    ResultCode *rc= rsp.mutable_result();
    rc->set_errorcode(1);
    User *user1=rsp.add_friend_list();
    user1->set_name("zhangsan");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2=rsp.add_friend_list();
    user2->set_name("lisi");
    user2->set_age(18);
    user2->set_sex(User::MAN);

    std::cout<<rsp.friend_list_size()<<std::endl;
}
int main1()
{   //封装了login请求对象的数据
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");
    //对象序列化=》 char* 
    std::string send_str;
    if(req.SerializeToString(&send_str))//序列化成功就会放在send_str里面了
    {
        std::cout << send_str << std::endl;
    }
    //从sent_ptr反序列化一个login请求对象
    LoginRequest reqB;
    if(reqB.LoginRequest::ParseFromString(send_str))
    {
        std::cout <<reqB.name()<<std::endl;
        std::cout <<reqB.pwd()<<std::endl;
    }
    return 0;
}