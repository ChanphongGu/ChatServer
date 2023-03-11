/**
 * 描述：负责连接与处理任务
 * 作者：guchenfeng
 * 日期：2023/1/7
 * 
*/
#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/uio.h>
#include <signal.h>
#include <stdlib.h>
#include <cstring>
#include <unordered_map>
#include <set>
#include <vector>
#include <cstring>
#include "locker.hpp"
#include "bloom_filter.h"
#include "mysql_connection.hpp"
#include "redis_connection.hpp"

//连接类 -- 处理连接与任务
class HttpConnection {
public:

    //连接池
    static HttpConnection* conns_pool;

    //所有的socket上的事件都被注册到同一个epoll_fd中
    static int m_epoll_fd;

    //统计用户数量
    static int m_user_num;

    //布隆过滤器
    static BloomFilter m_bloom_filter;

    //文件名最大长度
    static const int FILENAME_LEN = 200;

    //读缓冲区大小
    static const int READ_BUFFER_SIZE = 2048;

    //写缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024;

    //名字映射socket
    static std::unordered_map<std::string, int> m_name_socks;

    //socket映射名字
    static std::unordered_map<int, std::string> m_sock_names;

    //群组映射
    static std::unordered_map<int, std::set<int>> m_group;

    //源地址与目的地映射
    static std::unordered_map<std::string, std::string> m_sour_dest;

    //用户名与socket映射的互斥锁
    static Locker m_name_sock_mutex;

    //socket与用户名映射的互斥锁
    static Locker m_sock_name_mutex;

    //原地址与目的地址映射互斥锁
    static Locker m_sour_dest_mutex;

    //群组映射互斥锁
    static Locker m_group_mutex;

    /*设置静态EpollFd信息*/
    void static setEpollFd(int epoll_fd);

    /*枚举请求类型*/
    enum QUERY_CODE { COOKIE = 0, LOGIN, REGISTER, TARGET, CONTENT, GROUP, GROUP_MSG, NO_REQUEST};

    /*初始化连接池*/
    static HttpConnection* initConnPool(int MAX_SIZE);

    /*构造函数*/
    HttpConnection();

    /*析构函数*/
    ~HttpConnection();

    /*初始化*/
    void init(int sock_fd, const sockaddr_in& addr);

    /*初始化读写状态*/
    void initState();
    
    /*任务处理函数,由线程调用*/
    void process();

    /*关闭连接*/
    void closeConn();

    /*非阻塞一次性读*/
    bool readData();

    /*非阻塞一次性写*/
    bool writeData();

    /*填充写缓冲区*/
    bool setWriteBuffer(std::string content);

    /*
     * 请求处理相关
     */
    /*分析请求类型*/
    QUERY_CODE parseRequest();

    /*生成回复*/
    bool makeReply(QUERY_CODE code);

    /*检查会话过期*/
    std::string checkSession();

    /*验证登录信息*/
    bool login();

    /*处理注册*/
    bool registerUser();

    /*设置聊天对象*/
    bool target();

    /*消息转发*/
    bool transMsg();

    /*绑定群*/
    bool group();

    /*消息群广播*/
    bool tranGroupMsg();

private:

    //该连接的socket
    int m_sock_fd;

    //通信的socket地址
    sockaddr_in m_addr;

    //群组
    std::set<int> m_sock_groups;

    /*读相关*/
    //读缓冲区
    char m_read_buf[READ_BUFFER_SIZE];

    //标识度读缓冲区当前已读位置
    int m_read_index;

    /*写相关*/
    //写缓冲区
    char m_write_buf[WRITE_BUFFER_SIZE];

    //将要发送的字节数
    int bytes_to_send;

    //已发送的字节数
    int bytes_have_send;

    //写缓冲区待发送的位置
    int m_write_index;

};

#endif