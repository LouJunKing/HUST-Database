```SQL
--  请在以下空白处填写完成任务的语句，空白行可通过回车换行添加。
--  你也可以在命令行窗口完成任务，不过，在命令行键入的语句不会被保存。
CREATE DATABASE IF NOT EXISTS MyDb;
USE MyDb;
DROP TABLE IF EXISTS dept; -- 方便多次评测，可以删
CREATE TABLE dept(deptNo INT PRIMARY KEY, deptName VARCHAR(32));
DROP TABLE IF EXISTS staff; -- 方便多次评测，可以删
CREATE TABLE staff(staffNo INT PRIMARY KEY, staffName VARCHAR(32), 
                    gender CHAR(1), dob DATE, salary NUMERIC(8,2), deptNo INT,
                    CONSTRAINT FK_staff_deptNo FOREIGN KEY(deptNo) REFERENCES dept(deptNO));


--  结束 
```