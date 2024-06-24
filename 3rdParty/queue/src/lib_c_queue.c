#include "lib_c_queue.h"

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef float  f32;
typedef double f64;

typedef char INT8;
typedef short INT16;
typedef int INT32;
typedef long long INT64;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
#define NULL ((void *)0)

#define DEBUGMSG(...)       do{\
                                printf(__VA_ARGS__);\
                            }while(0)



Q_Status QUEUE_IsEmpty(Sq_Queue *Q)
{
	if(NULL == Q)
		return QUEUE_FALSE;
	if(Q->front == Q->rear) 
		return QUEUE_TRUE;
	
	return QUEUE_FALSE;
}


Q_Status QUEUE_IsFull(Sq_Queue *Q)
{
	if(NULL == Q)
		return QUEUE_FALSE;
	if((Q->rear + 1) % Q->maxLen == Q->front) 
		return QUEUE_TRUE;
	
	return QUEUE_FALSE;
}

Q_Status QUEUE_EnQueue(Sq_Queue *Q, void *e)
{			
	if(NULL == Q  || NULL == e)
	{
		DEBUGMSG("EnQueue pointer NULL err\r\n");
		return QUEUE_FALSE;
	}

#ifdef GSP_QT_SUPPORT	

    Q->gmutex->lock();
#endif
	qu32 size = Q->typeSize;
	s8 *s  = NULL;
	s8 *d  = NULL;
	qu32 front;
	qu32 rear = Q->rear;
	
	if(QUEUE_IsFull(Q) == QUEUE_TRUE) 
	{
		//DEBUGMSG("EnQueue QUEUE_IsFull err\r\n");
#ifdef GSP_QT_SUPPORT		
    	Q->gmutex->unlock();
#endif
		return QUEUE_FALSE;
	}

	s  = (s8 *)e;
	d  = (s8 *)((qu32)Q->base + rear * size);
	
	memcpy(d, s, size);

	s = d = NULL;
	Q->rear = (Q->rear+1) % Q->maxLen;
#ifdef GSP_QT_SUPPORT	
	Q->used++;
    Q->gmutex->unlock();
#else
	rear = Q->rear;
	front = Q->front;
	if(rear >= front)
		Q->used = rear - front;
	else
		Q->used = rear + Q->maxLen - front;
#endif
	return QUEUE_TRUE;
}

Q_Status QUEUE_DeQueue(Sq_Queue *Q, void *e ,qu32 loc)
{
	if(NULL == Q  || NULL == e)
		return QUEUE_FALSE;
	
#ifdef GSP_QT_SUPPORT
	//����
    Q->gmutex->lock();
#endif	
	qu32 size = Q->typeSize;	
	s8 *d  = NULL;
	qu32 pos = loc;
	qu32 front = (Q->front+pos)%Q->maxLen;
	qu32 rear;
	s8 *s  = NULL;
	s8 *s1 = NULL;

	if(QUEUE_IsEmpty(Q) == QUEUE_TRUE) 
	{
#ifdef GSP_QT_SUPPORT	
		// ����
		Q->gmutex->unlock();
#endif
		//DEBUGMSG("DeQueue QUEUE_IsEmpty err\r\n");
		return QUEUE_FALSE;
	}


	if(pos > Q->used)
	{
#ifdef GSP_QT_SUPPORT
		// ����
    	Q->gmutex->unlock();
#endif
		//DEBUGMSG("DeQueue pos err\r\n");
		return QUEUE_FALSE;
	}
	
	d = (s8 *)e;
	s = (s8 *)Q->base + front * size;
    memcpy(d, s, size);
	
	while(pos--)
	{
		s = (s8 *)Q->base + front * size;
		if(front>0)
		{
			front--;
		}
		else
		{
			front = Q->maxLen-1;
		}
		s1 = (s8 *)Q->base + front * size;
		
		
	    memcpy(s, s1, size);
	}
	d = s = s1 = NULL;
	Q->front = (Q->front + 1) % Q->maxLen;
#ifdef GSP_QT_SUPPORT		
	Q->used--;
	// ����
    Q->gmutex->unlock();
#else
	rear = Q->rear;
	front = Q->front;
	if(rear >= front)
		Q->used = rear - front;
	else
		Q->used = rear + Q->maxLen - front;
#endif
	return QUEUE_TRUE;
}

/*Q_Status QUEUE_TryQueue(Sq_Queue *Q, void *e,  qu32 count)
{
	qu32 size = Q->typeSize;
	gsp_s8 *d  = (gsp_s8 *)e;
	gsp_s8 *s  = (gsp_s8 *)Q->base + Q->front * size;

	if (QUEUE_IsEmpty(Q) == QUEUE_TRUE) return QUEUE_EMPTY;
	if (count > Q->used) return QUEUE_ERROR;
	
	while(count--)
	{	size = Q->typeSize;
		while(size-- > 0) *d++ = *s++;
	}
	
	d = s = NULL;
	
	return QUEUE_TRUE;
}
*/

Q_Status QUEUE_Destroy(Sq_Queue *Q)
{
	if(NULL == Q)
		return QUEUE_FALSE;
#ifdef GSP_QT_SUPPORT
	//����
    Q->gmutex->lock();
#endif
	free(Q->base);
	Q->base = NULL;
	Q->front = Q->rear = 0;
	Q->maxLen = Q->used = Q->typeSize = 0;
#ifdef GSP_QT_SUPPORT
	//����
    Q->gmutex->unlock();
#endif	
	return QUEUE_TRUE;
}

Q_Status QUEUE_Clear(Sq_Queue *Q)
{
	if(NULL == Q)
		return QUEUE_FALSE;

#ifdef GSP_QT_SUPPORT
    Q->gmutex->lock();
#endif	
	memset(Q->base, 0, Q->maxLen * Q->typeSize);
	Q->front = Q->rear = 0;	
	Q->used = 0;
#ifdef GSP_QT_SUPPORT		
    Q->gmutex->unlock();
#endif		
	return QUEUE_TRUE;
}

qu32 QUEUE_Traverse(Sq_Queue *Q, void *buf ,qu32 length)
{
	if(NULL == Q || NULL == buf)
		return QUEUE_FALSE;

#ifdef GSP_QT_SUPPORT
	Q->gmutex->lock();
#endif

	qu32 used = Q->used;
	qu32 size = Q->typeSize;	
	s8 *d  = (s8 *)buf;
	qu32 front = Q->front;
	s8 *s  = NULL;
	qu32 len = length;
	
	if(used > len)
	{
		used = len;
	}
	len = used;
	while(used--)
	{
		s  = (s8 *)Q->base + front * size;
	    memcpy(d, s, size);
		
		d += size;
		front = (front + 1) % Q->maxLen;
	}
	d = s = NULL;
#ifdef GSP_QT_SUPPORT		
	// ����
    Q->gmutex->unlock();
#endif		
	return len;
}

Q_Status QUEUE_GetIndex(Sq_Queue *Q, void *e ,qu32 loc)
{
	if(NULL == Q || NULL == e)
		return QUEUE_FALSE;

#ifdef GSP_QT_SUPPORT
	Q->gmutex->lock();
#endif

	qu32 size = Q->typeSize; 
	s8 *d  = NULL;
	qu32 pos = loc;
	qu32 front = (Q->front+pos)%Q->maxLen;
	s8 *s  = NULL;

	if(QUEUE_IsEmpty(Q) == QUEUE_TRUE) 
	{
		#ifdef GSP_QT_SUPPORT		
    	Q->gmutex->unlock();
		#endif	
		return QUEUE_FALSE;
	}
	
	if(pos > Q->used)
	{
		#ifdef GSP_QT_SUPPORT		
	    Q->gmutex->unlock();
		#endif	
		return QUEUE_FALSE;
	}
	d = (s8 *)e;
	s = (s8 *)Q->base + front * size;
	memcpy(d, s, size);
	d = s = NULL;
#ifdef GSP_QT_SUPPORT		
    Q->gmutex->unlock();
#endif	
	return QUEUE_TRUE;
}


