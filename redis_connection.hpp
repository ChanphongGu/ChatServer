/**
 * 描述：封装hiredis连接
 * 作者：guchenfeng
 * 日期：2022/12/11
 * 
*/
#ifndef REDIS_CONNECTION_HPP
#define REDIS_CONNECTION_HPP

#include <hiredis/hiredis.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <cstring>


//redis连接类
class RedisConnection {
public:

    /*redis地址*/
    struct redis_addr {
        std::string host = "127.0.0.1";
        unsigned int port = 6379;
    };
    /*构造函数*/
    RedisConnection() {

    }

    /*析构函数*/
    ~RedisConnection() {
        //如果连接存在，关闭连接
        if (m_conn) {
            close();
        }
    }

    /*连接*/
    bool connect() {
        m_conn = redisConnect(m_addr.host.c_str(), m_addr.port);
        //连接失败
        if (m_conn->err) {
            close();
            std::cout << "RedisConnection: redis server connection fail!" << std::endl;
            return false;
        }
        return true;
    }

    /*执行查询命令*/
    std::string query(std::string cmd) {
        std::string res = "";
        //无连接
        if (!m_conn) {
            std::cout << "RedisConnection: redis server no connection!" << std::endl;
            return res;
        }

        //获取查询结果
        redisReply* ret = (redisReply*)redisCommand(m_conn, cmd.c_str());
        if (ret->str) {
            res = ret->str;
        }
        //释放结果集内存
        freeReplyObject(ret);
        return res;
    }

    /*断开连接*/
    void close() {
        //连接存在则关闭
        if (m_conn) {
            redisFree(m_conn);
            m_conn = NULL;
        }
    }

private:

    //redis连接
    redisContext* m_conn;

    //redis地址
    redis_addr m_addr;
};

#endif