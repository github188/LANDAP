
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "ddp.h"
#include "ddp_platform.h"

/* ddp_proto_set_multi_ssid (D1)
 *   function to process set multiple ssid packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input set multiple ssid request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_set_multi_ssid
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = DDP_ALL_FIELD_SUCCESS;
    UINT1* inBody = NULL;
    UINT1* outMsg = NULL;
    struct ddp_header outHdr;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    INT1 band = 0;
    INT4 len = 0;
    INT4 pos = 0;
    struct ddp_message pkt;
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    struct ddp_exe_status outErr;
    struct ssid_entry table[SSID_NUMBER];
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
    memset(&outErr, 0, sizeof(outErr));
    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(&outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V1;
    outHdr.bodyLen = DDP_REPLY_LEN_SET_MULTI_SSID;
    if (inHdr->ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + DDP_REPLY_LEN_SET_MULTI_SSID;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + DDP_REPLY_LEN_SET_MULTI_SSID;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("%s (%d) : fail to allocate set multiple ssid reply msg\n", __FILE__, __LINE__);
        return -2;
    }
    memset(outMsg, 0, outMsgLen);
    if (inHdr->ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    if (inHdr->bodyLen != DDP_REQ_LEN_SET_MULTI_SSID || inBody == NULL) {
        ret = -1;
        DDP_DEBUG("%s (%d) : set multi ssid fail (incorrect body len)\n", __FILE__, __LINE__);
        outErr.errNo = DDP_INVALID_FIELD_SERVER_MESSAGE_TYPE_ERROR; outHdr.retCode = DDP_RETCODE_FAIL;
        goto process_set_multi_ssid_over;
    }
    
    pos = 0; len = 0;
    /* retrieve username and password from msg body and verify them */
    if (ddp_proto_check_op_authen(DDP_OP_SET_MULTI_SSID) == DDP_OP_AUTHEN_ON) {
        if (ret == DDP_ALL_FIELD_SUCCESS && inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            ret = ddp_proto_verify_authen(ifs, encUsername, encPassword);
        } else {
            ret = -2;
        }
        if (ret != DDP_ALL_FIELD_SUCCESS) {
            outHdr.retCode = DDP_RETCODE_AUTHEN_FAIL;
            goto process_set_multi_ssid_over;
        }
        pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
    }
    /* set ssid to platform */
    if (ret == DDP_ALL_FIELD_SUCCESS) {
        band = *(inBody + pos);

        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "Set multi ssid body content\n");
        if (band == DDP_WL_BAND_24GHZ) {
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "Frequency band : 2.4GHz\n");
        } else if (band == DDP_WL_BAND_5GHZ) {
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "Frequency band : 5GHz\n");
        } else {
            DDP_DEBUG("%s (%d) : unknown frequency band\n", __FILE__, __LINE__);
            ret = -2;
            outHdr.retCode = DDP_RETCODE_FAIL;
            outErr.errNo = DDP_INVALID_FIELD_SERVER_MESSAGE_TYPE_ERROR;
            goto process_set_multi_ssid_over;
        }

        memset(table, 0, sizeof(struct ssid_entry) * SSID_NUMBER);
        pos += 1;
        for (len = 0; len < SSID_NUMBER; len++) {
            memcpy(table[len].name, inBody + pos, DDP_FIELD_LEN_SSID_NAME);
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "SSID %d name : %s\n", len, table[len].name);
            pos += DDP_FIELD_LEN_SSID_NAME;
            memcpy(&table[len].vid, inBody + pos, DDP_FIELD_LEN_VLAN_ID);
            table[len].vid = DDP_NTOHL(table[len].vid);
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "        VID : %d\n", table[len].vid);
            pos += DDP_FIELD_LEN_VLAN_ID;
            memcpy(&table[len].security, inBody + pos, DDP_FIELD_LEN_SSID_SEC_TYPE);
            table[len].security = DDP_NTOHL(table[len].security);
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "        Sec : %d\n", table[len].security);
            pos += DDP_FIELD_LEN_SSID_SEC_TYPE;
            memcpy(table[len].password, inBody + pos, DDP_FIELD_LEN_SSID_SEC_PASSWD);
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "     Passwd : %s\n", table[len].password);
            pos += DDP_FIELD_LEN_SSID_SEC_PASSWD;
        }
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "\n");
        
        ret = ddp_platform_set_multi_ssid(band, table);
        if (ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG("%s (%d) : set multi ssid platform fail (error %d)\n", __FILE__, __LINE__, ret);
            outErr.errNo = DDP_INVALID_FIELD_SERVER_MESSAGE_TYPE_ERROR; outHdr.retCode = DDP_RETCODE_FAIL;
        }
    }

process_set_multi_ssid_over:
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&outHdr);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_EXEST) {
        print_exe_status(&outErr);
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
    } else if (outHdr.ipVer == IPV6_FLAG) {
        inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
        outAddr6 = (struct sockaddr_in6*)&pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV6_ADDRLEN);
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

/* ddp_proto_get_multi_ssid (D1)
 *   function to process get multiple ssid packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input get multiple ssid request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_get_multi_ssid
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = DDP_ALL_FIELD_SUCCESS;
    UINT1* inBody = NULL;
    UINT1* outMsg = NULL;
    struct ddp_header outHdr;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    INT4 pos = 0;
    INT4 len = 0;
    UINT1 band = 0;
    INT4 idx = 0;
    struct ddp_message pkt;
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    struct ssid_entry table[SSID_NUMBER];

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }
    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(&outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V1;
    outHdr.bodyLen = DDP_REPLY_LEN_GET_MULTI_SSID;
    if (inHdr->ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, (UINT1*)&outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + DDP_REPLY_LEN_GET_MULTI_SSID;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, (UINT1*)&outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + DDP_REPLY_LEN_GET_MULTI_SSID;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("%s (%d) : fail to allocate get multiple ssid reply msg\n", __FILE__, __LINE__);
        return -2;
    }
    memset(outMsg, 0, outMsgLen);
    if (inHdr->ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    if (inHdr->bodyLen != DDP_REQ_LEN_GET_MULTI_SSID) {
        outHdr.retCode = DDP_RETCODE_FAIL;
        ret = -1;
        *outBody = 0xFF;
        if (outHdr.ipVer == IPV4_FLAG) {
            outMsgLen = HDR_END_V4_OFFSET + 3;
        } else {
            outMsgLen = HDR_END_V6_OFFSET + 3;
        }
        goto process_get_multi_ssid_over_5g;
    }
    
    /* retrieve field value and set to platform */
    band = *inBody;
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "Get multi ssid body content\n");
    if (band == DDP_WL_BAND_24GHZ) {
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "Frequency band : 2.4GHz\n");
    } else if (band == DDP_WL_BAND_5GHZ) {
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "Frequency band : 5GHz\n");
    } else if (band == DDP_WL_BAND_ALL) {
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "All frequency bands\n");
    } else {
        DDP_DEBUG("%s (%d) : unknown frequency band\n", __FILE__, __LINE__);
        ret = -2;
        outHdr.retCode = DDP_RETCODE_FAIL;
        *outBody = 0xFF;
        if (outHdr.ipVer == IPV4_FLAG) {
            outMsgLen = HDR_END_V4_OFFSET + 3;
        } else {
            outMsgLen = HDR_END_V6_OFFSET + 3;
        }
        goto process_get_multi_ssid_over_5g;
    }
    /* 2.4GHz */
    if (band == DDP_WL_BAND_24GHZ || band == DDP_WL_BAND_ALL) {
        memset(table, 0, sizeof(struct ssid_entry) * SSID_NUMBER);
        ret = ddp_platform_get_multi_ssid(DDP_WL_BAND_24GHZ, table);
        if (ret == DDP_ALL_FIELD_SUCCESS) {
            *(outBody + 2) = DDP_WL_BAND_24GHZ;
            pos = 3;
            for (idx = 0; idx < SSID_NUMBER; idx++) {
                memcpy(outBody + pos, table[idx].name, DDP_FIELD_LEN_SSID_NAME);
                pos += DDP_FIELD_LEN_SSID_NAME;
                table[idx].vid = DDP_HTONL(table[idx].vid);
                memcpy(outBody + pos, &table[idx].vid, DDP_FIELD_LEN_VLAN_ID);
                pos += DDP_FIELD_LEN_VLAN_ID;
                table[idx].security = DDP_HTONL(table[idx].security);
                memcpy(outBody + pos, &table[idx].security, DDP_FIELD_LEN_SSID_SEC_TYPE);
                pos += DDP_FIELD_LEN_SSID_SEC_TYPE;
                memcpy(outBody + pos, table[idx].password, DDP_FIELD_LEN_SSID_SEC_PASSWD);
                pos += DDP_FIELD_LEN_SSID_SEC_PASSWD;
            }
        } else {
            *outBody = 0xFF;
            *(outBody + 2) = DDP_WL_BAND_24GHZ;
            if (outHdr.ipVer == IPV4_FLAG) {
                outMsgLen = HDR_END_V4_OFFSET + 3;
            } else {
                outMsgLen = HDR_END_V6_OFFSET + 3;
            }
        }

process_get_multi_ssid_over_24g:
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
        } else if (outHdr.ipVer == IPV6_FLAG) {
            inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
            outAddr6 = (struct sockaddr_in6*)&pkt.sender;
            outAddr6->sin6_family = inAddr6->sin6_family;
            memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
            //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV6_ADDRLEN);
            outAddr6->sin6_port = htons(UDP_PORT_SERVER);
        }
        if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
            printf("output packet in hex\n");
            print_message_hex(outMsg, outMsgLen);
        }
        sendout_msg(&pkt, outMsg, outMsgLen);
    }
    /* 5GHz */
    if (band == DDP_WL_BAND_5GHZ || band == DDP_WL_BAND_ALL) {
        memset(table, 0, sizeof(struct ssid_entry) * SSID_NUMBER);
        ret = ddp_platform_get_multi_ssid(DDP_WL_BAND_5GHZ, table);
        if (ret == DDP_ALL_FIELD_SUCCESS) {
            *(outBody + 2) = DDP_WL_BAND_5GHZ;
            pos = 3;
            for (idx = 0; idx < SSID_NUMBER; idx++) {
                memcpy(outBody + pos, table[idx].name, DDP_FIELD_LEN_SSID_NAME);
                pos += DDP_FIELD_LEN_SSID_NAME;
                table[idx].vid = DDP_HTONL(table[idx].vid);
                memcpy(outBody + pos, &table[idx].vid, DDP_FIELD_LEN_VLAN_ID);
                pos += DDP_FIELD_LEN_VLAN_ID;
                table[idx].security = DDP_HTONL(table[idx].security);
                memcpy(outBody + pos, &table[idx].security, DDP_FIELD_LEN_SSID_SEC_TYPE);
                pos += DDP_FIELD_LEN_SSID_SEC_TYPE;
                memcpy(outBody + pos, table[idx].password, DDP_FIELD_LEN_SSID_SEC_PASSWD);
                pos += DDP_FIELD_LEN_SSID_SEC_PASSWD;
            }
        } else {
            *outBody = 0xFF;
            *(outBody + 2) = DDP_WL_BAND_5GHZ;
            if (outHdr.ipVer == IPV4_FLAG) {
                outMsgLen = HDR_END_V4_OFFSET + 3;
            } else {
                outMsgLen = HDR_END_V6_OFFSET + 3;
            }
        }

process_get_multi_ssid_over_5g:
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
        } else if (outHdr.ipVer == IPV6_FLAG) {
            inAddr6 = (struct sockaddr_in6*)&inMsg->sender;
            outAddr6 = (struct sockaddr_in6*)&pkt.sender;
            outAddr6->sin6_family = inAddr6->sin6_family;
            memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
            //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV6_ADDRLEN);
            outAddr6->sin6_port = htons(UDP_PORT_SERVER);
        }
        if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
            printf("output packet in hex\n");
            print_message_hex(outMsg, outMsgLen);
        }
        sendout_msg(&pkt, outMsg, outMsgLen);
    }
    if (outMsg) { free(outMsg); outMsg = NULL; }
    return ret;
}
