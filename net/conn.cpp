#include"conn.h"

Conn::Conn(int fd) : m_fd(fd), m_isalive(true)
{
  m_Prev = NULL;
  m_Next = NULL;
}

Conn::~Conn()
{

}

ConnQueue::ConnQueue()
{
  m_head = new Conn(0);
  m_tail = new Conn(0);
  m_head->m_Prev = m_tail->m_Next = NULL;
  m_head->m_Next = m_tail;
  m_tail->m_Prev = m_head;
}

ConnQueue::~ConnQueue()
{
  Conn *tcur, *tnext;
  tcur = m_head;
  while( tcur != NULL )
  {
    tnext = tcur->m_Next;
    delete tcur;
    tcur = tnext;
  }
}

Conn *ConnQueue::InsertConn(int fd, LibeventThread *t)
{
  Conn *c = new Conn(fd);
  c->m_Thread = t;
  Conn *next = m_head->m_Next;

  c->m_Prev = m_head;
  c->m_Next = m_head->m_Next;
  m_head->m_Next = c;
  next->m_Prev = c;
  return c;
}

void ConnQueue::DeleteConn(Conn *c)
{
  c->m_Prev->m_Next = c->m_Next;
  c->m_Next->m_Prev = c->m_Prev;
  delete c;
}

/*
void ConnQueue::PrintQueue()
{
  Conn *cur = m_head->m_Next;
  while( cur->m_Next != NULL )
  {
    printf("%d ", cur->m_fd);
    cur = cur->m_Next;
  }
  printf("\n");
}
*/

