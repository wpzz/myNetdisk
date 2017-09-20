/////////////////////////////
//////////////////////////////
#include "../net/TcpServer.h"
#include <set>
#include <vector>
#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<assert.h>
#include<wait.h>
#include<sys/sendfile.h>
using namespace std;

class TestServer : public TcpEventServer
{
private:
  vector<Conn*> vec;
protected:
  void ReadEvent(Conn *conn);
  void WriteEvent(Conn *conn);
  void ConnectionEvent(Conn *conn);
  void CloseEvent(Conn *conn, short events);
public:
  TestServer(int count) : TcpEventServer(count) { }
  ~TestServer() { } 
  
  static void QuitCb(int sig, short events, void *data);
  static void TimeOutCb(int id, int short events, void *data);
};

void shell_deal(Conn* conn, char *buff)
{
	    char *myarg[128] = {0};
	    int i = 0;
	    char *p = strtok(buff, " ");
	    if(p == NULL)
	    {
	       return;
	    }

	    while(p != NULL)
	    {
	        myarg[i++] = p;
		p = strtok(NULL, " ");
	    }

	    int f_pipe[2];
	    pipe(f_pipe);
	    pid_t pid = fork();
	    assert(pid != -1);

	    if(pid == 0)
	    {
	        close(f_pipe[0]);
		dup2(f_pipe[1], 1);
		dup2(f_pipe[1], 2);

		execvp(myarg[0], myarg);
		perror("exec error");
		exit(0);
	    }
	    close(f_pipe[1]);
	    wait(NULL);
            char send_buff[1024] = {"ok#"};
	    memset(send_buff+3, 0, 1021);
            int s_num = read(f_pipe[0], send_buff+3, 1021);

	    do{
       	 	// send(c, file_buff, num, 0);
		conn->AddToWriteBuffer(send_buff, s_num+3);
		memset(send_buff, 0, 1024);
    	    }while((s_num = read(f_pipe[0], send_buff, 1024)));
}

void down_deal(Conn* conn, char *buff)
{

    int fd = open(buff, O_RDONLY);
    if(fd == -1)
    {
        char s_error[128] = "file open fail\n";
	conn->AddToWriteBuffer(s_error, strlen(s_error));
	return;
    }
    struct stat st;
    fstat(fd, &st);
    int file_size = st.st_size;
    char tmp[128] = {0};
    sprintf(tmp, "%d", file_size);
    send(conn->GetFd(), tmp, 128, 0);
    //conn->AddToWriteBuffer(tmp, 128);
    char file_buff[10240] = {0};
    int num = 0;
    int i = 0;
    
    while((num = read(fd, file_buff, 10240)))
    {
    /*
    //   int size = send(conn->GetFd(), file_buff, num, 0);
     //      sleep(1);
     //  i+= size;
     //  printf("%d   %d\n", size, i);
 
        conn->AddToWriteBuffer(file_buff, num);
//printf("%d\n", i+=num);
	memset(file_buff, 0, 10240);
    }*/
    
    int ret = 0;
    int send_size = 0;
    while(send_size != file_size){
        ret = sendfile(conn->GetFd(), fd, NULL, 10000);
	if(ret==-1 && errno == EAGAIN)
		printf("EAGAIN\n");
	else
		send_size += ret;
        printf("sendfile: %d  send_size: %d\n", ret, send_size);
    }
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
  if(strncmp(buff, "down", 4) == 0)
  {
      down_deal(conn, buff + 5);
  }
  else if(strcmp(buff,"**&&^%$are you alive^&&%") == 0)
  {
      char temp[33] = "&*$%&*(*i am alive%^#&(";
      conn->AddToWriteBuffer(temp, strlen(temp));
  }
  else
  {
      shell_deal(conn, buff);
  }
}

void TestServer::WriteEvent(Conn *conn)
{
   //cout<<endl<<"write event:   "<<getpid()<<endl<<endl;
}

void TestServer::ConnectionEvent(Conn *conn)
{
  TestServer *me = (TestServer*)conn->GetThread()->GetTcpConnect();
  printf("new connection: %d  :%u\n", conn->GetFd(), pthread_self());
  me->vec.push_back(conn);
}

void TestServer::CloseEvent(Conn *conn, short events)
{
  printf("connection closed: %d   :%d\n", conn->GetFd(), getpid());
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
 // TestServer *me = (TestServer*)data;
 // char temp[33] = "hello, world\n";
 // for(int i=0; i<me->vec.size(); i++)
 //   me->vec[i]->AddToWriteBuffer(temp, strlen(temp));
}

int main()
{
  printf("pid: %d\n", getpid());
  TestServer server(4);
  server.AddSignalEvent(SIGINT, TestServer::QuitCb);
  timeval tv = {10, 0};
  server.AddTimerEvent(TestServer::TimeOutCb, tv, false);
  server.SetPort(6500);
  server.StartRun(1);
  printf("done\n");
  
  return 0;
}
