#include"cmydb.h"

int main()
{
    cout<<getpid()<<endl;
    string server_host("localhost");
    string user("root");
    string password("111111");
    string db_name("test");
    CMyDb mydb;
    //mydb.initDB(server_host, user, password, db_name);
    mydb.initDB("localhost", "root", "111111", "yunpan");
    mydb.executeSQL("select * from client");
    return 0;
}
