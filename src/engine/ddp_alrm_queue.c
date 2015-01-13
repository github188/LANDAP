
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>


#include "ddp.h"
#include "msg_queue.h"

/* ddp_alrm_queue
 *   Reboot, reset, and factory reset require alarm function for delay time.
 *   Since a signal id is available for only one alarm, this alarm queue emulates alarm function.
 *
 * Add an alarm for a opcode
 *   * Add a define to repsent alarm id in the format ALRM_ELE_<opcode>
 *   * Increase ALRM_ARR_SIZE
 */
#define ALRM_ARR_SIZE 3
#define ALRM_ELE_REBOOT        0
#define ALRM_ELE_RESET         1
#define ALRM_ELE_FACTORY_RESET 2

enum {
    /* value of alrm_node.state
     */
    DDP_ALRM_NONE  = 0,
    DDP_ALRM_SET   = 1,

    /* return value of ddp_alrm_queue_countdown
     */
    DDP_ALRM_COUNT = 0,
    DDP_ALRM_UP    = 1,
};
/* alrm_node
 *   interval: the remaining time of the alarm
 *   msg: the message to be transferred to msg_queue
 */
struct alrm_node {
    UINT2 state;
    UINT2 flag;
    UINT4 interval;
    struct ddp_message *msg;
};
/* time resolution of alarm in seconds */
INT4 g_alrmStep = 1;
/* lock of alrm queue */
pthread_mutex_t alrm_queue_mutex;
/* alrm_queue
 *   In current design, this queue is created for delay timer of reboot, reset, and factory reset opcodes.
 *   Element 0: reboot
 *   Element 1: reset
 *   Element 2: factory reset
 */
struct alrm_node alrm_queue[ALRM_ARR_SIZE];

/* ddp_alrm_to_index
 *   helper function to map opcode to array index
 *
 *   opcode : opcode
 *
 *   return : array index or -1 (error)
 */
INT4
ddp_alrm_to_index
(
    UINT2 opcode
)
{
    if (opcode == DDP_OP_REBOOT) { return ALRM_ELE_REBOOT; }
    else if (opcode == DDP_OP_RESET) { return ALRM_ELE_RESET; }
    else if (opcode == DDP_OP_FACTORY_RESET) { return ALRM_ELE_FACTORY_RESET; }
    else { return -1; }
}

/* ddp_alrm_to_opcode
 *   helper function to map array index to opcode
 *
 *   idx : array index
 *
 *   return : opcode or 0 (error)
 */
UINT2
ddp_alrm_to_opcode
(
    INT4 idx
)
{
    if (idx == ALRM_ELE_REBOOT) { return DDP_OP_REBOOT; }
    else if (idx == ALRM_ELE_RESET) { return DDP_OP_RESET; }
    else if (idx == ALRM_ELE_FACTORY_RESET) { return DDP_OP_FACTORY_RESET; }
    else { return 0; }
}

/* ddp_alrm_queue_add
 *   function to add an alarm object to alrm_queue
 *
 *   opcode : opcode
 *   interval : the time before alarm is up in seconds
 *   msg : alarm object
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_alrm_queue_add
(
    UINT2 opcode,
    UINT4 interval,
    struct ddp_message *msg
)
{
    INT4 idx = ddp_alrm_to_index(opcode);
    if (idx < 0) {
        return -1;
    }
    /* check remaining time */
    if (interval <= 0) {
        return -2;
    }
    /* add */
    pthread_mutex_lock(&alrm_queue_mutex);
    /* replace the previous alarm */
    if (alrm_queue[idx].msg) {
        free(alrm_queue[idx].msg); alrm_queue[idx].msg = NULL;
    }
    alrm_queue[idx].state = DDP_ALRM_SET;
    alrm_queue[idx].interval = interval;
    alrm_queue[idx].msg = msg;
    pthread_mutex_unlock(&alrm_queue_mutex);
    return 0;
}

/* ddp_alrm_queue_delete
 *   function to delete an alarm object from alrm_queue
 *
 *   opcode : opcode
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_alrm_queue_delete
(
    UINT2 opcode
)
{
    INT4 idx = ddp_alrm_to_index(opcode);
    if (idx < 0) {
        return -1;
    }
    pthread_mutex_lock(&alrm_queue_mutex);
    /* destroy alrm */
    alrm_queue[idx].state = DDP_ALRM_NONE;
    alrm_queue[idx].interval = 0;
    if (alrm_queue[idx].msg) {
        free(alrm_queue[idx].msg); alrm_queue[idx].msg = NULL;
    }
    pthread_mutex_unlock(&alrm_queue_mutex);
    return 0;
}

/* ddp_alrm_queue_up
 *   function to run when an alarm is up
 *
 *   idx : array index
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_alrm_queue_up
(
    INT4 idx
)
{
    struct ddp_message *obj = NULL;
    pthread_mutex_lock(&alrm_queue_mutex);
    /* insert msg into msg_queue */
    if (alrm_queue[idx].state == DDP_ALRM_SET && alrm_queue[idx].msg) {
        obj = alrm_queue[idx].msg;
        alrm_queue[idx].msg = NULL;
        alrm_queue[idx].state = DDP_ALRM_NONE;
    }
    pthread_mutex_unlock(&alrm_queue_mutex);
    if (obj) {
        DDP_DEBUG("alrm_queue : Alarm is up (%d)\n", idx);
        msg_queue_insert_msg(g_mq, (UINT1*)obj);
    }
    return 0;
}

/* ddp_alrm_queue_countdown
 *   function to drement interval and return the reamining interval in seconds
 *
 *   idx : array index
 *
 *   return : refer to the enum defined above
 */
INT4
ddp_alrm_queue_countdown
(
    INT4 idx
)
{
    INT4 status = DDP_ALRM_COUNT;
    pthread_mutex_lock(&alrm_queue_mutex);
    /* decrement interval by 1 */
    if (alrm_queue[idx].interval > 0) {
        alrm_queue[idx].interval--;
    }
    /* check whether the alarm is up or not */
    if (alrm_queue[idx].state == DDP_ALRM_SET && alrm_queue[idx].interval == 0) {
        status = DDP_ALRM_UP;
    }
    pthread_mutex_unlock(&alrm_queue_mutex);
    return status;
}

/* ddp_alrm_queue_thread
 *   major task of alarm queue thread
 *
 *   strThreadName : thread name
 *
 *   return : none
 */
void
ddp_alrm_queue_thread
(
    INT1* strThreadName
)
{
    INT4 iLoop = 0;
    INT4 idx = 0;
    INT4 status = 0;
    /* init */
    if (pthread_mutex_init(&alrm_queue_mutex, NULL) != 0) {
        printf("%s (%d) : init lock fail\n", __FILE__, __LINE__);
    }
    memset(alrm_queue, 0, sizeof(struct alrm_node) * ALRM_ARR_SIZE);

    while (1) {
        iLoop = ddp_get_loop_flag();
        if (iLoop & DDP_OVER_SIG) { break; }

        for (idx = 0; idx < ALRM_ARR_SIZE; idx++) {
            /* decrement interval by 1 */
            status = ddp_alrm_queue_countdown(idx);
            /* if the alarm is counting down */
            if (status == DDP_ALRM_UP) {
                /* if time is up */
                ddp_alrm_queue_up(idx);
            }
        }

        sleep(g_alrmStep);
    } /* while loop */
    /* cleanup */
    for (idx = 0; idx < ALRM_ARR_SIZE; idx++) {
        if (alrm_queue[idx].msg) {
            free(alrm_queue[idx].msg); alrm_queue[idx].msg = NULL;
        }
    }
}

