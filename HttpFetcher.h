#pragma once
#include <string>

class HttpFetcher {
public:
    // 发送 GET 请求，返回服务器响应的字符串
    std::string get(const std::string& url); 
};