#pragma once

//
//	/include/linux/wait.h
//

//struct __wait_queue_head {int dumm;};
//typedef struct __wait_queue_head wait_queue_head_t;

struct __wait_queue_head {void *dummy; };
typedef struct __wait_queue_head wait_queue_head_t;

static __inline void init_waitqueue_head(wait_queue_head_t *q)
{
	//q->lock = WAITQUEUE_RW_LOCK_UNLOCKED;
	//INIT_LIST_HEAD(&q->task_list);
}
