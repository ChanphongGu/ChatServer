/**
 * 描述：布隆过滤器，用于判断用户账号
 * 作者：guchenfeng
 * 日期：2023/1/8
 * 
*/
#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <iostream>
#include <bitset>
#include <cstring>
//布隆过滤器类
class BloomFilter {
public:
    //无符号长整型，用于对哈希值取模
    typedef unsigned long long ULL;

    //布隆过滤器大小
    static const int BLOOM_SIZE = 10000000;

    //字符串哈希函数的进制，可以使用131或者13331
    static const int P = 131;

    /*构造函数*/
    BloomFilter();

    /*析构函数*/
    ~BloomFilter();

    /*初始化*/
    void init();

    /*获取字符串布隆结果*/
    bool get(std::string name);

    /*添加字符串至布隆*/
    bool add(std::string name);

    /*哈希函数*/
    ULL hash(std::string name);

    /*布隆位图*/
    std::bitset<BLOOM_SIZE>& get();
private:

    //位图
    std::bitset<BLOOM_SIZE> m_bloom_filter;

    
};




#endif