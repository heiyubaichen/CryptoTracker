#include "DatabaseManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <stdexcept>

DatabaseManager::DatabaseManager() : conn(nullptr) {
    conn = mysql_init(nullptr);
    if (!conn) {
        throw std::runtime_error("mysql_init() failed: insufficient memory.");
    }
}

DatabaseManager::~DatabaseManager() {
    if (conn) {
        mysql_close(conn);
    }
}

bool DatabaseManager::connect(const std::string& host, const std::string& user,
    const std::string& pass, const std::string& db) {
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(),
        db.c_str(), 0, nullptr, 0)) {
        throw std::runtime_error(std::string("数据库连接失败: ") + mysql_error(conn));
    }
    return true;
}

bool DatabaseManager::insertPrice(const std::string& symbol, double price, double change_24h) {
    char escaped[128];
    mysql_real_escape_string(conn, escaped, symbol.c_str(), static_cast<unsigned long>(symbol.size()));

    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO prices (symbol, price_usd, change_24h) VALUES ('%s', %.2f, %.2f)",
        escaped, price, change_24h);

    if (mysql_query(conn, sql)) {
        throw std::runtime_error(std::string("插入价格失败: ") + mysql_error(conn));
    }
    return true;
}

std::vector<PriceRecord> DatabaseManager::getRecentPrices(const std::string& symbol, int days) {
    std::vector<PriceRecord> records;

    char escaped[128];
    mysql_real_escape_string(conn, escaped, symbol.c_str(), static_cast<unsigned long>(symbol.size()));

    char sql[256];
    snprintf(sql, sizeof(sql),
        "SELECT price_usd, change_24h, DATE_FORMAT(created_at, '%%Y-%%m-%%d %%H:%%i:%%s') "
        "FROM prices WHERE symbol='%s' AND created_at > NOW() - INTERVAL %d DAY "
        "ORDER BY created_at ASC",
        escaped, days);

    if (mysql_query(conn, sql)) {
        throw std::runtime_error(std::string("查询历史数据失败: ") + mysql_error(conn));
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        throw std::runtime_error(std::string("获取查询结果失败: ") + mysql_error(conn));
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        PriceRecord rec;
        rec.price = std::stod(row[0]);
        rec.change_24h = std::stod(row[1]);
        rec.created_at = row[2];
        records.push_back(rec);
    }
    mysql_free_result(result);
    return records;
}