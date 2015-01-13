
#ifndef _TS_QUEUE_H_
#define _TS_QUEUE_H_

#include "ddp_general.h"


/* define of queue length
 * -1 means no limit
 */
#define TS_QUEUE_LEN_UNLIMIT (-1)
#define TS_QUEUE_LEN_DEFAULT 100

#ifdef __cplusplus
extern "C" {
#endif

/* ts_node
 *   struct to represent a node in queue
 *
 *   data : payload carried by the ts_node
 *   next : pointer to the next ts_node struct
 */
struct ts_node
{
    UINT1* data;
    struct ts_node* next;
};

/* ts_queue
 *   struct represent a queue
 *   it implements generic behavior of a thread-safe queue
 *   Developer may implement specific-purpose queue based on ts_queue
 *
 *   head : head of the queue
 *   tail : tail of the queue
 *   lock : lock to synchronize thread access
 *   name : queue name
 */
struct ts_queue
{
    struct ts_node* head;
    struct ts_node* tail;
    INT4 max_length;
    INT4 cur_length;
    pthread_mutex_t lock;
    INT1 *name;
};

/* ts_queue_insert_node
 *   function to push a ts_node with data payload to queue
 *
 *   q : the queue
 *   data : data payload
 *
 *   return : 0 -> success, others -> error
 */
INT4 ts_queue_insert_node(struct ts_queue* q, UINT1* data);
/* ts_queue_remove_node
 *   function to pop a ts_node from queue
 *
 *   q : the queue
 *
 *   return : a pointer to the pop ts_node or NULL (nothing returned)
 */
struct ts_node* ts_queue_remove_node(struct ts_queue* q);

/* ts_queue_new
 *   function to create a ts_queue with the input name
 *
 *   name : a string
 *
 *   return : pointer to newly-createde ts_queue of NULL (fail)
 */
struct ts_queue* ts_queue_new(INT1* name);
/* ts_queue_free
 *   function to release ts_queue
 *
 *   q : the q to be freed
 *
 *   return : NULL (success to free) or pointer to q (fail)
 */
struct ts_queue* ts_queue_free(struct ts_queue* q);
/* ts_queue_set_length
 *   function to set length of ts queue in unit of node
 *
 *   q : the q to be set max length
 *   len : new max length
 *
 *   return : 0 -> success, others -> error
 */
 INT4 ts_queue_set_length(struct ts_queue* q, INT4 len);

/* ts_queue_display_all
 *   function to display all nodes on queue
 *   it is used for debug
 *
 *   q : the q
 *
 *   return : none
 */
void ts_queue_display_all(struct ts_queue* q);

#ifdef __cplusplus
}
#endif

#endif /* _TS_QUEUE_H_ */
