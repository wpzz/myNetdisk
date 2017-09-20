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

#include"TcpServer.h"
#include"thread.h"

class Conn
{
  friend class ConnQueue;
  friend class TcpEventServer;

private:
  const int m_fd;			
  evbuffer *m_ReadBuf;		
  evbuffer *m_WriteBuf;		

  struct bufferevent *m_bev;

  Conn *m_Prev;				
  Conn *m_Next;				
  LibeventThread *m_Thread;

  Conn(int fd=0);
  ~Conn();

  bool m_isalive;

public:
  void setalive(bool isalive){ m_isalive = isalive; }
  bool getalive(){  return m_isalive;  }

  void setbev(struct bufferevent* bev){ m_bev = bev; }

  struct bufferevent* getbev(){ return m_bev; }

  void FlushBuffer(struct bufferevent *bev)
  {
     m_ReadBuf = bufferevent_get_input(bev);
     m_WriteBuf = bufferevent_get_output(bev);
  }

  LibeventThread *GetThread() { return m_Thread; }
  int GetFd() { return m_fd; }

  int GetReadBufferLen()
  { return evbuffer_get_length(m_ReadBuf); }

  int GetReadBuffer(char *buffer, int len)
  { return evbuffer_remove(m_ReadBuf, buffer, len); }

  int CopyReadBuffer(char *buffer, int len)
  { return evbuffer_copyout(m_ReadBuf, buffer, len); }
  
  int GetWriteBufferLen()
  { return evbuffer_get_length(m_WriteBuf); }

  int AddToWriteBuffer(char *buffer, int len)
  { return evbuffer_add(m_WriteBuf, buffer, len); }

  void MoveBufferData()
  { evbuffer_add_buffer(m_WriteBuf, m_ReadBuf); }

};

class ConnQueue
{
private:
  Conn *m_head;
  Conn *m_tail;
public:
  ConnQueue();
  ~ConnQueue();
  Conn *InsertConn(int fd, LibeventThread *t);
  void DeleteConn(Conn *c);
  //void PrintQueue();
};
