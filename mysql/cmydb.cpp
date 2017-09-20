#include"cmydb.h"

CMyDb::CMyDb()
{
    cout<<"cmydb() begin"<<endl;
    mysql_init(&_conn);
    /*if(_conn = NULL)
    {
        perror("mysql_init");
	exit(1);
    }*/
    cout<<"cmydb() end"<<endl;
}

CMyDb::~CMyDb()
{
 //   if(_conn != NULL)
        mysql_close(&_conn);
}
bool CMyDb::initDB(string server_host, string user, 
               string password, string db_name)
{
    cout<<"initdb() begin"<<endl;
    mysql_real_connect(&_conn, server_host.c_str(), user.c_str(), password.c_str(), db_name.c_str(), 0, NULL, 0);
     //mysql_real_connect(&_conn, "127.0.0.1", "root", "111111", "test", 3306, NULL, 0);

     cout<<"hhh"<<endl;
    /* if(_conn == NULL)
     {
     cout<<"lll"<<endl;
         perror("mysql_real_connect");
         exit(1);
     }*/
    cout<<"initdb() end"<<endl;
     return true;
}

bool CMyDb::executeSQL(string sql_stl)
{
    cout<<"executeSQL() begin"<<endl;
    if(mysql_query(&_conn, "set names utf8"))
        fprintf(stderr, "%d: %s\n", mysql_errno(&_conn), mysql_error(&_conn));

    int t = mysql_query(&_conn, sql_stl.c_str());
    if(t)
    {
        printf("Error making query: %s\n", mysql_error(&_conn));
	exit(1);
    }
    else
    {
        _res = mysql_use_result(&_conn);
	if(_res)
	{
	    for(int i=0; i<mysql_field_count(&_conn); i++)
	    {
	        _row = mysql_fetch_row(_res);
		if(_row <= 0)
		    break;

		for(int r=0; r<mysql_num_fields(_res); r++)
		    printf("%s\t", _row[i]);

		printf("\n");
	    }
	}

	mysql_free_result(_res);
    }

    cout<<"executeSQL() endl"<<endl;
    return true;
}

bool CMyDb::create_table(string table_str_sql)
{
    int t = mysql_query(&_conn, table_str_sql.c_str());
    if(t)
    {
        printf("Error making query: %s\n", mysql_error(&_conn));
	exit(1);
    }

    return true;
}
