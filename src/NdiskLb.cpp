#include "../net/TcpServer.h"
#include <set>
#include <vector>
#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<assert.h>
#include<wait.h>
using namespace std;

class CliConn:public Conn
{
private:
    Conn * _ser;
public:
    void setser(Conn *conn)
    {
        _ser = conn;
    }

    Conn* getser()
    {
        return _ser;
    }
};

class SerConn:public Conn
{
private:
    map<char*, Conn*> _mm;
public:
    void setcli(char* dir, Conn *conn)
    {
        _mm.insert(pair<char*, Conn*>(dir, conn));
    }

    Conn* getcli(char *dir)
    {
        map<char*, Conn*>::iterator ita;
	ita = _mm.find(dir);
	if(ita == _mm.end())
	     return NULL;
        return ita->second;
    }

    void delcli(char *dir)
    {
        map<char*, Conn*>::iterator ita;
	ita = _mm.find(dir);
	if(ita == _mm.end())
	     return ;
        _mm.erase(ita);
    }
};

class TestServer : public TcpEventServer
{
private:
  vector<Conn*> vec;
  vector<SerConn*> _servec;
  int _sernum;

protected:
  void ReadEvent(Conn *conn);
  void WriteEvent(Conn *conn);
  void ConnectionEvent(Conn *conn);
  void ConnectionEvent_conn(Conn *conn);
  void CloseEvent(Conn *conn, short events);
public:
  TestServer(int count, int sernum);
  ~TestServer() { } 

  static void QuitCb(int sig, short events, void *data);
  static void TimeOutCb(int id, int short events, void *data);
};

TestServer::TestServer(int count, int sernum) : TcpEventServer(count), 
                                                _sernum(sernum)
{
    
}

int analyse_buff(char *buff)
{
   if(strcmp(buff, "##log in**") == 0)
   	return 1;
   else if(strcmp(buff, "##register**")==0)
   	return 2;
   else if(strcmp(buff, "##quit**")==0)
   	return 3;
   return -1;
}

void TestServer::ReadEvent(Conn *conn)
{
  int n = conn->GetReadBufferLen();
  cout<<n<<":  ";
  char buff[1024];
  memset(buff, 0, 1024);
  conn->GetReadBuffer(buff, 1023);
  cout<<buff<<endl;
////////////////////////////////
////////////////////////////////
 
  if(strcmp(buff, "&*$%&*(*i am alive%^#&(") == 0)
  {
      conn->setalive(true);
  }
  else
  {
      
  }
}

void TestServer::WriteEvent(Conn *conn)
{
  //cout<<"hello"<<endl;
}

void TestServer::ConnectionEvent(Conn *conn)
{
  TestServer *me = (TestServer*)conn->GetThread()->GetTcpConnect();
  printf("new connection: %d\n", conn->GetFd());
  me->vec.push_back(conn);
  
  conn->FlushBuffer(conn->getbev());

  char enterface[] = {"\t\twelcome to wpzz NetDisk\n \
  \t1.log in\n\t2.register\n\t3.quit\n"};
  conn->AddToWriteBuffer(enterface, strlen(enterface));
 
}

void TestServer::ConnectionEvent_conn(Conn *conn)
{
  TestServer *me = (TestServer*)conn->GetThread()->GetTcpConnect();
  printf("new connection: %d\n", conn->GetFd());
  me->vec.push_back(conn);

}
void TestServer::CloseEvent(Conn *conn, short events)
{
  printf("connection closed: %d\n", conn->GetFd());
}

void TestServer::QuitCb(int sig, short events, void *data)
{ 
  printf("Catch the SIGINT signal, quit in one second\n");
  TestServer *me = (TestServer*)data;
  timeval tv = {1, 0};
  me->StopRun(&tv);
}

void TestServer::TimeOutCb(int id, short events, void *data)
{
/*
  TestServer *me = (TestServer*)data;
  char temp[33] = "**&&^%$are you alive^&&%";
  for(int i=0; i<me->vec.size(); i++)
  {
    me->vec[i]->AddToWriteBuffer(temp, strlen(temp));
    me->vec[i]->setalive(false);
  }
*/
}

int main()
{
  printf("pid: %d\n", getpid());
  TestServer server(3, 3);
  server.AddSignalEvent(SIGINT, TestServer::QuitCb);
  timeval tv = {1, 0};
  server.AddTimerEvent(TestServer::TimeOutCb, tv, false);
  server.SetPort(2111);
  server.StartRun(0);
  printf("done\n");
  
  return 0;
}
