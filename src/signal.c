#include "signal.h"


/* Private function prototypes */
static int isempty(SigQ q);

/* isempty
   Is a queue empty?
*/
int isempty(SigQ q)
{
  return(q.front == NULL);
}

/* sig_poll
   Removes the signal from the top of the queue and places it in s.  Returns
   0 if the queue is empty.
*/
int sig_poll(SigQ *q, Signal *s)
{
  
  if (!isempty(*q))
    {
      Sig_Link temp = q->front;
      *s = temp->s;
      q->front = temp->next;
      free(temp);
    }
  else return 0;
}

/* sig_push
   Pushes a signal onto the end of a queue.
*/
void sig_push(SigQ *q, Signal *s)
{
  Sig_Link temp;

  MALLOC(temp, sizeof(Sig_Element));
  temp->s = *s;
  temp->next = NULL;
  if (isempty(*q))
    {
      q->front = q->rear = temp;
    }
  else
    {
      q->rear->next = temp;
      q->rear = temp;
    }
}

/* sig_flush
   Flushes a queue (recursively).
*/
void sig_flush(SigQ *q)
{
  if (!isempty(*q))
    {
      Sig_Link temp = q->front;
      q->front = temp->next;
      free(temp);
      sig_flush(q);
    }
}

/* sig_initQ
   Sets a queue's pointers to NULL.
*/
void sig_initQ(SigQ *q)
{
  q->front = q->rear = NULL;
}
