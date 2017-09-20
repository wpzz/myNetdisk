//TcpEventServer.cpp
#include "thread.h"

#define SER_SETUP -1
#define EXIT_CODE -2


LibeventThread::LibeventThread(TcpEventServer * tcpConnect):m_tcpConnect(tcpConnect)
{
  //m_db = new CMyDb();
  //m_db.initDB("localhost", "root", "111111", "yunpan");
  //m_db.executeSQL("select * from client");

  m_connectQueue = new ConnQueue();
  m_base = event_base_new();
  if( NULL == m_base )
    m_tcpConnect->ErrorQuit("event base new error");

  int fds[2];
  if( pipe(fds) )
    m_tcpConnect->ErrorQuit("create pipe error");
  m_notifyReceiveFd = fds[0];
  m_notifySendFd = fds[1];

  event_set( &m_notifyEvent, m_notifyReceiveFd,
    EV_READ | EV_PERSIST, ThreadProcess, this);
  event_base_set(m_base, &m_notifyEvent);
  if ( event_add(&m_notifyEvent, 0) == -1 )
    m_tcpConnect->ErrorQuit("Can't monitor libevent notify pipe\n");
}

LibeventThread::LibeventThread()
{
 // m_db = new CMyDb();
 // m_db.initDB("localhost", "root", "111111", "yunpan");
 // m_db.executeSQL("select * from client");

  m_connectQueue = new ConnQueue();
  m_base = event_base_new();
  if( NULL == m_base )
    m_tcpConnect->ErrorQuit("event base new error");

  int fds[2];
  if( pipe(fds) )
    m_tcpConnect->ErrorQuit("create pipe error");
  m_notifyReceiveFd = fds[0];
  m_notifySendFd = fds[1];

  event_set( &m_notifyEvent, m_notifyReceiveFd,
    EV_READ | EV_PERSIST, ThreadProcess, this);
  event_base_set(m_base, &m_notifyEvent);
  if ( event_add(&m_notifyEvent, 0) == -1 )
    m_tcpConnect->ErrorQuit("Can't monitor libevent notify pipe\n");
}

LibeventThread::~LibeventThread()
{
  //delete m_db;
  delete m_connectQueue;
  event_base_free(m_base);
  close(m_notifyReceiveFd);
  close(m_notifySendFd);
}


void LibeventThread::ThreadProcess(int fd, short which, void *arg)
{
  LibeventThread *me = (LibeventThread *)arg;
  int pipefd = me->m_notifyReceiveFd;
  evutil_socket_t confd;
  read(pipefd, &confd, sizeof(evutil_socket_t));

  if( EXIT_CODE == confd )
  {
    event_base_loopbreak(me->m_base);
    return;
  }
  if( SER_SETUP == confd )
  {
     //////////need recv sockaddr//////////
     int port;
     read(pipefd, &port, sizeof(evutil_socket_t));
     struct sockaddr_in sin;
     memset(&sin, 0, sizeof(sin));
     sin.sin_family = AF_INET;
     //sin.sin_addr.s_addr = htonl(0x7f000001); //120.0.0.1
     sin.sin_addr.s_addr = inet_addr("127.0.0.1"); //120.0.0.1
     sin.sin_port = htons(port);

    confd = socket(PF_INET, SOCK_STREAM, 0);
    assert(confd != -1);

    int res = connect(confd, (struct sockaddr*)&sin, sizeof(sin));
     if(res == -1){
    	fprintf(stderr, "Error connect server!");
	return;
     }
     /*confd = bufferevent_socket_connect(bev, 
              (struct sockaddr*)&sin, sizeof(sin));
     if(confd < 0){

    	fprintf(stderr, "Error connect server!");
    	event_base_loopbreak(me->m_base);
	return;
     }*/
     struct bufferevent *bev;
     bev = bufferevent_socket_new(me->m_base, confd, BEV_OPT_CLOSE_ON_FREE);
     if (!bev)
     {
        fprintf(stderr, "Error constructing bufferevent!");
        event_base_loopbreak(me->m_base);
        return;
      }
      Conn *conn = me->m_connectQueue->InsertConn(confd, me);
      bufferevent_setcb(bev, me->m_tcpConnect->ReadEventCb, 
	  me->m_tcpConnect->WriteEventCb, me->m_tcpConnect->CloseEventCb, conn);
      bufferevent_enable(bev, EV_WRITE);
      bufferevent_enable(bev, EV_READ);
  
      conn->setbev(bev);
       
      me->m_tcpConnect->ConnectionEvent_conn(conn);
  }
  else {
  	struct bufferevent *bev;
	  bev = bufferevent_socket_new(me->m_base, confd, BEV_OPT_CLOSE_ON_FREE);
	  if (!bev)
	  {
	    fprintf(stderr, "Error constructing bufferevent!");
	    event_base_loopbreak(me->m_base);
	    return;
	  }
	  Conn *conn = me->m_connectQueue->InsertConn(confd, me);
	  bufferevent_setcb(bev, me->m_tcpConnect->ReadEventCb, 
		  me->m_tcpConnect->WriteEventCb, me->m_tcpConnect->CloseEventCb, conn);
	  bufferevent_enable(bev, EV_WRITE);
	  bufferevent_enable(bev, EV_READ);
  
	  conn->setbev(bev);

	  me->m_tcpConnect->ConnectionEvent(conn);
  }
}

void LibeventThread::Setup()
{
    pthread_create(&m_tid, NULL, WorkerLibevent, this);
}


void* LibeventThread::WorkerLibevent(void *arg)
{
  LibeventThread *me = (LibeventThread*)arg;
  //printf("thread %u started\n", (unsigned int)me->tid);
  event_base_dispatch(me->m_base);
  //printf("subthread done\n");
}

ThreadPool::ThreadPool(TcpEventServer* tcpConnect, int count)
                      :m_ThreadCount(count)
{
  m_threads = new LibeventThread[count];
  for(int i=0; i<count; i++){
     //LibeventThread *lthread  = new LibeventThread(tcpConnect);
     //m_vec.insert(*lthread);
     //m_threads[0][i] = new LibeventThread(tcpConnect);
     m_threads[i].SetTcpConnect(tcpConnect);
  }
}
ThreadPool::~ThreadPool()
{
  //for(int i=0; i<m_ThreadCount; i++){
     delete[] m_threads;
  //}
}

void ThreadPool::StartRun()
{
  for(int i=0; i<m_ThreadCount; i++){
     m_threads[i].Setup();
  }
}

void ThreadPool::StopRun()
{
  int contant = EXIT_CODE;
  for(int i=0; i<m_ThreadCount; i++)
  {
    write(m_threads[i].m_notifySendFd, &contant, sizeof(int));
  }
  //event_base_loopexit(m_MainBase->base, tv);
}

int ThreadPool::GetThread()
{
  int num = rand() % m_ThreadCount;
  return m_threads[num].m_notifySendFd;
}
    
//////////////////////////
/////////////////////////
////////////////////////
//////////////////////////
