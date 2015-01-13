
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ts_queue.h"

static INT1* strQueueName = "Queue";
INT1* node_str[] = { "node 1", "node 2", "node 3" };
const INT4 iBufSize = 21;

INT4
main
(
    INT4 argc,
    INT1** argv
)
{
    INT4 idx = 0;
    struct ts_node* node = NULL;
    UINT1* tmp = NULL;

    printf("Test of ts_queue : start\n");

    printf("  Create queue with name : %s\n", strQueueName);
    struct ts_queue* q = ts_queue_new(strQueueName);

    printf("  Insert nodes\n");
    for (idx = 0; idx < 3; idx++) {
        tmp = (UINT1*)malloc(21 * sizeof(UINT1));
        memset(tmp, 0, 21 * sizeof(UINT1));
        strcpy((INT1*)tmp, node_str[idx]);
        if (ts_queue_insert_node(q, tmp) != 0) {
            printf("  Add %s failed\n", tmp);
        }
    }

    printf("  Show data of all nodes :\n");
    ts_queue_display_all(q);

    printf("\n  Pop one node\n");
    node = ts_queue_remove_node(q);
    if (node->data) {
        printf("  Node data: %s\n", node->data);
        free(node->data);
        node->data = NULL;
    }
    if (node) {
        free(node);
        node = NULL;
    }

    printf("  Destroy queue\n");
    while (1) {
        node = ts_queue_remove_node(q);
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

    if (ts_queue_free(q)) {
        if (q->name) {
            printf("  Queue %s is not empty\n", q->name);
        }
    }

    printf("Test of ts_queue : end\n");
    return 0;
}
