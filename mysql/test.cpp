#include<mysql/mysql.h>
#include<iostream>
using namespace std;

int main()
{
    /*MYSQL mydb;
    mysql_init(&mydb);
    if(!mysql_real_connect(&mydb, "localhost", "root", "111111", "test", 0, NULL, 0))
    {
        cout<<"connect fail"<<endl;
    }
    mysql_close(&mydb);
    */
    MYSQL*mydb;
    mydb =  mysql_init(NULL);
    if(!mysql_real_connect(mydb, "localhost", "root", "111111", "test", 0, NULL, 0))
    {
        cout<<"connect fail"<<endl;
    }
    mysql_close(mydb);
    return 0;
}
