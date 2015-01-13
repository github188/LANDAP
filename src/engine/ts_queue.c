
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ts_queue.h"


/* ts_queue_insert_node
 *   function to push a ts_node with data payload to queue
 *
 *   q : the queue
 *   data : data payload
 *
 *   return : 0 -> success, others -> error
 */
INT4
ts_queue_insert_node
(
    struct ts_queue* q,
    UINT1* data
)
{
    INT4 iReturn = 0;
    if (data == NULL) {
        iReturn = -1;
        return iReturn;
    }

    struct ts_node* node = (struct ts_node*)malloc(sizeof(struct ts_node));
    if (node == NULL) {
        if (q->name) {
            printf("%s (%d) : queue %s create node failed\n", __FILE__, __LINE__, q->name);
        } else {
            printf("%s (%d) : queue create node fail\n", __FILE__, __LINE__);
        }
        iReturn = -2;
    }
    else {
        pthread_mutex_lock(&q->lock);
        if (q->max_length != TS_QUEUE_LEN_UNLIMIT && q->cur_length >= q->max_length) {
            if (q->name) {
                printf("%s (%d) : reach max length %d of queue %s\n", __FILE__, __LINE__, q->max_length, q->name);
            } else {
                printf("%s (%d) : reach max length %d of queue\n", __FILE__, __LINE__, q->max_length);
            }
            iReturn = -3;
        }
        else {
            node->data = data;
            node->next = NULL;

            if (q->head == NULL && q->tail == NULL) {
                q->head = node;
            }
            else {
                q->tail->next = node;
            }
            q->tail = node;
            q->cur_length++;
        }
        pthread_mutex_unlock(&q->lock);
    }
    return iReturn;
}

/* ts_queue_remove_node
 *   function to pop a ts_node from queue
 *
 *   q : the queue
 *
 *   return : a pointer to the pop ts_node or NULL (nothing returned)
 */
struct ts_node*
ts_queue_remove_node
(
    struct ts_queue* q
)
{
    struct ts_node* node = NULL;

    pthread_mutex_lock(&q->lock);
    if (q->head == NULL && q->tail == NULL) {
        if (q->name) {
            //printf("%s (%d) : queue %s is empty\n", __FILE__, __LINE__, q->name);
        }
    }
    else {
        if (q->head == q->tail) {
            q->tail = NULL;
        }
        node = q->head;
        q->head = q->head->next;
        if (q->cur_length > 0) {
            q->cur_length--;
        }
    }
    pthread_mutex_unlock(&q->lock);

    return node;
}

/* ts_queue_new
 *   function to create a ts_queue with the input name
 *
 *   name : a string
 *
 *   return : pointer to newly-createde ts_queue of NULL (fail)
 */
struct ts_queue*
ts_queue_new
(
    INT1* name
)
{
    INT4 iStrLen = 0;
    struct ts_queue* q = (struct ts_queue*)malloc(sizeof(struct ts_queue));
    if (q == NULL) {
        if (name) {
            printf("%s (%d) : ts queue %s init fail\n", __FILE__, __LINE__, name);
        } else {
            printf("%s (%d) ; ts queue fail\n", __FILE__, __LINE__);
        }
        return NULL;
    }

    q->head = q->tail = NULL;
    if (pthread_mutex_init(&q->lock, NULL) != 0) {
        printf("%s (%d) : init lock fail\n", __FILE__, __LINE__);
    }
    q->name = NULL;
    if (name) {
        iStrLen = strlen(name);
        if (iStrLen > 0) {
            q->name = (INT1*)malloc(iStrLen * sizeof(INT1) + 1);
            memset(q->name, 0, iStrLen + 1);
            strcpy(q->name, name);
        }
    }
    /* init length */
    q->max_length = TS_QUEUE_LEN_DEFAULT;
    q->cur_length = 0;

    return q;
}

/* ts_queue_free
 *   function to release ts_queue
 *
 *   q : the q to be freed
 *
 *   return : NULL (success to free) or pointer to q (fail)
 */
struct ts_queue*
ts_queue_free
(
    struct ts_queue* q
)
{
    if (q == NULL) {
        return NULL;
    }

    if (q->head == NULL && q->tail == NULL) {
        if (q->name) {
            free(q->name);
            q->name = NULL;
        }
        pthread_mutex_destroy(&q->lock);
        free(q);
        q = NULL;
    }
    return q;
}

/* ts_queue_set_length
 *   function to set length of ts_queue in unit of node
 *
 *   q : the q to be set max length
 *   len : new max length
 *
 *   return : 0 -> success, others -> error
 */
INT4
ts_queue_set_length
(
    struct ts_queue* q,
    INT4 len
)
{
    if (q == NULL) { return -1; }
    if (len == 0) { return -2; }

    q->max_length = len;
    return 0;
}

/* ts_queue_display_all
 *   function to display all nodes on queue
 *   it is used for debug
 *
 *   q : the q
 *
 *   return : none
 */
void
ts_queue_display_all
(
    struct ts_queue* q
)
{
    struct ts_node* node = q->head;
    UINT1* data = NULL;

    if (q == NULL) {
        printf("%s (%d) : no queue\n", __FILE__, __LINE__);
        return;
    }

    while (node) {
        data = node->data;
        if (data) {
            printf("%s (%d) : node data : %s\n", __FILE__, __LINE__, data);
        }
        node = node->next;
    }
}
