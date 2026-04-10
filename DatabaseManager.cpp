#include "DatabaseManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>

DatabaseManager::DatabaseManager() {
    // 初始化 MySQL 连接对象
    conn = mysql_init(nullptr);
}

DatabaseManager::~DatabaseManager() {
    if (conn) {
        mysql_close(conn);   // 关闭数据库连接
    }
}

bool DatabaseManager::connect(const std::string& host, const std::string& user,
    const std::string& pass, const std::string& db) {
    // 实际建立连接，参数：连接句柄，主机，用户，密码，数据库名，端口，socket，标志
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(),
        db.c_str(), 0, nullptr, 0)) {
        std::cerr << "MySQL connection failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    return true;
}

bool DatabaseManager::insertPrice(const std::string& symbol, double price, double change_24h) {
    // 构造 INSERT SQL 语句
    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO prices (symbol, price_usd, change_24h) VALUES ('%s', %.2f, %.2f)",
        symbol.c_str(), price, change_24h);
    // 执行 SQL
    if (mysql_query(conn, sql)) {
        std::cerr << "Insert failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    return true;
}

std::vector<PriceRecord> DatabaseManager::getRecentPrices(const std::string& symbol, int days) {
    std::vector<PriceRecord> records;
    // 构造 SELECT SQL，查询最近 days 天的记录，按时间升序
    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT price_usd, change_24h, DATE_FORMAT(created_at, '%%Y-%%m-%%d %%H:%%i:%%s') "
        "FROM prices WHERE symbol='%s' AND created_at > NOW() - INTERVAL %d DAY "
        "ORDER BY created_at ASC",
        symbol.c_str(), days);

    if (mysql_query(conn, sql)) {
        std::cerr << "Query failed: " << mysql_error(conn) << std::endl;
        return records;
    }

    // 获取查询结果集
    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return records;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        PriceRecord rec;
        rec.price = std::stod(row[0]);          // 第一列：价格
        rec.change_24h = std::stod(row[1]);     // 第二列：涨跌幅
        rec.created_at = row[2];                // 第三列：时间字符串
        records.push_back(rec);
    }
    mysql_free_result(result);   // 释放结果集内存
    return records;
}