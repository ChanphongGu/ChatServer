/**
 * 描述：服务器入口
 * 作者：guchenfeng
 * 日期：2023/1/10
 * 
*/
#include <iostream>
#include <cstring>
#include <sys/epoll.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include "util.hpp"
#include "thread_pool.hpp"
#include "http_connection.h"
#include "locker.hpp"
#include "mysql_connection.hpp"

#define MAX_FD 65535 //最大文件描述符个数
#define MAX_EVENT_NUM 20000 //最大监听事件数量

int main(int argc, char* argv[]) {
    //如无指定则以8045端口启动
    int port = 8045;
    //若指定则以指定端口运行
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    //对断开连接的SIGPIPE信号进行忽略处理
    Util::addSig(SIGPIPE, SIG_IGN);

    //创建线程池
    ThreadPool<HttpConnection>* threads_pool = NULL;
    try {
        threads_pool = new ThreadPool<HttpConnection>(10, 10000);
    } catch (...) {
        std::cout << "Server: create threadpool fail!" << std::endl;
        exit(-1);
    }

    //创建连接池，保存已连接客户端信息
    HttpConnection* conns_pool = HttpConnection::initConnPool(MAX_FD);

    //初始化布隆过滤器
    HttpConnection::m_bloom_filter.init();
    //读取数据库user表的数据，将用户名哈希到布隆过滤器的位图
    MysqlConnection mysql_conn;
    mysql_conn.init();
    mysql_conn.connect();
    auto mysql_ret = mysql_conn.query("SELECT * FROM user");
    for (auto v : mysql_ret) {
        //user表两个字段：name 、password；v[0]是name字段
        HttpConnection::m_bloom_filter.add(v[0]);
    }
    
    //创建监听的套接字
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);

    //设置端口复用
    int reuse = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    //绑定服务器ip地址，服务器只能从ip为此地址的网卡读取消息
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; //可以读取任意网卡的消息
    addr.sin_port = htons(port); //设置端口号

    int ret = bind(listen_fd, (struct sockaddr*) &addr, sizeof(addr));
    //绑定失败
    if (ret == -1) {
        std::cout << "Server: bind fail!" << std::endl;
    }

    //监听，操作系统会分配全连接和半连接队列
    ret = listen(listen_fd, 30);
    //监听失败
    if (ret == -1) {
        std::cout << "Server: listen fail!" << std::endl;
    }

    //获取epoll描述符、创建事件数组、初始化
    int epoll_fd = epoll_create(10);
    epoll_event events[MAX_EVENT_NUM];

    //将listen描述符添加到epoll
    Util::addFd(epoll_fd, listen_fd, false);
    //设置连接的epoll描述符，所有的连接中用的epoll操作都基于此描述符
    HttpConnection::setEpollFd(epoll_fd);

    while (true) {
        
        std::cout << "Server: waiting for message or connection!" << std::endl;
        //epoll阻塞调用 ，1代表阻塞
        int cnt = epoll_wait(epoll_fd, events, MAX_EVENT_NUM, -1);
        //服务器出错，则关闭服务器
        if (cnt < 0 && errno != EINTR) {
            std::cout << "Server: server error , shutdown!" << std::endl;
            break;
        }
        
        //循环处理事件
        for (int i = 0; i < cnt; i ++ ) {
            //获取处理的文件描述符
            int sock_fd = events[i].data.fd;

            //根据类型处理
            if (sock_fd == listen_fd) { //监听到新连接
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                //从全连接队列取一个连接
                int conn_fd = accept(listen_fd, (struct sockaddr*) &client_addr, &client_addr_len);
                //若无连接
                if (conn_fd < 0) {
                    std::cout << "Server: connect fail for no connection!" << std::endl;
                    continue; //继续下一个事件处理
                }

                //连接池满，释放该连接
                if (HttpConnection::m_user_num >= MAX_FD) {
                    close(conn_fd);
                    continue;
                }

                //将连接数据初始化，并放至数组
                conns_pool[conn_fd].init(conn_fd, client_addr);
                
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) { //异常断开或者出错事件
                std::cout << "Server: connection error, unlinking the user!" << std::endl;
                conns_pool[sock_fd].closeConn();
            } else if (events[i].events & EPOLLIN) { //读事件
                if (conns_pool[sock_fd].readData()) { //读成功
                    //添加至任务队列处理
                    threads_pool->append(conns_pool + sock_fd);
                } else { //读失败
                    std::cout << "Server: read fail, unlinking!" << std::endl;
                    conns_pool[sock_fd].closeConn();
                }
            } else if (events[i].events & EPOLLOUT) { //写事件
                if (!conns_pool[sock_fd].writeData()) { //写失败
                    std::cout << "Server: write fail, unlinking !" << std::endl;
                    conns_pool[sock_fd].closeConn();
                }
            }
        } 
    }
    //退出关闭连接，回收内存
    close(epoll_fd);
    close(listen_fd);
    delete[] conns_pool;
    delete threads_pool;

    return 0;
}