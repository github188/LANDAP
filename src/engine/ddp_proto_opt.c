/*
 * ddp_proto_opt.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ddp.h"
#include "ddp_platform.h"


/* ddp_proto_reset (AA)
 *   function to process reset packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input reset request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_reset
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    struct ddp_header outHdr;
    struct ddp_message pkt;
    UINT1* inBody = NULL;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    INT4 pos = 0;
    INT4 len = 0;
    struct ddp_exe_status outErr;
    INT4 ret = DDP_ALL_FIELD_SUCCESS;
    UINT4 delay = 0;
    struct ddp_message *delay_msg = NULL;
    //struct ddp_user newUser;

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
    memset(&outErr, 0, sizeof(outErr));
    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V2;
    outHdr.bodyLen = DDP_REPLY_LEN_RESET;
    if (outHdr.ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("fail to allocate reset reply msg\n");
        return -4;
    }
    memset(outMsg, 0, outMsgLen);
    if (outHdr.ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    /* verify output message */
    if (inHdr->bodyLen != DDP_REQ_LEN_RESET) {
        outHdr.retCode = DDP_RETCODE_FAIL;
        ret = -2;
    }
    pos = 0;
    /* authentication */
    if (ddp_proto_check_op_authen(DDP_OP_RESET) == DDP_OP_AUTHEN_ON) {
        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                ret = -3;
            }
        } else {
            ret = -4;
        }
        if (ret != DDP_ALL_FIELD_SUCCESS) {
            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
            goto process_reset_over;
        }
        pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
    }
    /* get delay time */
    if (ret == DDP_ALL_FIELD_SUCCESS) {
        memcpy((void*)&delay, (void*)(inBody + pos), DDP_FIELD_LEN_DELAY_TIME);
        delay = DDP_NTOHL(delay);
    }

process_reset_over:
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&outHdr);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_EXEST) {
        print_exe_status(&outErr);
    }

    memset(&pkt, 0, sizeof(pkt));
    pkt.ifindex = ifs->ifindex;
    if (outHdr.ipVer == IPV4_FLAG) {
        inAddr = (struct sockaddr_in*)&inMsg->sender;
        outAddr = (struct sockaddr_in*)&pkt.sender;
        outAddr->sin_family = inAddr->sin_family;
        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(outMsg, outMsgLen);
    }

    if (ret == DDP_ALL_FIELD_SUCCESS) {
        /* delayed reset */
        if (delay > 0) {
            DDP_DEBUG("Reset is delayed for %d seconds\n", delay);
            delay_msg = create_delay_msg(ifs, inHdr, inMsg);
            if (delay_msg == NULL) {
                outHdr.retCode = DDP_RETCODE_FAIL;
                ret = -4;
            } else {
                ret = ddp_alrm_queue_add(DDP_OP_RESET, delay, delay_msg);
                if (ret != 0) {
                    outHdr.retCode = DDP_RETCODE_FAIL;
                    ret = -5;
                }
            }
            pack_header(outMsg, &outHdr);
            pack_exe_status(outBody, &outErr);
            sendout_msg(&pkt, outMsg, outMsgLen);
        }
        /* reset immediately */
        else {
            /* reply if original message */
            if (inHdr->retCode == 0x7777) {
                pack_header(outMsg, &outHdr);
                pack_exe_status(outBody, &outErr);
                sendout_msg(&pkt, outMsg, outMsgLen);
            }
            len = 1;
            /* start to reset */
            ret = ddp_platform_set_field(ifs, DDP_FIELD_RESET, outBody, len);
            /* process should not come back if reboot is successful */
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                DDP_DEBUG("%s (%d) : set reset fail (error %d)\n", __FILE__, __LINE__, ret);
                outHdr.retCode = DDP_RETCODE_FAIL;
                pack_header(outMsg, &outHdr);
                pack_exe_status(outBody, &outErr);
                sendout_msg(&pkt, outMsg, outMsgLen);
            }
        }
    } else {
        pack_header(outMsg, &outHdr);
        pack_exe_status(outBody, &outErr);
        sendout_msg(&pkt, outMsg, outMsgLen);
    }

    if (outMsg) { free(outMsg); outMsg = NULL; }
    return ret;
}

/* ddp_proto_factory_reset (AB)
 *   function to process factory reset packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input reset request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_factory_reset
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    struct ddp_header outHdr;
    struct ddp_message pkt;
    UINT1* inBody = NULL;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    INT4 pos = 0;
    INT4 len = 0;
    struct ddp_exe_status outErr;
    INT4 ret = DDP_ALL_FIELD_SUCCESS;
    UINT4 delay = 0;
    struct ddp_message *delay_msg = NULL;

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
    memset(&outErr, 0, sizeof(outErr));
    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V2;
    outHdr.bodyLen = DDP_REPLY_LEN_FACTORY_RESET;
    if (outHdr.ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("fail to allocate factory reset reply msg\n");
        return -4;
    }
    memset(outMsg, 0, outMsgLen);
    if (outHdr.ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    /* verify output message */
    if (inHdr->bodyLen != DDP_REQ_LEN_FACTORY_RESET) {
        outHdr.retCode = DDP_RETCODE_FAIL;
        ret = -2;
    }
    pos = 0;
    /* authentication */
    if (ddp_proto_check_op_authen(DDP_OP_FACTORY_RESET) == DDP_OP_AUTHEN_ON) {
        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                ret = -3;
            }
        } else {
            ret = -4;
        }
        if (ret != DDP_ALL_FIELD_SUCCESS) {
            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
            goto process_factory_reset_over;
        }
        pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
    }
    /* get delay time */
    if (ret == DDP_ALL_FIELD_SUCCESS) {
        memcpy((void*)&delay, (void*)(inBody + pos), DDP_FIELD_LEN_DELAY_TIME);
        delay = DDP_NTOHL(delay);
    }


process_factory_reset_over:
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&outHdr);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_EXEST) {
        print_exe_status(&outErr);
    }

    memset(&pkt, 0, sizeof(pkt));
    pkt.ifindex = ifs->ifindex;
    if (outHdr.ipVer == IPV4_FLAG) {
        inAddr = (struct sockaddr_in*)&inMsg->sender;
        outAddr = (struct sockaddr_in*)&pkt.sender;
        outAddr->sin_family = inAddr->sin_family;
        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(outMsg, outMsgLen);
    }

    if (ret == DDP_ALL_FIELD_SUCCESS) {
        /* delayed factory reset */
        if (delay > 0) {
            DDP_DEBUG("Factory reset is delayed for %d seconds\n", delay);
            delay_msg = create_delay_msg(ifs, inHdr, inMsg);
            if (delay_msg == NULL) {
                outHdr.retCode = DDP_RETCODE_FAIL;
                ret = -4;
            } else {
                ret = ddp_alrm_queue_add(DDP_OP_FACTORY_RESET, delay, delay_msg);
                if (ret != 0) {
                    outHdr.retCode = DDP_RETCODE_FAIL;
                    ret = -5;
                }
            }
            pack_header(outMsg, &outHdr);
            pack_exe_status(outBody, &outErr);
            sendout_msg(&pkt, outMsg, outMsgLen);
        }
        /* factory reset immediately */
        else {
            /* reply if original message */
            if (inHdr->retCode == 0x7777) {
                pack_header(outMsg, &outHdr);
                pack_exe_status(outBody, &outErr);
                sendout_msg(&pkt, outMsg, outMsgLen);
            }
            len = 1;
            /* start to factory reset */
            ret = ddp_platform_set_field(ifs, DDP_FIELD_FACTORY_RESET, outBody, len);
            /* process should not come back if factory reset is successful. */
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                DDP_DEBUG("%s (%d) : set factory reset fail (error %d)\n", __FILE__, __LINE__, ret);
                outHdr.retCode = DDP_RETCODE_FAIL;
                pack_header(outMsg, &outHdr);
                pack_exe_status(outBody, &outErr);
                sendout_msg(&pkt, outMsg, outMsgLen);
            }
        }
    } else {
        pack_header(outMsg, &outHdr);
        pack_exe_status(outBody, &outErr);
        sendout_msg(&pkt, outMsg, outMsgLen);
    }

    if (outMsg) { free(outMsg); outMsg = NULL; }
    return ret;
}

/* ddp_proto_query_neighbor_info (D0)
 *   function to process query neighbor info packet and send through the incoming interfaces
 *   this op is enabled only in switch products.
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input query neighbor info request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_query_neighbor_info
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = 0;
    INT4 len = 0;
    struct ddp_header outHdr;
    struct ddp_message pkt;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }

    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(&outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V2;
    if (inHdr->ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
        if (ddp_platform_capable_process_report(ifs) == DDP_PROCESS_REPORT_YES) {
            outHdr.bodyLen = DDP_REPLY_LEN_QUERY_NEIGHBOR_INFO;
        } else {
            outHdr.bodyLen = 1;
        }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
        if (ddp_platform_capable_process_report(ifs) == DDP_PROCESS_REPORT_YES) {
            outHdr.bodyLen = DDP_REPLY_LEN_QUERY_NEIGHBOR_INFO;
        } else {
            outHdr.bodyLen = 1;
        }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);

    if (outMsg == NULL) {
        DDP_DEBUG("\n%s (%d) : fail to allocate query neighbor info reply msg\n", __FILE__, __LINE__);
        return -2;
    }

    memset(outMsg, 0, outMsgLen);

    if (inHdr->ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }

    /* query field value and fill in output message */
    if (ddp_platform_capable_process_report(ifs) == DDP_PROCESS_REPORT_YES) {

        /* fill body with the other devices' information collected from report messages */
        switch (i4fnDDP_proto_switch_Fill_QueryNeighb_Pkt(
                ifs, (INT1*)outBody, &(outHdr.bodyLen))){
            case DDP_RET_SWITCH_OK:
                break;
            case DDP_RET_SWITCH_ERROR_EMPTY_DB:
                DDP_DEBUG("\n%s (%d) : Empty DB.\n", __FILE__, __LINE__);
                break;
            default:
               DDP_DEBUG("\n%s (%d) : Failed to filled the packet.\n", __FILE__, __LINE__);
               break;
        }
        /* Update Message length */
        if (inHdr->ipVer == IPV4_FLAG) {
            outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
        } else {
            outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
        }
    }
    else {
        *outBody = 0;
    }
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&outHdr);
    }
    pack_header(outMsg, &outHdr);

    memset(&pkt, 0, sizeof(pkt));
    pkt.ifindex = ifs->ifindex;
    if (outHdr.ipVer == IPV4_FLAG) {
        inAddr = (struct sockaddr_in*)&inMsg->sender;
        outAddr = (struct sockaddr_in*)&pkt.sender;
        outAddr->sin_family = inAddr->sin_family;
        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(outMsg, outMsgLen);
    }
    sendout_msg(&pkt, outMsg, outMsgLen);

    if (outMsg) { free(outMsg); outMsg = NULL; }
    return ret;
}

/* ddp_proto_get_sys_date_time (AC)
 *   function to process get sys date time packet and send back date time information.
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input get sys date time request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_get_sys_date_time
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    struct ddp_header outHdr;
    struct ddp_message pkt;
    UINT1* inBody = NULL;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    INT4 pos = 0;
    INT4 len = 0;
    INT4 ret = DDP_ALL_FIELD_SUCCESS;
    struct dst_setting dst;
    UINT2 tmpU2 = 0;
    struct ntp_addr ntpAddr;

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V2;
    outHdr.bodyLen = DDP_REPLY_LEN_GET_SYS_DATE_TIME;
    if (outHdr.ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("fail to allocate reset reply msg\n");
        return -4;
    }
    memset(outMsg, 0, outMsgLen);
    if (outHdr.ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    /* verify output message */
    if (inHdr->bodyLen != DDP_REQ_LEN_GET_SYS_DATE_TIME) {
        outHdr.retCode = DDP_RETCODE_FAIL;
        ret = -2;
    }
    /* authentication */
    if (ddp_proto_check_op_authen(DDP_OP_GET_SYS_DATE_TIME) == DDP_OP_AUTHEN_ON) {
        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                ret = -3;
            }
        } else {
            ret = -4;
        }
        if (ret != DDP_ALL_FIELD_SUCCESS) {
            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
            goto process_get_sys_date_time_over;
        }
    }

    if (ret == DDP_ALL_FIELD_SUCCESS) {
        memset((void*)&ntpAddr, 0, sizeof(struct ntp_addr));
        pos = 0; len = DDP_FIELD_LEN_DATE_TIME;
        ddp_platform_get_field(ifs, DDP_FIELD_DATE_TIME, outBody + pos, &len);
        pos += DDP_FIELD_LEN_DATE_TIME; len = DDP_FIELD_LEN_TIMEZONE;
        ddp_platform_get_field(ifs, DDP_FIELD_TIMEZONE, outBody + pos, &len);
        pos += DDP_FIELD_LEN_TIMEZONE; len = DDP_FIELD_LEN_DAYLIGHT_SAVING;
        ddp_platform_get_field(ifs, DDP_FIELD_DAYLIGHT_SAVING, outBody + pos, &len);
        pos += DDP_FIELD_LEN_DAYLIGHT_SAVING; len = DDP_FIELD_LEN_NTP_ENABLE;
        ddp_platform_get_field(ifs, DDP_FIELD_NTP_ENABLE, outBody + pos, &len);
        pos += DDP_FIELD_LEN_NTP_ENABLE; //len = DDP_FIELD_LEN_NTP_SERVER;
        //ddp_platform_get_field(ifs, DDP_FIELD_NTP_SERVER, outBody + pos, &len);
        len = DDP_FIELD_LEN_NTP_SERVER + DDP_FIELD_LEN_NTP_FORMAT;
        ddp_platform_get_field(ifs, DDP_FIELD_NTP_SERVER, (UINT1*)&ntpAddr, &len);
        memcpy((void*)(outBody + pos), (void*)&ntpAddr.buf, DDP_FIELD_LEN_NTP_SERVER);
        /* DST
         * get dst value from platform by dst_setting struct
         * fill value to message
         */
        len = sizeof(dst);
        ddp_platform_get_field(ifs, DDP_FIELD_DST_SETTING, (UINT1*)&dst, &len);

        pos += DDP_FIELD_LEN_NTP_SERVER; len = DDP_FIELD_LEN_DST_YEAR;
        tmpU2 = dst.start_year; tmpU2 = DDP_HTONS(tmpU2);
        memcpy(outBody + pos, &tmpU2, len);
        pos += DDP_FIELD_LEN_DST_YEAR; len = DDP_FIELD_LEN_DST_MONTH;
        *(outBody + pos) = dst.start_month;
        pos += DDP_FIELD_LEN_DST_MONTH; len = DDP_FIELD_LEN_DST_DAY;
        *(outBody + pos) = dst.start_day;
        pos += DDP_FIELD_LEN_DST_DAY; len = DDP_FIELD_LEN_DST_HOUR;
        *(outBody + pos) = dst.start_hour;
        pos += DDP_FIELD_LEN_DST_HOUR; len = DDP_FIELD_LEN_DST_MINUTE;
        *(outBody + pos) = dst.start_minute;

        pos += DDP_FIELD_LEN_DST_MINUTE; len = DDP_FIELD_LEN_DST_YEAR;
        tmpU2 = dst.end_year; tmpU2 = DDP_HTONS(tmpU2);
        memcpy(outBody + pos, &tmpU2, len);
        pos += DDP_FIELD_LEN_DST_YEAR; len = DDP_FIELD_LEN_DST_MONTH;
        *(outBody + pos) = dst.end_month;
        pos += DDP_FIELD_LEN_DST_MONTH; len = DDP_FIELD_LEN_DST_DAY;
        *(outBody + pos) = dst.end_day;
        pos += DDP_FIELD_LEN_DST_DAY; len = DDP_FIELD_LEN_DST_HOUR;
        *(outBody + pos) = dst.end_hour;
        pos += DDP_FIELD_LEN_DST_HOUR; len = DDP_FIELD_LEN_DST_MINUTE;
        *(outBody + pos) = dst.end_minute;

        pos += DDP_FIELD_LEN_DST_MINUTE; len = DDP_FIELD_LEN_DST_OFFSET;
        *(outBody + pos) = dst.offset;
        pos += DDP_FIELD_LEN_DST_OFFSET; len = DDP_FIELD_LEN_NTP_FORMAT;
        *(outBody + pos) = ntpAddr.format;
        pos += DDP_FIELD_LEN_NTP_FORMAT; len = DDP_FIELD_LEN_NTP_SUPPORT;
        ddp_platform_get_field(ifs, DDP_FIELD_NTP_SUPPORT, outBody + pos, &len);
    }

process_get_sys_date_time_over:
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&outHdr);
    }
    pack_header(outMsg, &outHdr);

    memset(&pkt, 0, sizeof(pkt));
    pkt.ifindex = ifs->ifindex;
    if (outHdr.ipVer == IPV4_FLAG) {
        inAddr = (struct sockaddr_in*)&inMsg->sender;
        outAddr = (struct sockaddr_in*)&pkt.sender;
        outAddr->sin_family = inAddr->sin_family;
        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(outMsg, outMsgLen);
    }
    sendout_msg(&pkt, outMsg, outMsgLen);

    if (outMsg) { free(outMsg); outMsg = NULL; }
    return ret;
}

/* ddp_proto_set_sys_date_time (AD)
 *   function to process set sys date time packet and send back execute result.
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input set sys date time request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_set_sys_date_time
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    struct ddp_header outHdr;
    struct ddp_message pkt;
    UINT1* inBody = NULL;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    INT4 pos = 0;
    INT4 len = 0;
    UINT2 tmpU2 = 0;
    struct dst_setting dst;
    UINT1 ntp_enable = 0;
    struct ddp_exe_status outErr;
    struct ntp_addr ntpAddr;
    INT4 ret = DDP_ALL_FIELD_SUCCESS;

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
    memset(&outErr, 0, sizeof(outErr));
    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V2;
    outHdr.bodyLen = DDP_REPLY_LEN_SET_SYS_DATE_TIME;
    if (outHdr.ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("fail to allocate reset reply msg\n");
        return -4;
    }
    memset(outMsg, 0, outMsgLen);
    if (outHdr.ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    /* verify output message */
    if (inHdr->bodyLen != DDP_REQ_LEN_SET_SYS_DATE_TIME) {
        outHdr.retCode = DDP_RETCODE_FAIL;
        ret = -2;
    }
    /* authentication */
    if (ddp_proto_check_op_authen(DDP_OP_SET_SYS_DATE_TIME) == DDP_OP_AUTHEN_ON) {
        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                ret = -3;
            }
        } else {
            ret = -4;
        }
        if (ret != DDP_ALL_FIELD_SUCCESS) {
            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
            goto process_set_sys_date_time_over;
        }
    }

    if (ret == DDP_ALL_FIELD_SUCCESS) {
        /* if ntp enable is 1, set ntp first.
         * if ntp enable is 0, set date time by first 2 parameters.
         */
        memset((void*)&ntpAddr, 0, sizeof(struct ntp_addr));
        pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
        ntp_enable = *(inBody + pos + DDP_FIELD_LEN_DATE_TIME + DDP_FIELD_LEN_TIMEZONE + DDP_FIELD_LEN_DAYLIGHT_SAVING);
        if (ntp_enable == 1) {
            pos += DDP_FIELD_LEN_DATE_TIME + DDP_FIELD_LEN_TIMEZONE + DDP_FIELD_LEN_DAYLIGHT_SAVING;
            len = DDP_FIELD_LEN_NTP_ENABLE;
            ret = ddp_platform_set_field(ifs, DDP_FIELD_NTP_ENABLE, inBody + pos, len);
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                DDP_DEBUG("%s (%d) : set ntp enable fail (error %d)\n", __FILE__, __LINE__, ret);
                outErr.errNo = DDP_INVALID_FIELD_NTP_ENABLE; outHdr.retCode = DDP_RETCODE_FAIL;
            } else {
                ntpAddr.format = *(inBody + 97 + DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD);
                pos += DDP_FIELD_LEN_NTP_ENABLE;
                len = DDP_FIELD_LEN_NTP_SERVER;
                memcpy((void*)ntpAddr.buf, (void*)(inBody + pos), len);
                //ret = ddp_platform_set_field(ifs, DDP_FIELD_NTP_SERVER, inBody + pos, len);
                ret = ddp_platform_set_field(ifs, DDP_FIELD_NTP_SERVER, (UINT1*)&ntpAddr, len + 1);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set ntp server fail (error %d)\n", __FILE__, __LINE__, ret);
                    outErr.errNo = DDP_INVALID_FIELD_NTP_SERVER; outHdr.retCode = DDP_RETCODE_FAIL;
                }
            }
        }
        else {
            len = DDP_FIELD_LEN_DATE_TIME;
            ret = ddp_platform_set_field(ifs, DDP_FIELD_DATE_TIME, inBody + pos, len);
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                DDP_DEBUG("%s (%d) : set date time fail (error %d)\n", __FILE__, __LINE__, ret);
                outErr.errNo = DDP_INVALID_FIELD_DATETIME; outHdr.retCode = DDP_RETCODE_FAIL;
            }
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                pos += len; len = DDP_FIELD_LEN_TIMEZONE;
                ret = ddp_platform_set_field(ifs, DDP_FIELD_TIMEZONE, inBody + pos, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set time zone fail (error %d)\n", __FILE__, __LINE__, ret);
                    outErr.errNo = DDP_INVALID_FIELD_TIMEZONE; outHdr.retCode = DDP_RETCODE_FAIL;
                }
            }
        }
        /* DST
         * extract new setting value from message and fill in to dst_setting struct
         * apply dst_setting struct to platform
         */
        if (ret == DDP_ALL_FIELD_SUCCESS) {
            memset(&dst, 0, sizeof(dst));
            pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD + DDP_FIELD_LEN_DATE_TIME + DDP_FIELD_LEN_TIMEZONE;
            len = DDP_FIELD_LEN_DAYLIGHT_SAVING;
            dst.enable = *(inBody + pos);

            pos += DDP_FIELD_LEN_DAYLIGHT_SAVING + DDP_FIELD_LEN_NTP_ENABLE + DDP_FIELD_LEN_NTP_SERVER;
            len = DDP_FIELD_LEN_DST_YEAR; tmpU2 = 0;
            memcpy(&tmpU2, inBody + pos, len); tmpU2 = DDP_NTOHS(tmpU2);
            dst.start_year = tmpU2;
            pos += len; len = DDP_FIELD_LEN_DST_MONTH;
            dst.start_month = *(inBody + pos);
            pos += len; len = DDP_FIELD_LEN_DST_DAY;
            dst.start_day = *(inBody + pos);
            pos += len; len = DDP_FIELD_LEN_DST_HOUR;
            dst.start_hour = *(inBody + pos);
            pos += len; len = DDP_FIELD_LEN_DST_MINUTE;
            dst.start_minute = *(inBody + pos);

            pos += len; len = DDP_FIELD_LEN_DST_YEAR; tmpU2 = 0;
            memcpy(&tmpU2, inBody + pos, len); tmpU2 = DDP_NTOHS(tmpU2);
            dst.end_year = tmpU2;
            pos += len; len = DDP_FIELD_LEN_DST_MONTH;
            dst.end_month = *(inBody + pos);
            pos += len; len = DDP_FIELD_LEN_DST_DAY;
            dst.end_day = *(inBody + pos);
            pos += len; len = DDP_FIELD_LEN_DST_HOUR;
            dst.end_hour = *(inBody + pos);
            pos += len; len = DDP_FIELD_LEN_DST_MINUTE;
            dst.end_minute = *(inBody + pos);

            pos += len; len = DDP_FIELD_LEN_DST_OFFSET;
            dst.offset = *(inBody + pos);
            ret = ddp_platform_set_field(ifs, DDP_FIELD_DST_SETTING, (UINT1*)&dst, sizeof(dst));
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                DDP_DEBUG("%s (%d) : set daylight saving fail (error %d)\n", __FILE__, __LINE__, ret);
                outErr.errNo = DDP_INVALID_FIELD_DL_SAVING; outHdr.retCode = DDP_RETCODE_FAIL;
            }
        }
    }

process_set_sys_date_time_over:
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&outHdr);
    }
    pack_header(outMsg, &outHdr);
    pack_exe_status(outBody, &outErr);

    memset(&pkt, 0, sizeof(pkt));
    pkt.ifindex = ifs->ifindex;
    if (outHdr.ipVer == IPV4_FLAG) {
        inAddr = (struct sockaddr_in*)&inMsg->sender;
        outAddr = (struct sockaddr_in*)&pkt.sender;
        outAddr->sin_family = inAddr->sin_family;
        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(outMsg, outMsgLen);
    }
    sendout_msg(&pkt, outMsg, outMsgLen);

    if (outMsg) { free(outMsg); outMsg = NULL; }
    return ret;
}

/* ddp_proto_set_ipv4_addr (E1)
 *   function to process set ipv4 addr packet and send back execute result.
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input set ipv4 addr request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_set_ipv4_addr
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    struct ddp_header outHdr;
    struct ddp_message pkt;
    UINT1* inBody = NULL;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    INT4 pos = 0;
    INT4 len = 0;
    INT1 type = 0;
    INT1 value = 0;
    INT4 ret = DDP_ALL_FIELD_SUCCESS;

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V2;
    outHdr.bodyLen = DDP_REPLY_LEN_SET_IPV4_ADDR;
    if (outHdr.ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("fail to allocate reset reply msg\n");
        return -4;
    }
    memset(outMsg, 0, outMsgLen);
    if (outHdr.ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    /* verify output message */
    if (inBody == NULL) {
        outHdr.retCode = DDP_RETCODE_FAIL;
        ret = -2;
        *outBody = 1;
        sprintf((INT1*)(outBody + 1), "Set IPv4 address (op=%d) : input message without body", inHdr->opcode);
        goto process_set_ipv4_addr_over;
    }
    /* authentication */
    len = 0;
    pos = 0;
    if (ddp_proto_check_op_authen(DDP_OP_SET_IPV4_ADDR) == DDP_OP_AUTHEN_ON) {
        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                ret = -3;
            }
        } else {
            ret = -4;
        }
        if (ret != DDP_ALL_FIELD_SUCCESS) {
            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
            *outBody = 1;
            sprintf((INT1*)(outBody + 1), "Authentication fail");
            goto process_set_ipv4_addr_over;
        }
        pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
    }

    if (ret == DDP_ALL_FIELD_SUCCESS) {
        /* get type */
        type = *(inBody + pos);
        pos += 1;
        /* IPv4 address */
        if (type == 0) {
            /* DHCP */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_DHCP;
                value = *(inBody + pos);
                ret = ddp_platform_set_field(ifs, DDP_FIELD_DHCP, inBody + pos, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set dhcp fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set dhcp fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
            /* IPv4 */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
                if (value != 1) {
                    ret = ddp_platform_set_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, inBody + pos, len);
                }
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set ip fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set ip fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
            /* Subnet mask */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_SUBNET_MASK;
                if (value != 1) {
                    ret = ddp_platform_set_field(ifs, DDP_FIELD_SUBNET_MASK, inBody + pos, len);
                }
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set subnet mask fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set subnet mask fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
            /* Gateway */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_DEFAULT_GATEWAY;
                if (value != 1) {
                    ret = ddp_platform_set_field(ifs, DDP_FIELD_DEFAULT_GATEWAY, inBody + pos, len);
                }
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set default gateway fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set default gateway fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
        }
        else if (type == 1) {
            /* customized DNS */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_CUSTOMIZED_DNS;           
                ret = ddp_platform_set_field(ifs, DDP_FIELD_CUSTOMIZED_DNS, inBody + pos, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set customized dns fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set customized dns fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
            /* Primary DNS */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_PRIMARY_DNS;           
                ret = ddp_platform_set_field(ifs, DDP_FIELD_PRIMARY_DNS, inBody + pos, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set primary dns fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set primary dns fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
            /* Secondary DNS */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_SECONDARY_DNS;           
                ret = ddp_platform_set_field(ifs, DDP_FIELD_SECONDARY_DNS, inBody + pos, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set secondary dns fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set secdonary dns fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
        }
        else {
            ret = -5;
            DDP_DEBUG("%s (%d) : type (%d) is not supported\n", __FILE__, __LINE__, type);
            *outBody = 1;
            sprintf((INT1*)(outBody + 1), "Type (%d) is not supported", type);
        }
    }

process_set_ipv4_addr_over:
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&outHdr);
    }
    pack_header(outMsg, &outHdr);

    memset(&pkt, 0, sizeof(pkt));
    pkt.ifindex = ifs->ifindex;
    if (outHdr.ipVer == IPV4_FLAG) {
        inAddr = (struct sockaddr_in*)&inMsg->sender;
        outAddr = (struct sockaddr_in*)&pkt.sender;
        outAddr->sin_family = inAddr->sin_family;
        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(outMsg, outMsgLen);
    }
    sendout_msg(&pkt, outMsg, outMsgLen);

    if (outMsg) { free(outMsg); outMsg = NULL; }
    return ret;
}

/* ddp_proto_set_ipv6_addr (E2)
 *   function to process set ipv6 addr packet and send back execute result.
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input set ipv6 addr request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_set_ipv6_addr
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    struct ddp_header outHdr;
    struct ddp_message pkt;
    UINT1* inBody = NULL;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    INT4 pos = 0;
    INT4 len = 0;
    INT1 type = 0;
    INT1 value = 0;
    INT4 ret = DDP_ALL_FIELD_SUCCESS;

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V2;
    outHdr.bodyLen = DDP_REPLY_LEN_SET_IPV6_ADDR;
    if (outHdr.ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("fail to allocate reset reply msg\n");
        return -4;
    }
    memset(outMsg, 0, outMsgLen);
    if (outHdr.ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    /* verify output message */
    if (inBody == NULL) {
        outHdr.retCode = DDP_RETCODE_FAIL;
        *outBody = 1;
        sprintf((INT1*)(outBody + 1), "Set IPv6 address (op=%d) : input message without body", inHdr->opcode);
        ret = -2;
        goto process_set_ipv6_addr_over;
    }
    /* authentication */
    len = 0;
    pos = 0;
    if (ddp_proto_check_op_authen(DDP_OP_SET_IPV6_ADDR) == DDP_OP_AUTHEN_ON) {
        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                ret = -3;
            }
        } else {
            ret = -4;
        }
        if (ret != DDP_ALL_FIELD_SUCCESS) {
            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
            *outBody = 1;
            sprintf((INT1*)(outBody + 1), "Authentication fail");
            goto process_set_ipv6_addr_over;
        }
        pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
    }

    if (ret == DDP_ALL_FIELD_SUCCESS) {
        /* get type */
        type = *(inBody + pos);
        pos += 1;
        /* IPv4 address */
        if (type == 0) {
            /* DHCPv6 */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_DHCP_IPV6;
                value = *(inBody + pos);
                ret = ddp_platform_set_field(ifs, DDP_FIELD_DHCP_IPV6, inBody + pos, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set dhcpv6 fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set dhcpv6 fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
            /* IPv6 */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
                if (value != 1) {
                    ret = ddp_platform_set_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, inBody + pos, len);
                }
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set ipv6 fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set ipv6 fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
            /* IPv6 prefix */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_DEVICE_IPV6_PREFIX;
                ret = ddp_platform_set_field(ifs, DDP_FIELD_DEVICE_IPV6_PREFIX, inBody + pos, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set ipv6 prefix fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set ipv6 prefix fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
            /* IPv6 gateway */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_DEFAULT_GATEWAY_IPV6;
                if (value != 1) {
                    ret = ddp_platform_set_field(ifs, DDP_FIELD_DEFAULT_GATEWAY_IPV6, inBody + pos, len);
                }
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set gatewayv6 fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set gatewayv6 fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
        }
        else if (type == 1) {
            /* Primary IPv6 DNS */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_PRIMARY_DNS_IPV6;           
                ret = ddp_platform_set_field(ifs, DDP_FIELD_PRIMARY_DNS_IPV6, inBody + pos, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set primary dns v6 fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set primary dns v6 fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
            /* Secondary IPv6 DNS */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_SECONDARY_DNS_IPV6;           
                ret = ddp_platform_set_field(ifs, DDP_FIELD_SECONDARY_DNS_IPV6, inBody + pos, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set secondary dns v6 fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set secondary dns v6 fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
        } else {
            ret = -5;
            DDP_DEBUG("%s (%d) : type (%d) is not supported\n", __FILE__, __LINE__, type);
            *outBody = 1;
            sprintf((INT1*)(outBody + 1), "Type (%d) is not supported", type);
        }
    }

process_set_ipv6_addr_over:
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&outHdr);
    }
    pack_header(outMsg, &outHdr);

    memset(&pkt, 0, sizeof(pkt));
    pkt.ifindex = ifs->ifindex;
    if (outHdr.ipVer == IPV4_FLAG) {
        inAddr = (struct sockaddr_in*)&inMsg->sender;
        outAddr = (struct sockaddr_in*)&pkt.sender;
        outAddr->sin_family = inAddr->sin_family;
        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(outMsg, outMsgLen);
    }
    sendout_msg(&pkt, outMsg, outMsgLen);

    if (outMsg) { free(outMsg); outMsg = NULL; }
    return ret;
}

/* ddp_proto_set_device_info (E3)
 *   function to process set device info packet and send back execute result.
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input set device info request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_set_device_info
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    struct ddp_header outHdr;
    struct ddp_message pkt;
    UINT1* inBody = NULL;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    INT4 pos = 0;
    INT4 len = 0;
    INT1 type = 0;
    UINT2 value = 0;
    INT4 ret = DDP_ALL_FIELD_SUCCESS;

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V2;
    outHdr.bodyLen = DDP_REPLY_LEN_SET_DEVICE_INFO;
    if (outHdr.ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("fail to allocate reset reply msg\n");
        return -4;
    }
    memset(outMsg, 0, outMsgLen);
    if (outHdr.ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    /* verify output message */
    if (inBody == NULL) {
        outHdr.retCode = DDP_RETCODE_FAIL;
        ret = -2;
        *outBody = 1;
        sprintf((INT1*)(outBody + 1), "Input message without body");
        goto process_set_device_info_over;
    }
    /* authentication */
    len = 0;
    pos = 0;
    if (ddp_proto_check_op_authen(DDP_OP_SET_DEVICE_INFO) == DDP_OP_AUTHEN_ON) {
        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
            if (ret != DDP_ALL_FIELD_SUCCESS) {
                ret = -3;
            }
        } else {
            ret = -4;
        }
        if (ret != DDP_ALL_FIELD_SUCCESS) {
            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
            *outBody = 1;
            sprintf((INT1*)(outBody + 1), "Authentication fail");
            goto process_set_device_info_over;
        }
        pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
    }

    if (ret == DDP_ALL_FIELD_SUCCESS) {
        /* get type */
        type = *(inBody + pos);
        pos += 1;

        if (type == 0) {
            /* Device name */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_SYSTEM_NAME;
                ret = ddp_platform_set_field(ifs, DDP_FIELD_SYSTEM_NAME, inBody + pos, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set device name fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set device name fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
        }
        else if (type == 1) {
            /* Web port */
            if (ret == DDP_ALL_FIELD_SUCCESS) {
                len = DDP_FIELD_LEN_WEB_SERVICE_PORT;
                memcpy(&value, inBody + pos, len);
                value = DDP_NTOHS(value);
                ret = ddp_platform_set_field(ifs, DDP_FIELD_WEB_SERVICE_PORT, (UINT1*)&value, len);
                if (ret != DDP_ALL_FIELD_SUCCESS) {
                    DDP_DEBUG("%s (%d) : set web service port fail (error %d)\n", __FILE__, __LINE__, ret);
                    *outBody = 1;
                    sprintf((INT1*)(outBody + 1), "Set web service prot fail (error %d)", ret);
                    outHdr.retCode = DDP_RETCODE_FAIL;
                }
                pos += len;
            }
        }
        else {
            ret = -5;
            DDP_DEBUG("%s (%d) : type (%d) is not supported\n", __FILE__, __LINE__, type);
            *outBody = 1;
            sprintf((INT1*)(outBody + 1), "Type (%d) is not supported", type);
        }
    }

process_set_device_info_over:
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&outHdr);
    }
    pack_header(outMsg, &outHdr);

    memset(&pkt, 0, sizeof(pkt));
    pkt.ifindex = ifs->ifindex;
    if (outHdr.ipVer == IPV4_FLAG) {
        inAddr = (struct sockaddr_in*)&inMsg->sender;
        outAddr = (struct sockaddr_in*)&pkt.sender;
        outAddr->sin_family = inAddr->sin_family;
        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(outMsg, outMsgLen);
    }
    sendout_msg(&pkt, outMsg, outMsgLen);

    if (outMsg) { free(outMsg); outMsg = NULL; }
    return ret;
}

