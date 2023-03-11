#include "bloom_filter.h"
/*获取字符串布隆结果*/
bool BloomFilter::get(std::string name) {
    //字符串哈希
    ULL index = hash(name) % BLOOM_SIZE;
    //如果布隆过滤器中存在
    if (!m_bloom_filter[index]) {
        return false;
    }
    //不存在则不必访问数据库
    return true;
}

/*添加字符串至布隆*/
bool BloomFilter::add(std::string name) {
    //字符串哈希
    ULL index = hash(name) % BLOOM_SIZE;
    //添加至布隆过滤器
    m_bloom_filter[index] = 1;
    return true;
}

/*哈希函数*/
BloomFilter::ULL BloomFilter::hash(std::string name) {
    ULL res = 0;
    //字符串P进制求哈希值
    for (auto c : name) {
        res = res * P + c;
    }
    return res;
}

/*构造函数*/
BloomFilter::BloomFilter() {}

/*析构函数*/
BloomFilter::~BloomFilter() {}

/*初始化*/
void BloomFilter::init() {
    //每一位都置为0
    m_bloom_filter.reset();
}

/*布隆位图*/
std::bitset<BloomFilter::BLOOM_SIZE>& BloomFilter::get() {
    return m_bloom_filter;
}