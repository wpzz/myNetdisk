#include "TcpServer.h"

TcpEventServer::TcpEventServer(int count)
{
  //m_ThreadCount = count;
  m_Port = -1;
  m_MainBase = new LibeventThread(this);
  //m_Threads = new LibeventThread[m_ThreadCount];
  //m_MainBase->tid = pthread_self();
  //m_MainBase->base = event_base_new();

  //for(int i=0; i<m_ThreadCount; i++)
  //{
  //  SetupThread(&m_Threads[i]);
  //}
  m_threadpool = new ThreadPool(this, count);

}

TcpEventServer::~TcpEventServer()
{
  StopRun(NULL);

  delete m_MainBase;
  delete m_threadpool;
}

void TcpEventServer::ErrorQuit(const char *str)
{
  fprintf(stderr, "%s", str);   
  if( errno != 0 )    
    fprintf(stderr, " : %s", strerror(errno));    
  fprintf(stderr, "\n");        
  exit(1);    
}

/*void TcpEventServer::SetupThread(LibeventThread *me)
{
  me->tcpConnect = this;
  me->base = event_base_new();
  if( NULL == me->base )
    ErrorQuit("event base new error");

  int fds[2];
  if( pipe(fds) )
    ErrorQuit("create pipe error");
  me->notifyReceiveFd = fds[0];
  me->notifySendFd = fds[1];

  event_set( &me->notifyEvent, me->notifyReceiveFd,
    EV_READ | EV_PERSIST, ThreadProcess, me );
  event_base_set(me->base, &me->notifyEvent);
  if ( event_add(&me->notifyEvent, 0) == -1 )
    ErrorQuit("Can't monitor libevent notify pipe\n");
}

void *TcpEventServer::WorkerLibevent(void *arg)
{
  LibeventThread *me = (LibeventThread*)arg;
  //printf("thread %u started\n", (unsigned int)me->tid);
  event_base_dispatch(me->base);
  //printf("subthread done\n");
}
*/

bool TcpEventServer::StartRun(int flag)
{
  evconnlistener *listener;

  if( m_Port != EXIT_CODE )
  {
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(m_Port);
    listener = evconnlistener_new_bind(m_MainBase->getbase(), 
      ListenerEventCb, (void*)this,
      LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
      (sockaddr*)&sin, sizeof(sockaddr_in));
    if( NULL == listener )
      ErrorQuit("TCP listen error");
  }
  /*
  for(int i=0; i<m_ThreadCount; i++)
  {
    pthread_create(&m_Threads[i].tid, NULL,  
      WorkerLibevent, (void*)&m_Threads[i]);
  }
  */
  m_threadpool->StartRun();

  if(flag == 0)
  {
	///////need to read .xml///////////////
  	int ser_setup = -1;
	  int thread_fd = m_threadpool->GetThread();
	  write(thread_fd, &ser_setup, sizeof(int));
	  int port = 6500;
	  printf("serport:%d\n", port);
	  write(thread_fd, &port, sizeof(int));
  }

  event_base_dispatch(m_MainBase->m_base);

  if( m_Port != EXIT_CODE )
  {
    //printf("free listen\n");
    evconnlistener_free(listener);
  }
}

void TcpEventServer::StopRun(timeval *tv)
{
/*
  int contant = EXIT_CODE;
  for(int i=0; i<m_ThreadCount; i++)
  {
    write(m_Threads[i].notifySendFd, &contant, sizeof(int));
  }
  */
  if(tv != NULL)
  {
  	m_threadpool->StopRun();

  	event_base_loopexit(m_MainBase->getbase(), tv);
  }
}

void TcpEventServer::ListenerEventCb(struct evconnlistener *listener, 
                  evutil_socket_t fd,
                  struct sockaddr *sa, 
                  int socklen, 
                  void *user_data)
{
  TcpEventServer *server = (TcpEventServer*)user_data;

  //int num = rand() % server->m_ThreadCount;
  int sendfd = server->m_threadpool->GetThread();
  //int sendfd = server->m_threads[num].notifySendFd;
  write(sendfd, &fd, sizeof(evutil_socket_t));
}

void TcpEventServer::ReadEventCb(struct bufferevent *bev, void *data)
{
  Conn *conn = (Conn*)data;
  conn->m_ReadBuf = bufferevent_get_input(bev);
  conn->m_WriteBuf = bufferevent_get_output(bev);

  conn->m_Thread->m_tcpConnect->ReadEvent(conn);
} 

void TcpEventServer::WriteEventCb(struct bufferevent *bev, void *data)
{
  Conn *conn = (Conn*)data;
  conn->m_ReadBuf = bufferevent_get_input(bev);
  conn->m_WriteBuf = bufferevent_get_output(bev);

  conn->m_Thread->m_tcpConnect->WriteEvent(conn);

}

void TcpEventServer::CloseEventCb(struct bufferevent *bev, short events, void *data)
{
  Conn *conn = (Conn*)data;
  conn->m_Thread->m_tcpConnect->CloseEvent(conn, events);
  conn->GetThread()->m_connectQueue->DeleteConn(conn);
  bufferevent_free(bev);
}

bool TcpEventServer::AddSignalEvent(int sig, void (*ptr)(int, short, void*))
{
  event *ev = evsignal_new(m_MainBase->getbase(), sig, ptr, (void*)this);
  if ( !ev || 
    event_add(ev, NULL) < 0 )
  {
    event_del(ev);
    return false;
  }

  DeleteSignalEvent(sig);
  m_SignalEvents[sig] = ev;

  return true;
}

bool TcpEventServer::DeleteSignalEvent(int sig)
{
  map<int, event*>::iterator iter = m_SignalEvents.find(sig);
  if( iter == m_SignalEvents.end() )
    return false;

  event_del(iter->second);
  m_SignalEvents.erase(iter);
  return true;
}

event *TcpEventServer::AddTimerEvent(void (*ptr)(int, short, void *), 
                  timeval tv, bool once)
{
  int flag = 0;
  if( !once )
    flag = EV_PERSIST;

  event *ev = new event;
  event_assign(ev, m_MainBase->getbase(), -1, flag, ptr, (void*)this);
  if( event_add(ev, &tv) < 0 )
  {
    event_del(ev);
    return NULL;
  }
  return ev;
}

bool TcpEventServer::DeleteTImerEvent(event *ev)
{
  int res = event_del(ev);
  return (0 == res);
}
