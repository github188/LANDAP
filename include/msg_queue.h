
#ifndef _MSG_QUEUE_H_
#define _MSG_QUEUE_H_

#include <stdio.h>

#include "ddp_general.h"
#include "ts_queue.h"

/* define for msg queue length */
#define MSG_QUEUE_LEN_UNLIMIT TS_QUEUE_LEN_UNLIMIT
#define MSG_QUEUE_LEN_DEFAULT TS_QUEUE_LEN_DEFAULT

#ifdef __cplusplus
extern "C" {
#endif

/* Message queue between recv thread and proc thread.
 * 3 parties (recv thread, proc thread, and report timer) would access this queue.
 */

/* msg_queue
 *   struct represent msg queue
 *
 *   q : ts_queue
 */
struct msg_queue {
    struct ts_queue* q;
};

/* msg_queue_init
 *   function to create a ts_queue which is used as msg_queue
 *
 *   name : queue name
 *
 *   return : pointer to msg_queue (suucess) or NULL (fail)
 */
struct msg_queue* msg_queue_init(INT1* name);
/* msg_queue_free
 *   fucntion to release msg_queue
 *
 *   mq : the queue
 *
 *   return : 0 -> success, others -> error
 */
INT4 msg_queue_free(struct msg_queue* mq);

/* msg_queue_insert_msg
 *   function to push data into the msg queue
 *
 *   mq : the queue to be inserted
 *   msg : new msg payload
 *
 *   return : 0 -> success, others ->error
 */
INT4 msg_queue_insert_msg(struct msg_queue* mq, UINT1* msg);
/* msg_queue_remove_msg
 *   function to popup a msg from msg queue
 *
 *   mq : the queue
 *
 *   return : the msg payload carried in the popup node or NULL (fail)
 */
UINT1* msg_queue_remove_msg(struct msg_queue* mq);
/* msg_queue_set_length
 *   function to set length of msg queue in the unit of node
 *   please had beter set the length before use the queue
 *
 *   mq : the queue
 *   len : the queue length.
 *         positive -> set limit, -1 -> unlimited, 0 is not acceptable
 *
 *   return : 0 -> success, others ->error
 */
INT4 msg_queue_set_length(struct msg_queue* mq, INT4 len);

#ifdef __cplusplus
}
#endif

#endif /* _MSG_QUEUE_H_ */
