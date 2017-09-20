#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include<arpa/inet.h>
#include<assert.h>

#include"../mysql/cmydb.h"

#include <map>
#include<vector>
using std::map;
using std::vector;

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include"conn.h"
#include"TcpServer.h"

class TcpEventServer;
class Conn;
class ConnQueue;
class LibeventThread;

class LibeventThread
{
friend class ThreadPool;
friend class TcpEventServer;

private:
  pthread_t m_tid;				
  struct event_base *m_base;	
  struct event m_notifyEvent;	
  int m_notifyReceiveFd;		
  int m_notifySendFd;			
  ConnQueue* m_connectQueue;		

  //CMyDb *m_db;
  
  static void ThreadProcess(int fd, short which, void *arg);
  static void* WorkerLibevent(void *);

  TcpEventServer * m_tcpConnect;	
public:
  LibeventThread(TcpEventServer *const tcpConnect);
  LibeventThread();
  ~LibeventThread();
  
  int getpid() {   return m_tid;   }

  void Setup();

  void SetTcpConnect(TcpEventServer * tcpConnect){
      m_tcpConnect = tcpConnect;
  }
  TcpEventServer* GetTcpConnect(){
      return m_tcpConnect;
  }

  struct event_base* getbase(){
      return m_base;
  }
};
//void ThreadProcess(int fd, short which, void *arg);

class ThreadPool
{
private:
  int m_ThreadCount;
  //vector<LibeventThread > m_vec;
  LibeventThread *m_threads;
public:
  ThreadPool(TcpEventServer* tcpConnect, int m_count);
  ~ThreadPool();

  void StartRun();
  void StopRun();
  int GetThread();
};


