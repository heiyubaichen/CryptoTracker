# CryptoTracker - 加密货币行情追踪器

基于 C++ 的命令行工具，实时获取比特币、以太坊等加密货币价格，存储历史数据并计算简单移动平均线（SMA）。

## 功能
- 获取实时价格及 24h 涨跌幅
- 存储历史快照到 MySQL
- 查询最近 N 天价格记录
- 计算 SMA(5) / SMA(10) 并给出趋势信号

## 技术栈
- C++11
- libcurl（HTTP 请求）
- nlohmann/json（JSON 解析）
- MySQL C API（数据持久化）

## 编译运行
使用该命令进行编译：
cl /EHsc /utf-8 /I"D:\software\vcpkg\installed\x64-windows\include" /I"C:\Program Files\MySQL\MySQL Server 8.0\include" main.cpp Tracker.cpp DatabaseManager.cpp HttpFetcher.cpp JsonParser.cpp /link /LIBPATH:"D:\software\vcpkg\installed\x64-windows\lib" libcurl.lib /LIBPATH:"C:\Program Files\MySQL\MySQL Server 8.0\lib" libmysql.lib