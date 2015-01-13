
#include <stdlib.h>
#include <pthread.h>

#include "ts_queue.h"
#include "msg_queue.h"


/* Message queue between recv thread and proc thread.
 * 3 parties (recv thread, proc thread, and report timer) would access this queue.
 */

/* msg_queue_init
 *   function to create a ts_queue which is used as msg_queue
 *
 *   name : queue name
 *
 *   return : pointer to msg_queue (suucess) or NULL (fail)
 */
struct msg_queue*
msg_queue_init
(
    INT1* name
)
{
    struct msg_queue* mq = NULL;

	mq = (struct msg_queue*)malloc(sizeof(struct msg_queue));
    if (mq == NULL) {
        if (name) {
            printf("%s (%d) : allocate msg queue %s fail\n", __FILE__, __LINE__, name);
        } else {
            printf("%s (%d) : allocate msg queue fail\n", __FILE__, __LINE__);
        }
        return mq;
    }

    mq->q = ts_queue_new(name);
    if (mq->q == NULL) {
        if (name) {
            printf("%s (%d) : msg queue %s init fail\n", __FILE__, __LINE__, name);
        } else {
            printf("%s (%d) : msg queue init fail\n", __FILE__, __LINE__);
        }
        free(mq); mq = NULL;
        return NULL;
    }
    ts_queue_set_length(mq->q, MSG_QUEUE_LEN_DEFAULT);
    return mq;
}

/* msg_queue_free
 *   fucntion to release msg_queue
 *
 *   mq : the queue
 *
 *   return : 0 -> success, others -> error
 */
INT4
msg_queue_free
(
    struct msg_queue* mq
)
{
    struct ts_node* node = NULL;

    if (mq == NULL) {
        printf("%s (%d) : free null msg queue\n", __FILE__, __LINE__);
        return 0;
    }

    while (1) {
        node = ts_queue_remove_node(mq->q);
        if (node == NULL) {
            break;
        }
        if (node->data) {
            free(node->data);
            node->data = NULL;
        }
        free(node);
        node = NULL;
    }
    ts_queue_free(mq->q);
    mq->q = NULL;
    free(mq);
    mq = NULL;
    return 0;
}

/* msg_queue_insert_msg
 *   function to push data into the msg queue
 *
 *   mq : the queue to be inserted
 *   msg : new msg payload
 *
 *   return : 0 -> success, others ->error
 */
INT4
msg_queue_insert_msg
(
    struct msg_queue* mq,
    UINT1* msg
)
{
    INT4 ret = 0;
    if (msg == NULL) {
        printf("%s (%d) : msg queue: pkt is null\n", __FILE__, __LINE__);
        return -1;
    }

    if (mq == NULL) {
        printf("%s (%d) : msg queue does not exist. (insert fail)\n", __FILE__, __LINE__);
        return -2;
    }
    /* insert */
    ret = ts_queue_insert_node(mq->q, msg);
    if (ret < 0) {
        if (mq->q->name) {
            printf("%s (%d) : msg queue %s insert pkt fail (error %d)\n", __FILE__, __LINE__, mq->q->name, ret);
        } else {
            printf("%s (%d) : msg queue insert pkt fail (error %d)\n", __FILE__, __LINE__, ret);
        }
        return -3;
    }
    return 0;
}

/* msg_queue_remove_msg
 *   function to popup a msg from msg queue
 *
 *   mq : the queue
 *
 *   return : the msg payload carried in the popup node or NULL (fail)
 */
UINT1*
msg_queue_remove_msg
(
    struct msg_queue* mq
)
{
    struct ts_node* node = NULL;
    UINT1* tmp = NULL;

    if (mq == NULL) {
        printf("%s (%d) : msg queue does not exist (remove fail)\n", __FILE__, __LINE__);
        return NULL;
    }
    /* remove */
    node = ts_queue_remove_node(mq->q);
    if (node != NULL) {
        tmp = node->data;
        node->data = NULL;
        free(node);
    }
    return tmp;
}

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
INT4
msg_queue_set_length
(
    struct msg_queue* mq,
    INT4 len
)
{
    INT4 ret = 0;
    if (mq == NULL) { return -1; }
    if (len == 0) { return -2; }

    ret = ts_queue_set_length(mq->q, len);
    return ret;
}
