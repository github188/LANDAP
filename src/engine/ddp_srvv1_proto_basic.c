
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "ddp.h"
#include "ddp_platform.h"

/* ddp_proto_general_report (A0)
 *   function to generate general report packet and send through all valid interfaces
 *
 *
 *   Note: This function is used for two aspects:
 *
 *   1. Timer-triggered:
 *      For periodically send the general report.
 *
 *   2. Switch collect neighbors' information:
 *      When receiving "general report" packets, only switch will collect and
 *      maintain a local database for future request "Query Neighbor".
 *
 *   ifs : interface (unify api)
 *   inHdr : input header
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_general_report
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//    INT4 ret = 0;
//    UINT4 pos = 0;
//    INT4 len = 0;
//    INT4 valRes = 0;
//    struct ddp_header outHdr;
//    INT4 outMsgLen = 0;
//    UINT1* outMsg = NULL;
//    UINT1* outBody = NULL;
//    INT1 addr[16];
//    struct ddp_message pkt;
//    struct sockaddr_in* outAddr = NULL;
//    struct sockaddr_in6* outAddr6 = NULL;
//
//    if (inHdr == NULL /*|| inHdr->ipVer == IPV6_FLAG*/) { return -1; }
///*
//    *   1. Timer-triggered (IPV4_REQ):
//    *      For periodically send the general report.
//    *
//    *   2. Switch collect neighbors' information: (IPV4_REPORT or IPV6_REPORT)
//    *      When receiving "general report" packets, only switch will collect and
//    *      maintain a local database for future request "Query Neighbor".
//*/
//
//    /* process incoming report packet */
//    if (inHdr->identifier == IPV4_REPORT || inHdr->identifier == IPV6_REPORT) {
//        /* process report message from other devices */
//    	goto switch_process_general_report;
//    }
//    else{
//    	DDP_DEBUG("%s (%d) : Periodically Send.\n", __FILE__, __LINE__);
//    }
//
//    /* generate report packet */
//    memset(&outHdr, 0, sizeof(outHdr));
//    outHdr.ipVer = inHdr->ipVer;
//    outHdr.seq = inHdr->seq;
//    outHdr.opcode = inHdr->opcode;
//    outHdr.protoVer = DDP_PROTO_V1;
//    outHdr.bodyLen = DDP_REPLY_LEN_GENERAL_REPORT;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outHdr.identifier = IPV4_REPORT;
//        outMsgLen = HDR_END_V4_OFFSET + DDP_REPLY_LEN_GENERAL_REPORT;
//    } else {
//        outHdr.identifier = IPV6_REPORT;
//        outMsgLen = HDR_END_V6_OFFSET + DDP_REPLY_LEN_GENERAL_REPORT;
//    }
//    /* allocate output message */
//    outMsg = (UINT1*)malloc(outMsgLen);
//    if (outMsg == NULL) {
//        DDP_DEBUG("%s (%d) : fail to allocate general report pkt\n", __FILE__, __LINE__);
//        return -2;
//    }
//    memset(outMsg, 0, outMsgLen);
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outBody = outMsg + HDR_END_V4_OFFSET;
//    } else {
//        outBody = outMsg + HDR_END_V6_OFFSET;
//    }
//    /* query field value and fillin output message */
//    for (ifs = g_ifList->next; ifs != NULL; ifs = ifs->next) {
//        valRes = 0;
//        memset(outMsg, 0, outMsgLen);
//        pos = 0; len = DDP_FIELD_LEN_PRODUCT_NAME;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_PRODUCT_NAME, outBody + pos, &len);
//        pos += DDP_FIELD_LEN_PRODUCT_NAME; len = DDP_FIELD_LEN_PRODUCT_CATEGORY;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_PRODUCT_CATEGORY, outBody + pos, &len);
//        pos += DDP_FIELD_LEN_PRODUCT_CATEGORY; len = DDP_FIELD_LEN_VERSION;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_VERSION, outBody + pos, &len);
//        pos += DDP_FIELD_LEN_VERSION; len = DDP_FIELD_LEN_MAC_ADDR;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_MAC_ADDR, outBody + pos, &len);
//        memcpy(outHdr.macAddr, outBody + pos, DDP_FIELD_LEN_MAC_ADDR);
//        pos += DDP_FIELD_LEN_MAC_ADDR; len = DDP_FIELD_LEN_HARDWARE_VERSION;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_HARDWARE_VERSION, outBody + pos, &len);
//        pos += DDP_FIELD_LEN_HARDWARE_VERSION; len = DDP_FIELD_LEN_SERIAL_NUMBER;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_SERIAL_NUMBER, outBody + pos, &len);
//        pos += DDP_FIELD_LEN_SERIAL_NUMBER; len = DDP_FIELD_LEN_SYSTEM_NAME;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_SYSTEM_NAME, outBody + pos, &len);
//        pos += DDP_FIELD_LEN_SYSTEM_NAME; len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, outBody + pos, &len);
//        if (outHdr.ipVer == IPV4_FLAG) {
//            memcpy(outHdr.ipAddr.ipv4Addr, outBody + pos, IPV4_ADDRLEN);
//        }
//        pos += DDP_FIELD_LEN_DEVICE_IP_ADDR; len = DDP_FIELD_LEN_SUBNET_MASK;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_SUBNET_MASK, outBody + pos, &len);
//        pos += DDP_FIELD_LEN_SUBNET_MASK; len = DDP_FIELD_LEN_INTERFACE_NUMBER;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_INTERFACE_NUMBER, outBody + pos, &len);
//        pos += DDP_FIELD_LEN_INTERFACE_NUMBER; len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, outBody + pos, &len);
//        if (outHdr.ipVer == IPV6_FLAG) {
//            memcpy(outHdr.ipAddr.ipv6Addr, outBody + pos, IPV6_ADDRLEN);
//        }
//        pos += DDP_FIELD_LEN_DEVICE_IPV6_ADDR; len = DDP_FIELD_LEN_DEVICE_IPV6_PREFIX;
//        valRes = ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_PREFIX, outBody + pos, &len);
//        /* pack header into output message */
//        if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
//            printf("Header of output packet\n");
//            print_unpack_header(&outHdr);
//        }
//        pack_header(outMsg, &outHdr);
//
//        memset(&pkt, 0, sizeof(pkt));
//        pkt.ifindex = ifs->ifindex;
//        if (outHdr.ipVer == IPV4_FLAG) {
//            outAddr = (struct sockaddr_in*)&pkt.sender;
//            /* broadcast address and server port */
//            outAddr->sin_family = AF_INET;
//            memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//            outAddr->sin_port = htons(UDP_PORT_CLIENT);
//        } else {
//            outAddr6 = (struct sockaddr_in6*)&pkt.sender;
//            outAddr6->sin6_family = AF_INET6;
//            if (inet_pton(AF_INET6, DDP_IPV6_MCAST, addr) != 1) {
//                DDP_DEBUG("%s (%d) : set ipv6 multicast address fail (error %d)\n", __FILE__, __LINE__, errno);
//            }
//            memcpy(&outAddr6->sin6_addr, addr, IPV6_ADDRLEN);
//            //memcpy(&outAddr6->sin6_addr, &inAddr->sin6_addr, IPV6_ADDRLEN);
//            outAddr6->sin6_port = htons(UDP_PORT_CLIENT);
//        }
//        if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
//            printf("output packet in hex\n");
//            print_message_hex(outMsg, outMsgLen);
//        }
//        sendout_msg(&pkt, outMsg, outMsgLen);
//
//        if (ddp_platform_capable_process_report(ifs) == DDP_PROCESS_REPORT_YES){
//            /* Update database TTL, purge out-phased nodes. */
//            DDP_DEBUG("\n%s (%d) : Update DB TTL \n", __FILE__, __LINE__);
//            if(i4fnDDP_proto_switch_NeighbDB_Update_AllNode_TTL() != DDP_RET_SWITCH_OK){
//        	    DDP_DEBUG("%s (%d) : Failed to update TTL of database.\n", __FILE__, __LINE__);
//            }
//        }
//    } /* for loop */
//
//    if (outMsg) { free(outMsg); outMsg = NULL; }
//    goto general_report_over;
//
//    /* Only switch device will collect neighbors' general report information. */
//    /* IPV4_REQ is for periodic timer usage and  DDP_PROCESS_REPORT_YES is for switch */
//switch_process_general_report:
//    if (ddp_platform_capable_process_report(ifs) == DDP_PROCESS_REPORT_YES){
//
//        if( i4fnDDP_proto_switch_NeighbDB_Proc_Pkt(ifs, inHdr, inMsg) != DDP_RET_SWITCH_OK){
//            DDP_DEBUG("\n%s (%d) : Failed to insert node from pkt.\n", __FILE__, __LINE__);
//        }
//
//    }
//    else{
//    	DDP_DEBUG("\n%s (%d) : Discard report packet.\n", __FILE__, __LINE__);
//    }
//
//general_report_over:
//    return ret;
//}

/* ddp_proto_discovery (A1)
 *   function to process discovery packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input discovery request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_srvv1_proto_discovery
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = 0;

    if (	ifs == NULL
    		|| inHdr == NULL
			|| inMsg == NULL
    		/*|| inHdr->ipVer == IPV6_FLAG*/)
    	return -1;

    char buf[100];
    sprintf(buf, "%d.%d.%d.%d", inHdr->ipAddr.ipv4Addr[0], inHdr->ipAddr.ipv4Addr[1],
    							inHdr->ipAddr.ipv4Addr[2], inHdr->ipAddr.ipv4Addr[3]);
    DDP_INFO("%s\n", buf);

    return ret;
}

/* ddp_proto_set_basic_info (A2)
 *   function to process set basic info packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input set basic info request message
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_set_basic_info
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//    INT4 ret = DDP_ALL_FIELD_SUCCESS;
//    UINT2 tmpU2 = 0;
//    UINT1* inBody = NULL;
//    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
//    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
//    UINT1* outMsg = NULL;
//    struct ddp_header outHdr;
//    UINT1* outBody = NULL;
//    INT4 outMsgLen = 0;
//    INT4 pos = 0;
//    INT4 len = 0;
//    struct ddp_message pkt;
//    struct sockaddr_in* inAddr = NULL;
//    struct sockaddr_in* outAddr = NULL;
//    struct sockaddr_in6* inAddr6 = NULL;
//    struct sockaddr_in6* outAddr6 = NULL;
//    struct ddp_exe_status outErr;
//
//    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
//    memset(&outErr, 0, sizeof(outErr));
//    memset(&outHdr, 0, sizeof(outHdr));
//    outHdr.ipVer = inHdr->ipVer;
//    outHdr.seq = inHdr->seq;
//    outHdr.opcode = inHdr->opcode;
//    memcpy(&outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
//    outHdr.protoVer = DDP_PROTO_V1;
//    outHdr.bodyLen = DDP_REPLY_LEN_SET_BASIC_INFO;
//    if (inHdr->ipVer == IPV4_FLAG) {
//        outHdr.identifier = IPV4_REPLY;
//        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
//        outMsgLen = HDR_END_V4_OFFSET + DDP_REPLY_LEN_SET_BASIC_INFO;
//    } else {
//        outHdr.identifier = IPV6_REPLY;
//        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
//        outMsgLen = HDR_END_V6_OFFSET + DDP_REPLY_LEN_SET_BASIC_INFO;
//    }
//    /* allocate output message */
//    outMsg = (UINT1*)malloc(outMsgLen);
//    if (outMsg == NULL) {
//        DDP_DEBUG("%s (%d) : fail to allocate set basic info reply msg\n", __FILE__, __LINE__);
//        return -2;
//    }
//    memset(outMsg, 0, outMsgLen);
//    if (inHdr->ipVer == IPV4_FLAG) {
//        outBody = outMsg + HDR_END_V4_OFFSET;
//    } else {
//        outBody = outMsg + HDR_END_V6_OFFSET;
//    }
//    if (inHdr->bodyLen != DDP_REQ_LEN_SET_BASIC_INFO) {
//        outHdr.retCode = DDP_RETCODE_FAIL;
//        ret = -1;
//    }
//    /* authentication */
////    if (ddp_proto_check_op_authen(DDP_OP_SET_BASIC_INFO) == DDP_OP_AUTHEN_ON) {
////        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
////            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
////            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
////            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
////        } else {
////            ret = -2;
////        }
////        if (ret != DDP_ALL_FIELD_SUCCESS) {
////            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
////            goto process_set_basic_info_over;
////        }
////    }
//
//    ret = DDP_ALL_FIELD_SUCCESS;
//    /* retrieve field value and set to platform */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD; len = DDP_FIELD_LEN_SYSTEM_NAME;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_SYSTEM_NAME, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set system name fail (error %d)\n", __FILE__, __LINE__, ret);
//            outErr.errNo = DDP_INVALID_FIELD_NAME; outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set ip addr fail (error %d)\n", __FILE__, __LINE__, ret);
//            outErr.errNo = DDP_INVALID_FIELD_IP; outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_WEB_SERVICE_PORT;
//        tmpU2 = (UINT2)*(inBody + pos);
//        tmpU2 = DDP_NTOHS(tmpU2);
//        //ret = ddp_platform_set_field(ifs, DDP_FIELD_WEB_SERVICE_PORT, inBody + pos, len);
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_WEB_SERVICE_PORT, (UINT1*)&tmpU2, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set web service port fail (error %d)\n", __FILE__, __LINE__, ret);
//            outErr.errNo = DDP_INVALID_FIELD_PORT; outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += (len + DDP_FIELD_LEN_RESERVED_1); len = DDP_FIELD_LEN_CUSTOMIZED_DNS;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_CUSTOMIZED_DNS, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set customized dns fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL; //outErr.errNo = DDP_INVALID_FIELD_NAME;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_SUBNET_MASK;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_SUBNET_MASK, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set subnet mask fail (error %d)\n", __FILE__, __LINE__, ret);
//            outErr.errNo = DDP_INVALID_FIELD_NETMASK; outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_DEFAULT_GATEWAY;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_DEFAULT_GATEWAY, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set default gateway fail (error %d)\n", __FILE__, __LINE__, ret);
//            outErr.errNo = DDP_INVALID_FIELD_GATEWAY; outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_PRIMARY_DNS;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_PRIMARY_DNS, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set primary dns fail (error %d)\n", __FILE__, __LINE__, ret);
//            outErr.errNo = DDP_INVALID_FIELD_PRIMARY_DNS; outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_SECONDARY_DNS;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_SECONDARY_DNS, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set secondary dns fail (error %d)\n", __FILE__, __LINE__, ret);
//            outErr.errNo = DDP_INVALID_FIELD_SECONDARY_DNS; outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_DHCP;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_DHCP, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set dhcp fail (error %d)\n", __FILE__, __LINE__, ret);
//            outErr.errNo = DDP_INVALID_FIELD_DHCP; outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set ipv6 addr fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL; //outErr.errNo = DDP_INVALID_FIELD_NAME;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_DEVICE_IPV6_PREFIX;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_DEVICE_IPV6_PREFIX, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set ipv6 prefixs fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL; //outErr.errNo = DDP_INVALID_FIELD_NAME;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_DEFAULT_GATEWAY_IPV6;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_DEFAULT_GATEWAY_IPV6, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set default gateway ipv6 fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL; //outErr.errNo = DDP_INVALID_FIELD_NAME;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_PRIMARY_DNS_IPV6;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_PRIMARY_DNS_IPV6, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set primary dns ipv6 fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL; //outErr.errNo = DDP_INVALID_FIELD_NAME;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_SECONDARY_DNS_IPV6;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_SECONDARY_DNS_IPV6, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set secondary dns ipv6 fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL; //outErr.errNo = DDP_INVALID_FIELD_NAME;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_DHCP_IPV6;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_DHCP_IPV6, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set dhcp ipv6 fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL; //outErr.errNo = DDP_INVALID_FIELD_NAME;
//        }
//    }
//
//process_set_basic_info_over:
//    /* packing */
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
//        printf("Header of output packet\n");
//        print_unpack_header(&outHdr);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_EXEST) {
//        print_exe_status(&outErr);
//    }
//    pack_header(outMsg, &outHdr);
//    pack_exe_status(outBody, &outErr);
//
//    memset(&pkt, 0, sizeof(pkt));
//    pkt.ifindex = ifs->ifindex;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        inAddr = (struct sockaddr_in*)&inMsg->sender;
//        outAddr = (struct sockaddr_in*)&pkt.sender;
//        outAddr->sin_family = inAddr->sin_family;
//        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr->sin_port = htons(UDP_PORT_SERVER);
//    } else if (outHdr.ipVer == IPV6_FLAG) {
//        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
//        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
//        outAddr6->sin6_family = inAddr6->sin6_family;
//        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV6_ADDRLEN);
//        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
//        printf("output packet in hex\n");
//        print_message_hex(outMsg, outMsgLen);
//    }
//    sendout_msg(&pkt, outMsg, outMsgLen);
//
//    if (outMsg) { free(outMsg); outMsg = NULL; }
//    return ret;
//}

/* ddp_proto_user_verify (A3)
 *   function to process user verify packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input user verify request message
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_user_verify
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//    struct ddp_header outHdr;
//    struct ddp_message pkt;
//    UINT1* inBody = NULL;
//    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
//    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
//    struct sockaddr_in* inAddr = NULL;
//    struct sockaddr_in* outAddr = NULL;
//    struct sockaddr_in6* inAddr6 = NULL;
//    struct sockaddr_in6* outAddr6 = NULL;
//    UINT1* outMsg = NULL;
//    UINT1* outBody = NULL;
//    INT4 outMsgLen = 0;
//    INT4 type = 0;
//    INT4 len = 0;
//    INT4 ret = DDP_ALL_FIELD_SUCCESS;
//
//    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
//
//    memset(&outHdr, 0, sizeof(outHdr));
//    outHdr.ipVer = inHdr->ipVer;
//    outHdr.seq = inHdr->seq;
//    outHdr.opcode = inHdr->opcode;
//    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
//    outHdr.protoVer = DDP_PROTO_V1;
//    outHdr.bodyLen = DDP_REPLY_LEN_USER_VERIFY;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outHdr.identifier = IPV4_REPLY;
//        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
//        outMsgLen = HDR_END_V4_OFFSET + DDP_REPLY_LEN_USER_VERIFY;
//    } else {
//        outHdr.identifier = IPV6_REPLY;
//        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
//        outMsgLen = HDR_END_V6_OFFSET + DDP_REPLY_LEN_USER_VERIFY;
//    }
//    /* allocate output message */
//    outMsg = (UINT1*)malloc(outMsgLen);
//    if (outMsg == NULL) {
//        DDP_DEBUG("fail to allocate user verify reply msg\n");
//        return -4;
//    }
//    memset(outMsg, 0, outMsgLen);
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outBody = outMsg + HDR_END_V4_OFFSET;
//    } else {
//        outBody = outMsg + HDR_END_V6_OFFSET;
//    }
//    /* verify output message */
//    if (inHdr->bodyLen != DDP_REQ_LEN_USER_VERIFY) {
//        outHdr.retCode = DDP_RETCODE_FAIL;
//        ret = -2;
//    }
//    /* retrieve username and password from msg body and verify them */
//    if (ddp_proto_check_op_authen(DDP_OP_USER_VERIFY) == DDP_OP_AUTHEN_ON) {
//        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
//            memset(encUsername, 0, sizeof(encUsername));
//            memset(encPassword, 0, sizeof(encPassword));
//            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
//            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
//            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
//        } else {
//            ret = -3;
//        }
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
//            goto process_user_verify_over;
//        }
//    }
//    /* get user type */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        type = ddp_proto_query_user_type(ifs, encUsername, encPassword);
//        if (type < DDP_USER_TYPE_MIN || DDP_USER_TYPE_MAX < type) {
//            outHdr.retCode = DDP_RETCODE_FAIL;
//            goto process_user_verify_over;
//        }
//        type = DDP_HTONL(type);
//        memcpy(outBody, &type, sizeof(type));
//    } else {
//        outHdr.retCode = DDP_RETCODE_FAIL;
//    }
//
//process_user_verify_over:
//    /* packing */
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
//        printf("Header of output packet\n");
//        print_unpack_header(&outHdr);
//    }
//    pack_header(outMsg, &outHdr);
//
//    memset(&pkt, 0, sizeof(pkt));
//    pkt.ifindex = ifs->ifindex;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        inAddr = (struct sockaddr_in*)&inMsg->sender;
//        outAddr = (struct sockaddr_in*)&pkt.sender;
//        outAddr->sin_family = inAddr->sin_family;
//        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr->sin_port = htons(UDP_PORT_SERVER);
//    } else {
//        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
//        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
//        outAddr6->sin6_family = inAddr6->sin6_family;
//        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
//        printf("output packet in hex\n");
//        print_message_hex(outMsg, outMsgLen);
//    }
//    sendout_msg(&pkt, outMsg, outMsgLen);
//
//    if (outMsg) { free(outMsg); outMsg = NULL; }
//    return 0;
//}

/* ddp_proto_change_id_psw (A4)
 *   function to process change id password packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input change id psw request message
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_change_id_psw
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//    struct ddp_header outHdr;
//    struct ddp_message pkt;
//    UINT1* inBody = NULL;
//    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
//    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
//    struct sockaddr_in* inAddr = NULL;
//    struct sockaddr_in* outAddr = NULL;
//    struct sockaddr_in6* inAddr6 = NULL;
//    struct sockaddr_in6* outAddr6 = NULL;
//    UINT1* outMsg = NULL;
//    UINT1* outBody = NULL;
//    INT4 outMsgLen = 0;
//    INT4 len = 0;
//    struct ddp_exe_status outErr;
//    INT4 ret = DDP_ALL_FIELD_SUCCESS;
//    struct ddp_user newUser;
//
//    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
//    memset(&outErr, 0, sizeof(outErr));
//    memset(&outHdr, 0, sizeof(outHdr));
//    outHdr.ipVer = inHdr->ipVer;
//    outHdr.seq = inHdr->seq;
//    outHdr.opcode = inHdr->opcode;
//    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
//    outHdr.protoVer = DDP_PROTO_V1;
//    outHdr.bodyLen = DDP_REPLY_LEN_CHANGE_ID_PSW;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outHdr.identifier = IPV4_REPLY;
//        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
//        outMsgLen = HDR_END_V4_OFFSET + DDP_REPLY_LEN_CHANGE_ID_PSW;
//    } else {
//        outHdr.identifier = IPV6_REPLY;
//        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
//        outMsgLen = HDR_END_V6_OFFSET + DDP_REPLY_LEN_CHANGE_ID_PSW;
//    }
//    /* allocate output message */
//    outMsg = (UINT1*)malloc(outMsgLen);
//    if (outMsg == NULL) {
//        DDP_DEBUG("fail to allocate chage id psw reply msg\n");
//        return -4;
//    }
//    memset(outMsg, 0, outMsgLen);
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outBody = outMsg + HDR_END_V4_OFFSET;
//    } else {
//        outBody = outMsg + HDR_END_V6_OFFSET;
//    }
//    /* verify output message */
//    if (inHdr->bodyLen != DDP_REQ_LEN_CHANGE_ID_PSW) {
//        outHdr.retCode = DDP_RETCODE_FAIL;
//        ret = -2;
//    }
//    /* authentication */
//    if (ddp_proto_check_op_authen(DDP_OP_CHANGE_ID_PSW) == DDP_OP_AUTHEN_ON) {
//        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
//            memset(encUsername, 0, sizeof(encUsername));
//            memset(encPassword, 0, sizeof(encPassword));
//            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
//            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
//            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
//            if (ret == DDP_ALL_FIELD_SUCCESS) {
//                if (ddp_proto_query_user_type(ifs, encUsername, encPassword) != DDP_USER_TYPE_ADMIN) {
//                    ret = -3;
//                }
//            }
//        } else {
//            ret = -4;
//        }
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
//            goto process_change_id_psw_over;
//        }
//    }
//    /* set new user info */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        memset(encUsername, 0, sizeof(encUsername));
//        memset(encPassword, 0, sizeof(encPassword));
//        memcpy(encUsername, inBody + DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD, DDP_FIELD_LEN_USERNAME);
//        memcpy(encPassword, inBody + (DDP_FIELD_LEN_USERNAME * 2) + DDP_FIELD_LEN_PASSWORD, DDP_FIELD_LEN_PASSWORD);
//        memset(&newUser, 0, sizeof(newUser));
//        if (string_decode(encUsername, newUser.name, sizeof(newUser.name), DDP_ENC_BASE64) <= 0) {
//            ret = -3; outHdr.retCode = DDP_RETCODE_FAIL; outErr.errNo = DDP_INVALID_FIELD_CHANGEID_ADMIN_ILLEGAL_CHAR;
//        } else {
//            if (strlen(newUser.name) > DDP_MAX_LEN_USERNAME) {
//                ret = -3;
//                outHdr.retCode = DDP_RETCODE_FAIL;
//                outErr.errNo = DDP_INVALID_FIELD_CHANGEID_ADMIN_LENGTH; outErr.extension = DDP_MAX_LEN_USERNAME;
//            }
//        }
//        newUser.type = DDP_USER_TYPE_ADMIN;
//        if (string_decode(encPassword, newUser.pass, sizeof(newUser.pass), DDP_ENC_BASE64) <= 0) {
//            ret = -4; outHdr.retCode = DDP_RETCODE_FAIL; outErr.errNo = DDP_INVALID_FIELD_CHANGEID_PASSWORD_ILLEGAL_CHAR;
//        } else {
//            if (strlen(newUser.pass) > DDP_MAX_LEN_PASSWORD) {
//                ret = -4;
//                outHdr.retCode = DDP_RETCODE_FAIL;
//                outErr.errNo = DDP_INVALID_FIELD_CHANGEID_PASSWORD_LENGTH; outErr.extension = DDP_MAX_LEN_PASSWORD;
//            }
//        }
//
//        ret = ddp_platform_set_user(ifs, &newUser);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set user fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL;
//            outErr.errNo = (UINT2)ret;
//        }
//    }
//
//process_change_id_psw_over:
//    /* packing */
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
//        printf("Header of output packet\n");
//        print_unpack_header(&outHdr);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_EXEST) {
//        print_exe_status(&outErr);
//    }
//    pack_header(outMsg, &outHdr);
//    pack_exe_status(outBody, &outErr);
//
//    memset(&pkt, 0, sizeof(pkt));
//    pkt.ifindex = ifs->ifindex;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        inAddr = (struct sockaddr_in*)&inMsg->sender;
//        outAddr = (struct sockaddr_in*)&pkt.sender;
//        outAddr->sin_family = inAddr->sin_family;
//        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr->sin_port = htons(UDP_PORT_SERVER);
//    } else {
//        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
//        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
//        outAddr6->sin6_family = inAddr6->sin6_family;
//        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
//        printf("output packet in hex\n");
//        print_message_hex(outMsg, outMsgLen);
//    }
//    sendout_msg(&pkt, outMsg, outMsgLen);
//
//    if (outMsg) { free(outMsg); outMsg = NULL; }
//    return ret;
//}

/* ddp_proto_query_support_opt (A5)
 *   function to process query support opt packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input query support opt request message
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_query_support_opt
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//    INT4 ret  = DDP_ALL_FIELD_SUCCESS;
//    INT4 len = 0;
//    struct ddp_header outHdr;
//    struct ddp_message pkt;
//    UINT1* inBody = NULL;
//    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
//    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
//    struct sockaddr_in* inAddr = NULL;
//    struct sockaddr_in* outAddr = NULL;
//    struct sockaddr_in6* inAddr6 = NULL;
//    struct sockaddr_in6* outAddr6 = NULL;
//    UINT1* outMsg = NULL;
//    UINT1* outBody = NULL;
//    INT4 outMsgLen = 0;
//    INT4 idx = 0;
//    UINT2 tmp = 0;
//    INT4 count = 0;
//    INT4 maxOpNum = ddp_get_op_number(); /* get max op number to calculate max length of body */
//
//    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
//
//    memset(&outHdr, 0, sizeof(outHdr));
//    outHdr.ipVer = inHdr->ipVer;
//    outHdr.seq = inHdr->seq;
//    outHdr.opcode = inHdr->opcode;
//    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
//    outHdr.protoVer = DDP_PROTO_V1;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outHdr.identifier = IPV4_REPLY;
//        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
//        outMsgLen = HDR_END_V4_OFFSET + maxOpNum * sizeof(UINT2);
//    } else {
//        outHdr.identifier = IPV6_REPLY;
//        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
//        outMsgLen = HDR_END_V6_OFFSET + maxOpNum * sizeof(UINT2);
//    }
//    /* allocate output message */
//    outMsg = (UINT1*)malloc(outMsgLen);
//    if (outMsg == NULL) {
//        DDP_DEBUG("fail to allocate query support opt reply msg\n");
//        ret = -1;
//        goto process_query_opt_over;
//    }
//    memset(outMsg, 0, outMsgLen);
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outBody = outMsg + HDR_END_V4_OFFSET;
//    } else {
//        outBody = outMsg + HDR_END_V6_OFFSET;
//    }
//    /* verify input message and authentication */
//    if (inHdr->protoVer == DDP_PROTO_V1) {
//        if (inHdr->bodyLen != DDP_REQ_LEN_QUERY_SUPPORT_OPT) {
//            outHdr.retCode = DDP_RETCODE_FAIL;
//            ret = -1;
//        }
//    } else if (inHdr->protoVer == DDP_PROTO_V2) {
//        if (inHdr->bodyLen != 0) {
//            outHdr.retCode = DDP_RETCODE_FAIL;
//            ret = -1;
//        }
//    }
//    /* retrieve username and password from msg body and verify them */
//    if (ddp_proto_check_op_authen(DDP_OP_QUERY_SUPPORT_OPT) == DDP_OP_AUTHEN_ON) {
//        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
//            memset(encUsername, 0, sizeof(encUsername));
//            memset(encPassword, 0, sizeof(encPassword));
//            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
//            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
//            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
//        } else {
//            ret = -2;
//        }
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
//            goto process_query_opt_over;
//        }
//    }
//    /* get available op */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        for (idx = 0; idx < maxOpNum; idx++) {
//            if (g_opList[idx].opcode == 0 || g_opList[idx].enable != DDP_OP_ENABLE) { continue; }
//            if (ddp_platform_op_support(ifs, g_opList[idx].opcode) != 1) { continue; }
//            if (0x00A1 <= g_opList[idx].opcode && g_opList[idx].opcode <= 0x00A5) { continue; }
//            tmp = g_opList[idx].opcode;
//            tmp = DDP_HTONS(tmp);
//            memcpy(outBody + count * sizeof(UINT2), &tmp, sizeof(UINT2));
//            count++;
//        }
//        /* count in the last 0 element */
//        count++;
//    }
//
//process_query_opt_over:
//    memset(&pkt, 0, sizeof(pkt));
//    pkt.ifindex = ifs->ifindex;
//    /* re-write body len firld in header */
//    outHdr.bodyLen = count * sizeof(UINT2);
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
//
//        inAddr = (struct sockaddr_in*)&inMsg->sender;
//        outAddr = (struct sockaddr_in*)&pkt.sender;
//        outAddr->sin_family = inAddr->sin_family;
//        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr->sin_port = htons(UDP_PORT_SERVER);
//    } else {
//        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
//
//        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
//        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
//        outAddr6->sin6_family = inAddr6->sin6_family;
//        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
//    }
//    /* packing */
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
//        printf("Header of output packet\n");
//        print_unpack_header(&outHdr);
//    }
//    pack_header(outMsg, &outHdr);
//
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
//        printf("output packet in hex\n");
//        print_message_hex(outMsg, outMsgLen);
//    }
//    sendout_msg(&pkt, outMsg, outMsgLen);
//
//    if (outMsg) { free(outMsg); outMsg = NULL; }
//
//    return ret;
//}

/* ddp_proto_device_alert_info (200)
 *   function to process device alert info packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input device alert info request message
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_device_alert_info
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//    INT4 ret = 0;
//    struct ddp_header outHdr;
//    UINT1* outMsg = NULL;
//    UINT1* outBody = NULL;
//    INT4 outMsgLen = 0;
//    INT4 len = 0;
//    struct ddp_message pkt;
//    struct sockaddr_in* inAddr = NULL;
//    struct sockaddr_in* outAddr = NULL;
//    struct sockaddr_in6* inAddr6 = NULL;
//    struct sockaddr_in6* outAddr6 = NULL;
//
//    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
//
//    memset(&outHdr, 0, sizeof(outHdr));
//    outHdr.ipVer = inHdr->ipVer;
//    outHdr.seq = inHdr->seq;
//    outHdr.opcode = inHdr->opcode;
//    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
//    outHdr.protoVer = DDP_PROTO_V1;
//    outHdr.bodyLen = DDP_REPLY_LEN_DEVICE_ALERT_INFO;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outHdr.identifier = IPV4_REPLY;
//        len = IPV4_ADDRLEN;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, outHdr.ipAddr.ipv4Addr, &len);
//        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
//    } else {
//        outHdr.identifier = IPV6_REPLY;
//        len = IPV6_ADDRLEN;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, outHdr.ipAddr.ipv6Addr, &len);
//        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
//    }
//    /* allocate output message */
//    outMsg = (UINT1*)malloc(outMsgLen);
//    if (outMsg == NULL) {
//        DDP_DEBUG("fail to allocate device alert info reply msg\n");
//        return -4;
//    }
//    memset(outMsg, 0, outMsgLen);
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outBody = outMsg + HDR_END_V4_OFFSET;
//    } else {
//        outBody = outMsg + HDR_END_V6_OFFSET;
//    }
//    /* get alert report info */
//    len = outHdr.bodyLen;
//    ret = ddp_platform_get_field(ifs, DDP_FIELD_ALERT_REPORT_INFO, outBody, &len);
//    if (ret != DDP_ALL_FIELD_SUCCESS) {
//        outHdr.retCode = DDP_RETCODE_FAIL;
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
//        printf("Header of output packet\n");
//        print_unpack_header(&outHdr);
//    }
//    pack_header(outMsg, &outHdr);
//
//    memset(&pkt, 0, sizeof(pkt));
//    pkt.ifindex = ifs->ifindex;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        inAddr = (struct sockaddr_in*)&inMsg->sender;
//        outAddr = (struct sockaddr_in*)&pkt.sender;
//        outAddr->sin_family = inAddr->sin_family;
//        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr->sin_port = htons(UDP_PORT_SERVER);
//    } else {
//        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
//        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
//        outAddr6->sin6_family = inAddr6->sin6_family;
//        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
//        printf("output packet in hex\n");
//        print_message_hex(outMsg, outMsgLen);
//    }
//    sendout_msg(&pkt, outMsg, outMsgLen);
//
//    if (outMsg) { free(outMsg); outMsg = NULL; }
//
//    return ret;
//}

/* ddp_proto_snmp_cfg (D4)
 *   function to process snmp cfg packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input snmp cfg request message
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_snmp_cfg
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//#define SNMP_CFG_STR_LEN (32 + 1)
//    INT4 ret = DDP_ALL_FIELD_SUCCESS;
//    UINT1* inBody = NULL;
//    struct ddp_header outHdr;
//    UINT1* outMsg = NULL;
//    UINT1* outBody = NULL;
//    INT4 outMsgLen = 0;
//    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
//    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
//    struct ddp_message pkt;
//    struct sockaddr_in* inAddr = NULL;
//    struct sockaddr_in* outAddr = NULL;
//    struct sockaddr_in6* inAddr6 = NULL;
//    struct sockaddr_in6* outAddr6 = NULL;
//    struct ddp_exe_status outErr;
//    INT4 len = 0;
//    INT4 pos = 0;
//    INT1 str[SNMP_CFG_STR_LEN];
//
//    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
//
//    memset(&outErr, 0, sizeof(outErr));
//    memset(&outHdr, 0, sizeof(outHdr));
//    outHdr.ipVer = inHdr->ipVer;
//    outHdr.seq = inHdr->seq;
//    outHdr.opcode = inHdr->opcode;
//    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
//    outHdr.protoVer = DDP_PROTO_V1;
//    outHdr.bodyLen = DDP_REPLY_LEN_SNMP_CFG;
//    if (inHdr->ipVer == IPV4_FLAG) {
//        outHdr.identifier = IPV4_REPLY;
//        len = IPV4_ADDRLEN;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, outHdr.ipAddr.ipv4Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
//        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
//    } else {
//        outHdr.identifier = IPV6_REPLY;
//        len = IPV6_ADDRLEN;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, outHdr.ipAddr.ipv6Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
//        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
//    }
//    /* allocate output message */
//    outMsg = (UINT1*)malloc(outMsgLen);
//    if (outMsg == NULL) {
//        DDP_DEBUG("fail to allocate snmp cfg reply msg\n");
//        return -3;
//    }
//    memset(outMsg, 0, outMsgLen);
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outBody = outMsg + HDR_END_V4_OFFSET;
//    } else {
//        outBody = outMsg + HDR_END_V6_OFFSET;
//    }
//    /* verify input message */
//    if (inHdr->bodyLen != DDP_REQ_LEN_SNMP_CFG || inBody == NULL) {
//        outHdr.retCode = DDP_RETCODE_FAIL;
//        ret = -1;
//        goto process_snmp_cfg_over;
//    }
//    /* process input body */
//    pos = 0; len = 0;
//    /* retrieve username and password from msg body and verify them */
//    if (ddp_proto_check_op_authen(DDP_OP_SNMP_CFG) == DDP_OP_AUTHEN_ON) {
//        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
//            memset(encUsername, 0, sizeof(encUsername));
//            memset(encPassword, 0, sizeof(encPassword));
//            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
//            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
//            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
//        } else {
//            ret = -2;
//        }
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
//            goto process_snmp_cfg_over;
//        }
//        pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
//    }
//    /* retrieve field value */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        len = DDP_FIELD_LEN_SNMP_GLOBAL_STATE;
//        ret = ddp_platform_set_field(ifs, DDP_FIELD_SNMP_GLOBAL_STATE, inBody + pos, len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set snmp global state fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO;
//        memset(str, 0, SNMP_CFG_STR_LEN);
//        memcpy(str, inBody + pos, len);
//        if (strlen(str) > 0) {
//            ret = ddp_platform_set_field(ifs, DDP_FIELD_SNMP_COMMUNITY_NAME_RO, (UINT1*)str, strlen(str));
//        }
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set snmp community name RO fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW;
//        memset(str, 0, SNMP_CFG_STR_LEN);
//        memcpy(str, inBody + pos, len);
//        if (strlen(str) > 0) {
//            ret = ddp_platform_set_field(ifs, DDP_FIELD_SNMP_COMMUNITY_NAME_RW, (UINT1*)str, strlen(str));
//        }
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : set snmp community name RW fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_SNMP_VIEW_NAME_RO;
//        memset(str, 0, SNMP_CFG_STR_LEN);
//        memcpy(str, inBody + pos, len);
//        if (strlen(str) > 0) {
//            ret = ddp_platform_set_field(ifs, DDP_FIELD_SNMP_VIEW_NAME_RO, (UINT1*)str, strlen(str));
//            if (ret != DDP_ALL_FIELD_SUCCESS) {
//                DDP_DEBUG("%s (%d) : set snmp view name RO fail (error %d)\n", __FILE__, __LINE__, ret);
//            }
//        } else {
//            ret = DDP_INVALID_FIELD_SNMP_VIEWNAME_LOST;
//        }
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            outHdr.retCode = DDP_RETCODE_FAIL; outErr.errNo = ret;
//        }
//    }
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        pos += len; len = DDP_FIELD_LEN_SNMP_VIEW_NAME_RW;
//        memset(str, 0, SNMP_CFG_STR_LEN);
//        memcpy(str, inBody + pos, len);
//        if (strlen(str) > 0) {
//            ret = ddp_platform_set_field(ifs, DDP_FIELD_SNMP_VIEW_NAME_RW, (UINT1*)str, strlen(str));
//            if (ret != DDP_ALL_FIELD_SUCCESS) {
//                DDP_DEBUG("%s (%d) : set snmp view name RW fail (error %d)\n", __FILE__, __LINE__, ret);
//            }
//        } else {
//            ret = DDP_INVALID_FIELD_SNMP_VIEWNAME_LOST;
//        }
//        if (ret != DDP_ALL_FIELD_SUCCESS) { outHdr.retCode = DDP_RETCODE_FAIL; outErr.errNo = ret; }
//    }
//
//process_snmp_cfg_over:
//    /* packing */
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
//        printf("Header of output packet\n");
//        print_unpack_header(&outHdr);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_EXEST) {
//        print_exe_status(&outErr);
//    }
//    pack_header(outMsg, &outHdr);
//    pack_exe_status(outBody, &outErr);
//
//    memset(&pkt, 0, sizeof(pkt));
//    pkt.ifindex = ifs->ifindex;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        inAddr = (struct sockaddr_in*)&inMsg->sender;
//        outAddr = (struct sockaddr_in*)&pkt.sender;
//        outAddr->sin_family = inAddr->sin_family;
//        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr->sin_port = htons(UDP_PORT_SERVER);
//    } else {
//        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
//        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
//        outAddr6->sin6_family = inAddr6->sin6_family;
//        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
//        printf("output packet in hex\n");
//        print_message_hex(outMsg, outMsgLen);
//    }
//    sendout_msg(&pkt, outMsg, outMsgLen);
//
//    if (outMsg) { free(outMsg); outMsg = NULL; }
//    return ret;
//}

/* ddp_proto_snmp_get (DA)
 *   function to process snmp get packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input snmp get request message
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_snmp_get
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//#define SNMP_MAX_NUMBER 32
//    INT4 ret = DDP_ALL_FIELD_SUCCESS;
//    //UINT1* inBody = NULL;
//    struct ddp_header outHdr;
//    UINT1* outMsg = NULL;
//    UINT1* outBody = NULL;
//    INT4 outMsgLen = 0;
//    struct ddp_message pkt;
//    struct sockaddr_in* inAddr = NULL;
//    struct sockaddr_in* outAddr = NULL;
//    struct sockaddr_in6* inAddr6 = NULL;
//    struct sockaddr_in6* outAddr6 = NULL;
//    INT4 len = 0;
//    INT4 pos = 0;
//    UINT1 global_state = 0;
//    INT4 count = 0;
//    INT4 finish = 0;
//    struct snmp_container communityRO;
//    struct snmp_container communityRW;
//    struct snmp_container viewRO;
//    struct snmp_container viewRW;
//
//    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
//
//    memset(&outHdr, 0, sizeof(outHdr));
//    outHdr.ipVer = inHdr->ipVer;
//    outHdr.seq = inHdr->seq;
//    outHdr.opcode = inHdr->opcode;
//    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
//    outHdr.protoVer = DDP_PROTO_V1;
//    outHdr.bodyLen = DDP_REPLY_LEN_SNMP_GET;
//    if (inHdr->ipVer == IPV4_FLAG) {
//        outHdr.identifier = IPV4_REPLY;
//        len = IPV4_ADDRLEN;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, outHdr.ipAddr.ipv4Addr, &len);
//        //if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
//        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
//    } else {
//        outHdr.identifier = IPV6_REPLY;
//        len = IPV6_ADDRLEN;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, outHdr.ipAddr.ipv6Addr, &len);
//        //if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
//        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
//    }
//    /* allocate output message */
//    outMsg = (UINT1*)malloc(outMsgLen);
//    if (outMsg == NULL) {
//        DDP_DEBUG("fail to allocate snmp cfg reply msg\n");
//        return -3;
//    }
//    memset(outMsg, 0, outMsgLen);
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outBody = outMsg + HDR_END_V4_OFFSET;
//    } else {
//        outBody = outMsg + HDR_END_V6_OFFSET;
//    }
//
//    /* retrieve field value */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        len = DDP_FIELD_LEN_SNMP_GLOBAL_STATE;
//        ret = ddp_platform_get_field(ifs, DDP_FIELD_SNMP_GLOBAL_STATE, (UINT1*)&global_state, &len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : get snmp global state fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    /* get community name ro */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        communityRO.number = 0;
//        len = DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO;
//        ret = ddp_platform_get_field(ifs, DDP_FIELD_SNMP_COMMUNITY_NAME_RO, (UINT1*)&communityRO, &len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : get snmp community name ro fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    /* get community name rw */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        communityRW.number = 0;
//        len = DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW;
//        ret = ddp_platform_get_field(ifs, DDP_FIELD_SNMP_COMMUNITY_NAME_RW, (UINT1*)&communityRW, &len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : get snmp community name rw fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    /* get view name ro */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        viewRO.number = 0;
//        len = DDP_FIELD_LEN_SNMP_VIEW_NAME_RO;
//        ret = ddp_platform_get_field(ifs, DDP_FIELD_SNMP_VIEW_NAME_RO, (UINT1*)&viewRO, &len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : get snmp view name ro fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//    /* get view name rw */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        viewRW.number = 0;
//        len = DDP_FIELD_LEN_SNMP_VIEW_NAME_RW;
//        ret = ddp_platform_get_field(ifs, DDP_FIELD_SNMP_VIEW_NAME_RW, (UINT1*)&viewRW, &len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            DDP_DEBUG("%s (%d) : get snmp view name rw fail (error %d)\n", __FILE__, __LINE__, ret);
//            outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//
//    /* loop to construct output message */
//    count = 0;
//    while (finish == 0) {
//        while (communityRO.number > 0) {
//            if (count < SNMP_MAX_NUMBER) {
//                pos = count * 33 + 1;
//                *(outBody + pos) = 0x01;
//                pos += 1;
//                len = DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO;
//                memcpy(outBody + pos, communityRO.list + (communityRO.number - 1) * DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO, DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO);
//                count++;
//                communityRO.number--;
//            } else {
//                goto process_snmp_get_send;
//            }
//        }
//        while (communityRW.number > 0) {
//            if (count < SNMP_MAX_NUMBER) {
//                pos = count * 33 + 1;
//                *(outBody + pos) = 0x02;
//                pos += 1;
//                len = DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW;
//                memcpy(outBody + pos, communityRW.list + (communityRW.number - 1) * DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW, DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW);
//                count++;
//                communityRW.number--;
//            } else {
//                goto process_snmp_get_send;
//            }
//        }
//        while (viewRO.number > 0) {
//            if (count < SNMP_MAX_NUMBER) {
//                pos = count * 33 + 1;
//                *(outBody + pos) = 0x03;
//                pos += 1;
//                len = DDP_FIELD_LEN_SNMP_VIEW_NAME_RO;
//                memcpy(outBody + pos, viewRO.list + (viewRO.number - 1) * DDP_FIELD_LEN_SNMP_VIEW_NAME_RO, DDP_FIELD_LEN_SNMP_VIEW_NAME_RO);
//                count++;
//                viewRO.number--;
//            } else {
//                goto process_snmp_get_send;
//            }
//        }
//        while (viewRW.number > 0) {
//            if (count < SNMP_MAX_NUMBER) {
//                pos = count * 33 + 1;
//                *(outBody + pos) = 0x04;
//                pos += 1;
//                len = DDP_FIELD_LEN_SNMP_VIEW_NAME_RW;
//                memcpy(outBody + pos, viewRW.list + (viewRW.number - 1) * DDP_FIELD_LEN_SNMP_VIEW_NAME_RW, DDP_FIELD_LEN_SNMP_VIEW_NAME_RW);
//                count++;
//                viewRW.number--;
//            } else {
//                goto process_snmp_get_send;
//            }
//        }
//
//process_snmp_get_send:
//        /* add global state to out message */
//        *outBody = global_state;
//        /* check finish signal */
//        if (communityRO.number == 0 && communityRW.number == 0 && viewRO.number == 0 && viewRW.number == 0) {
//            finish = 1;
//        }
//        /* append termination flag and calculate body length according to finish flag */
//        if (finish == 0) {
//            pos = count * 33 + 1;
//            *(outBody + pos) = 0x00; *(outBody + pos + 1) = 0x00;
//            *(outBody + pos + 2) = 0xFF; *(outBody + pos + 3) = 0xFF;
//            outHdr.bodyLen = pos + 4;
//        } else {
//            pos = count * 33 + 1;
//            *(outBody + pos) = 0x00; *(outBody + pos + 1) = 0x00;
//            *(outBody + pos + 2) = 0x00; *(outBody + pos + 3) = 0x00;
//            outHdr.bodyLen = pos + 4;
//        }
//        /* calculate msg len according to new body length */
//        if (outHdr.ipVer == IPV4_FLAG) {
//            outMsgLen = outHdr.bodyLen + HDR_END_V4_OFFSET;
//        } else {
//            outMsgLen = outHdr.bodyLen + HDR_END_V6_OFFSET;
//        }
//        /* packing */
//        if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
//            printf("Header of output packet\n");
//            print_unpack_header(&outHdr);
//        }
//        pack_header(outMsg, &outHdr);
//
//        memset(&pkt, 0, sizeof(pkt));
//        pkt.ifindex = ifs->ifindex;
//        if (outHdr.ipVer == IPV4_FLAG) {
//            inAddr = (struct sockaddr_in*)&inMsg->sender;
//            outAddr = (struct sockaddr_in*)&pkt.sender;
//            outAddr->sin_family = inAddr->sin_family;
//            memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
//            //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//            outAddr->sin_port = htons(UDP_PORT_SERVER);
//        } else {
//            inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
//            outAddr6 = (struct sockaddr_in6*)&pkt.sender;
//            outAddr6->sin6_family = inAddr6->sin6_family;
//            memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
//            //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//            outAddr6->sin6_port = htons(UDP_PORT_SERVER);
//        }
//        if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
//            printf("output packet in hex\n");
//            print_message_hex(outMsg, outMsgLen);
//        }
//        sendout_msg(&pkt, outMsg, outMsgLen);
//        /* reset count */
//        count = 0;
//    } /* while */
//
//    /* */
//    if (communityRO.list) { free(communityRO.list); communityRO.list = NULL; }
//    if (communityRW.list) { free(communityRW.list); communityRW.list = NULL; }
//    if (viewRO.list) { free(viewRO.list); viewRO.list = NULL; }
//    if (viewRW.list) { free(viewRW.list); viewRW.list = NULL; }
//    if (outMsg) { free(outMsg); outMsg = NULL; }
//    return ret;
//}

/* ddp_proto_reboot (103)
 *   function to process reboot packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input reboot request message
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_reboot
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//    INT4 ret = DDP_ALL_FIELD_SUCCESS;
//    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
//    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
//    UINT1* inBody = NULL;
//    struct ddp_header outHdr;
//    UINT1* outMsg = NULL;
//    UINT1* outBody = NULL;
//    INT4 outMsgLen = 0;
//    struct ddp_message pkt;
//    struct sockaddr_in* inAddr = NULL;
//    struct sockaddr_in* outAddr = NULL;
//    struct sockaddr_in6* inAddr6 = NULL;
//    struct sockaddr_in6* outAddr6 = NULL;
//    struct ddp_exe_status outErr;
//    INT4 len = 0;
//    INT4 pos = 0;
//    UINT4 delay = 0;
//    struct ddp_message *delay_msg = NULL;
//
//    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
//
//    memset(&outErr, 0, sizeof(outErr));
//    memset(&outHdr, 0, sizeof(outHdr));
//    outHdr.ipVer = inHdr->ipVer;
//    outHdr.seq = inHdr->seq;
//    outHdr.opcode = inHdr->opcode;
//    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
//    outHdr.protoVer = DDP_PROTO_V1;
//    outHdr.bodyLen = DDP_REPLY_LEN_REBOOT;
//    if (inHdr->ipVer == IPV4_FLAG) {
//        outHdr.identifier = IPV4_REPLY;
//        len = IPV4_ADDRLEN;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, outHdr.ipAddr.ipv4Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
//        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
//    } else {
//        outHdr.identifier = IPV6_REPLY;
//        len = IPV6_ADDRLEN;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, outHdr.ipAddr.ipv6Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
//        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
//    }
//    /* allocate output message */
//    outMsg = (UINT1*)malloc(outMsgLen);
//    if (outMsg == NULL) {
//        DDP_DEBUG("fail to allocate reboot reply msg\n");
//        return -3;
//    }
//    memset(outMsg, 0, outMsgLen);
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outBody = outMsg + HDR_END_V4_OFFSET;
//    } else {
//        outBody = outMsg + HDR_END_V6_OFFSET;
//    }
//
//    /* verify input message */
//    if (inHdr->bodyLen < DDP_REQ_LEN_REBOOT) {
//        outHdr.retCode = DDP_RETCODE_FAIL;
//        ret = -1;
//    }
//    pos = 0;
//    /* authentication */
//    if (ddp_proto_check_op_authen(DDP_OP_REBOOT) == DDP_OP_AUTHEN_ON) {
//        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
//            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
//            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
//            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
//        } else {
//            ret = -2;
//        }
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
//            goto process_reboot_over;
//        }
//        pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
//    }
//    /* get wait timer first */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        /* Delay time */
//        memcpy((void*)&delay, (void*)(inBody + pos), DDP_FIELD_LEN_DELAY_TIME);
//        delay = DDP_NTOHL(delay);
//        /* get wait time from platform to return */
//        len = DDP_FIELD_LEN_REBOOT_WAIT_TIME;
//        ret = ddp_platform_get_field(ifs, DDP_FIELD_REBOOT_WAIT_TIME, (UINT1*)&outErr.waitTimer, &len);
//        if (ret == DDP_ALL_FIELD_SUCCESS) {
//            outErr.opStatus = DDP_EXE_ST_DONE;//RENEWAL;
//        } else {
//            outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//
//process_reboot_over:
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
//        printf("Header of output packet\n");
//        print_unpack_header(&outHdr);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_EXEST) {
//        print_exe_status(&outErr);
//    }
//
//    memset(&pkt, 0, sizeof(pkt));
//    pkt.ifindex = ifs->ifindex;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        inAddr = (struct sockaddr_in*)&inMsg->sender;
//        outAddr = (struct sockaddr_in*)&pkt.sender;
//        outAddr->sin_family = inAddr->sin_family;
//        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr->sin_port = htons(UDP_PORT_SERVER);
//    } else {
//        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
//        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
//        outAddr6->sin6_family = inAddr6->sin6_family;
//        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
//        printf("output packet in hex\n");
//        print_message_hex(outMsg, outMsgLen);
//    }
//
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        /* delayed reboot */
//        if (delay > 0) {
//            DDP_DEBUG("Reboot is delayed for %d seconds\n", delay);
//            delay_msg = create_delay_msg(ifs, inHdr, inMsg);
//            if (delay_msg == NULL) {
//                outHdr.retCode = DDP_RETCODE_FAIL;
//                ret = -4;
//            } else {
//                ret = ddp_alrm_queue_add(DDP_OP_REBOOT, delay, delay_msg);
//                if (ret != 0) {
//                    outHdr.retCode = DDP_RETCODE_FAIL;
//                    ret = -5;
//                }
//            }
//            pack_header(outMsg, &outHdr);
//            pack_exe_status(outBody, &outErr);
//            sendout_msg(&pkt, outMsg, outMsgLen);
//        }
//        /* reboot immediately */
//        else {
//            /* reply if original message */
//            if (inHdr->retCode == 0x7777) {
//                pack_header(outMsg, &outHdr);
//                pack_exe_status(outBody, &outErr);
//                sendout_msg(&pkt, outMsg, outMsgLen);
//            }
//            len = 1;
//            /* start to reboot */
//            ret = ddp_platform_set_field(ifs, DDP_FIELD_REBOOT, outBody, len);
//            /* process should not come back if reboot is successful */
//            if (ret != DDP_ALL_FIELD_SUCCESS) {
//                DDP_DEBUG("%s (%d) : set reboot fail (error %d)\n", __FILE__, __LINE__, ret);
//                outHdr.retCode = DDP_RETCODE_FAIL;
//                pack_header(outMsg, &outHdr);
//                pack_exe_status(outBody, &outErr);
//                sendout_msg(&pkt, outMsg, outMsgLen);
//            }
//        }
//    } else {
//        pack_header(outMsg, &outHdr);
//        pack_exe_status(outBody, &outErr);
//        sendout_msg(&pkt, outMsg, outMsgLen);
//    }
//    if (outMsg) { free(outMsg); outMsg = NULL; }
//    return ret;
//}

/* ddp_proto_ddp_info (224)
 *   function to process ddp info packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input ddp info request message
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_ddp_info
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//    INT4 ret = DDP_ALL_FIELD_SUCCESS;
//    UINT1* inBody = NULL;
//    struct ddp_header outHdr;
//    UINT1* outMsg = NULL;
//    UINT1* outBody = NULL;
//    INT4 outMsgLen = 0;
//    struct ddp_message pkt;
//    struct sockaddr_in* inAddr = NULL;
//    struct sockaddr_in* outAddr = NULL;
//    struct sockaddr_in6* inAddr6 = NULL;
//    struct sockaddr_in6* outAddr6 = NULL;
//    INT4 len = 0;
//
//    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
//
//    memset(&outHdr, 0, sizeof(outHdr));
//    outHdr.ipVer = inHdr->ipVer;
//    outHdr.seq = inHdr->seq;
//    outHdr.opcode = inHdr->opcode;
//    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
//    outHdr.protoVer = DDP_PROTO_V1;
//    outHdr.bodyLen = DDP_REPLY_LEN_DDP_INFO;
//    if (inHdr->ipVer == IPV4_FLAG) {
//        outHdr.identifier = IPV4_REPLY;
//        len = IPV4_ADDRLEN;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, outHdr.ipAddr.ipv4Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
//        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
//    } else {
//        outHdr.identifier = IPV6_REPLY;
//        len = IPV6_ADDRLEN;
//        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, outHdr.ipAddr.ipv6Addr, &len);
//        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
//        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
//    }
//    /* allocate output message */
//    outMsg = (UINT1*)malloc(outMsgLen);
//    if (outMsg == NULL) {
//        DDP_DEBUG("fail to allocate ddp info reply msg\n");
//        return -3;
//    }
//    memset(outMsg, 0, outMsgLen);
//    if (outHdr.ipVer == IPV4_FLAG) {
//        outBody = outMsg + HDR_END_V4_OFFSET;
//    } else {
//        outBody = outMsg + HDR_END_V6_OFFSET;
//    }
//
//    /* get wait timer first */
//    if (ret == DDP_ALL_FIELD_SUCCESS) {
//        len = DDP_FIELD_LEN_SPEC_VERSION;
//        ret = ddp_platform_get_field(ifs, DDP_FIELD_SPEC_VERSION, outBody, &len);
//        if (ret != DDP_ALL_FIELD_SUCCESS) {
//            outHdr.retCode = DDP_RETCODE_FAIL;
//        }
//    }
//
//process_ddp_info_over:
//    /* packing */
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
//        printf("Header of output packet\n");
//        print_unpack_header(&outHdr);
//    }
//    pack_header(outMsg, &outHdr);
//
//    memset(&pkt, 0, sizeof(pkt));
//    pkt.ifindex = ifs->ifindex;
//    if (outHdr.ipVer == IPV4_FLAG) {
//        inAddr = (struct sockaddr_in*)&inMsg->sender;
//        outAddr = (struct sockaddr_in*)&pkt.sender;
//        outAddr->sin_family = inAddr->sin_family;
//        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr->sin_port = htons(UDP_PORT_SERVER);
//    } else {
//        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
//        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
//        outAddr6->sin6_family = inAddr6->sin6_family;
//        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
//        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
//        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
//    }
//    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
//        printf("output packet in hex\n");
//        print_message_hex(outMsg, outMsgLen);
//    }
//    sendout_msg(&pkt, outMsg, outMsgLen);
//
//    if (outMsg) { free(outMsg); outMsg = NULL; }
//    return ret;
//}

