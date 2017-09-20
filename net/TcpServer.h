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

#include <map>
using std::map;

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include"conn.h"
#include"thread.h"

class TcpEventServer;
class Conn;
class ConnQueue;
class LibeventThread;
class ThreadPool;


class TcpEventServer
{
friend class ThreadPool;
friend class LibeventThread;
private:
  //int m_ThreadCount;					
  int m_Port;							
  LibeventThread *m_MainBase;			
  //LibeventThread *m_Threads;			
  ThreadPool* m_threadpool;
  map<int, event*> m_SignalEvents;

public:
  static const int EXIT_CODE = -1;

private:
  //void SetupThread(LibeventThread *thread);

  //static void *WorkerLibevent(void *arg);
  //static void ThreadProcess(int fd, short which, void *arg);
  static void ListenerEventCb(evconnlistener *listener, evutil_socket_t fd,
    sockaddr *sa, int socklen, void *user_data);
  static void ReadEventCb(struct bufferevent *bev, void *data);
  static void WriteEventCb(struct bufferevent *bev, void *data); 
  static void CloseEventCb(struct bufferevent *bev, short events, void *data);

//protected:
public:
  virtual void ConnectionEvent(Conn *conn) { }
  virtual void ConnectionEvent_conn(Conn *conn) { }

  virtual void ReadEvent(Conn *conn) { }

  virtual void WriteEvent(Conn *conn) { }

  virtual void CloseEvent(Conn *conn, short events) { }

  virtual void ErrorQuit(const char *str);

public:
  TcpEventServer(int count);
  ~TcpEventServer();

  void SetPort(int port)
  { m_Port = port; }

  bool StartRun(int flag);    //0:lb   1:server
  void StopRun(timeval *tv);

  bool AddSignalEvent(int sig, void (*ptr)(int, short, void*));
  bool DeleteSignalEvent(int sig);

  event *AddTimerEvent(void(*ptr)(int, short, void*),
    timeval tv, bool once);
  bool DeleteTImerEvent(event *ev);
};

