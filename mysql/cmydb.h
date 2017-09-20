#pragma once
#include<iostream>
#include<mysql/mysql.h>
#include<string>
#include<stdio.h>
#include<stdlib.h>
using namespace std;


class CMyDb
{
public:
    CMyDb();
    bool initDB(string server_host, string user, 
               string password, string db_name);
    bool executeSQL(string sql_stl);
    bool create_table(string table_str_sql);
    ~CMyDb();
private:
    MYSQL _conn;
    MYSQL_RES *_res;
    MYSQL_ROW _row;
};
