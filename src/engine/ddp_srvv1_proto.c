
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ddp.h"
#include "ddp_platform.h"

INT4 ddp_srvv1_proto_discovery(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
INT4 ddp_srvv1_proto_set_basic_info(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
INT4 ddp_srvv1_proto_user_verify(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
INT4 ddp_srvv1_proto_change_id_psw(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);

/* ddp_srvv1_proto_process_message
 *   dispatch function to process message
 *
 *   inMsg : input raw message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_srvv1_proto_process_message
(
    struct ddp_message* inMsg
)
{
    INT4 retVal = 0;
    struct ddp_interface* ifs = NULL;
    struct ddp_header inHdr;

    /* extract header */
    extract_header(inMsg->payload, inMsg->size, &inHdr);

    /* print out header after parsing */
    if (g_debugFlag & DDP_DEBUG_PRINT_IN_MSG_HDR) {
        printf("Header of input packet\n");
        print_unpack_header(&inHdr);
    }

    if (inMsg->ifindex == 0xFFFFFFFF) {
        /* send through all interfaces */
    } else {
        ifs = ddp_platform_search_if_by_index(g_ifList, inMsg->ifindex);
        if (ifs == NULL) {
            DDP_DEBUG("No interface is matched (if index %d) --> Drop it\n", inMsg->ifindex);
            retVal = -1;
            goto process_msg_over;
        }
    }

    /*if (ddp_proto_filter(&inHdr, ifs) != 1) {
        DDP_DEBUG("Req is not for this host/interface --> Drop it\n")
        goto process_msg_over;
    }*/
    /* op code */    
    switch (inHdr.opcode) {
        case DDP_OP_DISCOVERY:
            retVal = ddp_srvv1_proto_discovery(ifs, &inHdr, inMsg);
            break;
        case DDP_OP_SET_BASIC_INFO:
        	retVal = ddp_srvv1_proto_set_basic_info(ifs, &inHdr, inMsg);
            break;
        case DDP_OP_USER_VERIFY:
            retVal = ddp_srvv1_proto_user_verify(ifs, &inHdr, inMsg);
            break;
        case DDP_OP_CHANGE_ID_PSW:
            retVal = ddp_srvv1_proto_change_id_psw(ifs, &inHdr, inMsg);
            break;
//        case DDP_OP_QUERY_SUPPORT_OPT:
//            if (ddp_proto_check_op_enable(DDP_OP_QUERY_SUPPORT_OPT) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_query_support_opt(ifs, &inHdr, inMsg);
//            }
//            break;
        default:
            DDP_DEBUG("%s (%d) : opcode (%X) is not supported\n", __FILE__, __LINE__, inHdr.opcode);
            break;
    }

process_msg_over:
    DDP_DEBUG("----- REPLY over -----\n");
    return retVal;
}
