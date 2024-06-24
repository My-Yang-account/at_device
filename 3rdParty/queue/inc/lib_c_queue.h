//queue.h
#ifdef GSP_QT_SUPPORT
#include <QMutex>
#endif

#ifndef LIB_QUEUE_H
#define LIB_QUEUE_H


typedef unsigned long  qu32;

#ifdef GSP_QT_SUPPORT
#define Q_INIT(q,type,size)\
	do\
	{	q.front  = q.rear = q.used  = 0;\
		q.typeSize = sizeof(type);\
		q.base = (type *)malloc(size*q.typeSize);\
		memset(q.base, 0, size*q.typeSize);\
		q.maxLen = size;\
		q.used	= 0;\
		q.gmutex = new QMutex;\
		q.EnQueue = QUEUE_EnQueue;\
		q.DeQueue = QUEUE_DeQueue;\
		q.IsEmpty = QUEUE_IsEmpty;\
		q.IsFull  = QUEUE_IsFull;\
		q.Clear   = QUEUE_Clear;\
		q.Traverse= QUEUE_Traverse;\
		q.GetIndex= QUEUE_GetIndex;\
		q.Destroy = QUEUE_Destroy;\
	} while (0);
#define Q_INIT_BUF(q,type,buf,size)\
		do\
		{	q.front  = q.rear = q.used	= 0;\
			q.typeSize = sizeof(type);\
			q.base = buf;\
			memset(q.base, 0, size*q.typeSize);\
			q.maxLen = size;\
			q.used	= 0;\
			q.gmutex = new QMutex;\
			q.EnQueue = QUEUE_EnQueue;\
			q.DeQueue = QUEUE_DeQueue;\
			q.IsEmpty = QUEUE_IsEmpty;\
			q.IsFull  = QUEUE_IsFull;\
			q.Clear   = QUEUE_Clear;\
			q.Traverse= QUEUE_Traverse;\
			q.GetIndex= QUEUE_GetIndex;\
			q.Destroy = QUEUE_Destroy;\
		} while (0);

#define Q_INIT_BUF_LEN(q,typesize,buf,size)\
		do\
		{	q.front  = q.rear = q.used	= 0;\
			q.typeSize = typesize;\
			q.base = buf;\
			memset(q.base, 0, size*q.typeSize);\
			q.maxLen = size;\
			q.used	= 0;\
			q.gmutex = new QMutex;\
			q.EnQueue = QUEUE_EnQueue;\
			q.DeQueue = QUEUE_DeQueue;\
			q.IsEmpty = QUEUE_IsEmpty;\
			q.IsFull  = QUEUE_IsFull;\
			q.Clear   = QUEUE_Clear;\
			q.Traverse= QUEUE_Traverse;\
			q.GetIndex= QUEUE_GetIndex;\
			q.Destroy = QUEUE_Destroy;\
		} while (0);
#else
#define Q_INIT(q,type,size)\
	do\
	{	q.front  = q.rear = q.used  = 0;\
		q.typeSize = sizeof(type);\
		q.base = (type *)malloc(size*q.typeSize);\
		memset(q.base, 0, size*q.typeSize);\
		q.maxLen = size;\
		q.used	= 0;\
		q.EnQueue = QUEUE_EnQueue;\
		q.DeQueue = QUEUE_DeQueue;\
		q.IsEmpty = QUEUE_IsEmpty;\
		q.IsFull  = QUEUE_IsFull;\
		q.Clear   = QUEUE_Clear;\
		q.Traverse= QUEUE_Traverse;\
		q.GetIndex= QUEUE_GetIndex;\
		q.Destroy = QUEUE_Destroy;\
	} while (0);
#define Q_INIT_BUF(q,type,buf,size)\
		do\
		{	q.front  = q.rear = q.used	= 0;\
			q.typeSize = sizeof(type);\
			q.base = buf;\
			memset(q.base, 0, size*q.typeSize);\
			q.maxLen = size;\
			q.used	= 0;\
			q.EnQueue = QUEUE_EnQueue;\
			q.DeQueue = QUEUE_DeQueue;\
			q.IsEmpty = QUEUE_IsEmpty;\
			q.IsFull  = QUEUE_IsFull;\
			q.Clear   = QUEUE_Clear;\
			q.Traverse= QUEUE_Traverse;\
			q.GetIndex= QUEUE_GetIndex;\
			q.Destroy = QUEUE_Destroy;\
		} while (0);

#define Q_INIT_BUF_LEN(q,typesize,buf,size)\
		do\
		{	q.front  = q.rear = q.used	= 0;\
			q.typeSize = typesize;\
			q.base = buf;\
			memset(q.base, 0, size*q.typeSize);\
			q.maxLen = size;\
			q.used	= 0;\
			q.EnQueue = QUEUE_EnQueue;\
			q.DeQueue = QUEUE_DeQueue;\
			q.IsEmpty = QUEUE_IsEmpty;\
			q.IsFull  = QUEUE_IsFull;\
			q.Clear   = QUEUE_Clear;\
			q.Traverse= QUEUE_Traverse;\
			q.GetIndex= QUEUE_GetIndex;\
			q.Destroy = QUEUE_Destroy;\
		} while (0);

#endif

typedef enum queue_status		
{
	QUEUE_FALSE=0, QUEUE_TRUE
}Q_Status;



typedef struct queue	 
{
	void		*base;
	qu32		front;
	qu32		rear;
	qu32		maxLen;
	qu32		used;
	qu32		typeSize;
#ifdef GSP_QT_SUPPORT	
    QMutex *gmutex;
#endif
	Q_Status (*EnQueue)(struct queue *Q, void *e);
	Q_Status (*DeQueue)(struct queue *Q, void *e, qu32 loc);
	Q_Status (*IsEmpty)(struct queue *Q);
	Q_Status (*IsFull) (struct queue *Q);
	Q_Status (*Destroy)(struct queue *Q);
	Q_Status (*Clear)(struct queue *Q);
	qu32 (*Traverse)(struct queue *Q, void *buf ,qu32 length);
	Q_Status (*GetIndex)(struct queue *Q, void *e, qu32 loc);
}Sq_Queue;




Q_Status QUEUE_EnQueue(Sq_Queue *Q, void *e);

Q_Status QUEUE_DeQueue(Sq_Queue *Q, void *e ,qu32 loc);

Q_Status QUEUE_IsEmpty(Sq_Queue *Q);

Q_Status QUEUE_IsFull(Sq_Queue *Q);

Q_Status QUEUE_Destroy(Sq_Queue *Q);

Q_Status QUEUE_Clear(Sq_Queue *Q);

qu32 QUEUE_Traverse(Sq_Queue *Q, void *buf ,qu32 length);

Q_Status QUEUE_GetIndex(Sq_Queue *Q, void *e ,qu32 loc);





#endif

