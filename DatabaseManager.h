#pragma once
#include <mysql.h>
#include <string>
#include <vector>

// 定义一个结构体，存放一条价格记录
struct PriceRecord {
    double price;           // 价格
    double change_24h;      // 24h 涨跌幅
    std::string created_at; // 时间戳（字符串形式）
};

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    // 连接数据库
    bool connect(const std::string& host, const std::string& user,
        const std::string& pass, const std::string& db);
    // 插入一条价格记录
    bool insertPrice(const std::string& symbol, double price, double change_24h);
    // 获取最近 days 天的价格记录
    std::vector<PriceRecord> getRecentPrices(const std::string& symbol, int days);

private:
    MYSQL* conn;   // MySQL 连接对象指针
};