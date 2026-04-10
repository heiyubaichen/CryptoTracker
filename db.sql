CREATE DATABASE crypto_db;
USE crypto_db;

CREATE TABLE prices (
    id INT PRIMARY KEY AUTO_INCREMENT,
    symbol VARCHAR(20) NOT NULL,
    price_usd DECIMAL(12,2) NOT NULL,
    change_24h DECIMAL(8,2),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_symbol_time ON prices(symbol, created_at);