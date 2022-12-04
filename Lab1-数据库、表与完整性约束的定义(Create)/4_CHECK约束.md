```SQL
--  请在以下空白处填写完成任务的语句，空白行可通过回车换行添加。
--  你也可以在命令行窗口完成任务，不过，在命令行键入的语句不会被保存。
CREATE DATABASE IF NOT EXISTS MyDb;
USE MyDb;
DROP TABLE IF EXISTS products;
CREATE TABLE products(pid CHAR(10) PRIMARY KEY, name VARCHAR(32),
                      brand CHAR(10), price INT,
                      CONSTRAINT CK_products_brand CHECK(brand IN ('A','B')),
                      CONSTRAINT CK_products_price CHECK(price > 0));
--  结束
``` 