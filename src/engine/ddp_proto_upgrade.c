
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ddp.h"
#include "ddp_platform.h"
#include "tftp_cli.h"


/* tftp_info struct for information transfer between ddp_upgrade and tftp threads */
struct tftp_info g_tftpInfo;
/* flag to indicate whether tftp thread is running or not */
INT4 g_upgradeFlag = 0;
pthread_mutex_t g_upgradeMutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_t g_upgradeThreadId = 0;


/* ddp_upgrade_get_flag
 *   function to get the state of upgrade flag
 *
 *   return : 0 -> no tftp thread, 1 -> tftp is running
 */
INT4
ddp_upgrade_get_flag
(
    void
)
{
    INT4 tmp = 0;
    pthread_mutex_lock(&g_upgradeMutex);
    tmp = g_upgradeFlag;
    pthread_mutex_unlock(&g_upgradeMutex);
    return tmp;
}

/* ddp_upgrade_set_flag
 *   function to set upgrade flag
 *   Only 1 tftp thread is allowed, this flag is designed to do mutex
 *
 *   flag : new state of upgrade flag
 *
 *   return : 0 -> success, others -> fail
 */
INT4
ddp_upgrade_set_flag
(
    INT4 flag
)
{
    INT4 tmp = 1;

    pthread_mutex_lock(&g_upgradeMutex);
    /* stop */
    if (g_upgradeFlag == 1 && flag == 0) {
        /* only tftp thread can set flag back to 0 */
        //if (pthread_self() == g_upgradeThreadId) {
            g_upgradeFlag= 0;
            tmp = 0;
        //}
    }
    /* start */
    else if (g_upgradeFlag == 0 && flag == 1) {
        g_upgradeFlag = 1;
        tmp = 0;
    }
    pthread_mutex_unlock(&g_upgradeMutex);
    return tmp;
}

/* extract_upgrade_info
 *   retrieve values from raw data
 *
 *   info : ddp_upgrade_info struct to be fillin
 *   data : raw data
 *
 *   return : 0 -> success, others -> error
 */
INT4
extract_upgrade_info
(
    struct ddp_upgrade_info* info,
    UINT1* data
)
{
    INT4 pos = 0;

    if (info == NULL || data == NULL) { return -1; }

    if (*data == DDP_UPGRADE_CONFIG) { info->subServiceType = DDP_UPGRADE_CONFIG; }
    else if (*data == DDP_UPGRADE_QUERY) { info->subServiceType = DDP_UPGRADE_QUERY; }
    else { return -2; }

    if (info->subServiceType == DDP_UPGRADE_QUERY) { return 0; }

    pos += DDP_FIELD_LEN_SUB_SERVICE_TYPE;
    info->serverType = *(data + pos);
    pos += DDP_FIELD_LEN_SERVER_TYPE;
    info->ipType = *(data + pos);
    pos += DDP_FIELD_LEN_SERVER_IP_ADDR_TYPE;
    if (info->ipType == DDP_UPGRADE_IPV4_ADDR) {
        memcpy(info->ipAddr.ipv4Addr, data + pos, IPV4_ADDRLEN);
        pos += IPV4_ADDRLEN;
    } else if (info->ipType == DDP_UPGRADE_IPV6_ADDR) {
        memcpy(info->ipAddr.ipv6Addr, data + pos, IPV6_ADDRLEN);
        pos += IPV6_ADDRLEN;
    }
    memcpy(info->url, data + pos, DDP_FIELD_LEN_UPGRADE_FILE_URL);
    if (g_debugFlag & DDP_DEBUG_PRINT_IN_MSG_HDR) {
        print_upgrade_info(info);
    }
    return 0;
}

/* ddp_proto_upgrade_fw (100)
 *   function to process fw upgrade request
 *
 *   ifs : interface
 *   inHdr : input header generated from input message
 *   inMsg : input message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_upgrade_fw
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = DDP_ALL_FIELD_SUCCESS;
    UINT1* inBody = NULL;
    struct ddp_header outHdr;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    struct ddp_message pkt;
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    INT4 len = 0;
    INT4 pos = 0;
    struct ddp_upgrade_info upgradeInfo;
    struct tftp_info tftpInfo;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }

    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V1;
    outHdr.bodyLen = DDP_REPLY_LEN_FW_UPGRADE;
    if (inHdr->ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = IPV4_ADDRLEN;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = IPV6_ADDRLEN;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("Fail to allocate output message\n");
        return -3;
    }
    memset(outMsg, 0, outMsgLen);
    if (outHdr.ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    /* get field value */
    memset(&tftpInfo, 0, sizeof(tftpInfo));
    memset(&upgradeInfo, 0, sizeof(struct ddp_upgrade_info));
    if (inBody) {
        pos = 0;
        /* retrieve username and password from msg body and verify them */
        if (ddp_proto_check_op_authen(DDP_OP_FW_UPGRADE) == DDP_OP_AUTHEN_ON) {
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
                goto process_fw_upgrade_over;
            }
            pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
        }
        extract_upgrade_info(&upgradeInfo, inBody + pos);
    }
    else {
        ret = -4;
        outHdr.retCode = DDP_RETCODE_FAIL;
        goto process_fw_upgrade_over;
    }
    /* config */
    if (upgradeInfo.subServiceType == DDP_UPGRADE_CONFIG) {
        pos = 0; *outBody = DDP_UPGRADE_CONFIG;
        pos += DDP_FIELD_LEN_SUB_SERVICE_TYPE;
        if (inHdr->ipVer == IPV4_FLAG) {
            if (inHdr->bodyLen != DDP_REQ_LEN_FW_UPGRADE_IPV4) {
                ret = -5;
                outHdr.retCode = DDP_RETCODE_FAIL;
                *(outBody + pos) = DDP_UPGRADE_FAIL;
                pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                strncpy((INT1*)outBody + pos, "Req body len is not correct", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                goto process_fw_upgrade_over;
            }
        } else if (inHdr->ipVer == IPV6_FLAG) {
            if (inHdr->bodyLen != DDP_REQ_LEN_FW_UPGRADE_IPV6) {
                ret = -5;
                outHdr.retCode = DDP_RETCODE_FAIL;
                *(outBody + pos) = DDP_UPGRADE_FAIL;
                pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                strncpy((INT1*)outBody + pos, "Req body len is not correct", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                goto process_fw_upgrade_over;
            }
        } else {
            ret = -5;
            outHdr.retCode = DDP_RETCODE_FAIL;
            *(outBody + pos) = DDP_UPGRADE_FAIL;
            pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
            strncpy((INT1*)outBody + pos, "IP address family is not supported", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
            goto process_fw_upgrade_over;
        }
        /* set tftp info */
        tftpInfo.action = DDP_TFTP_GET;
        tftpInfo.mode = g_tftpMode_bin;
        strcpy(tftpInfo.fremote, (INT1*)upgradeInfo.url);
        len = DDP_FIELD_LEN_FW_LOCAL_PATH;
        ddp_platform_get_field(ifs, DDP_FIELD_FW_LOCAL_PATH, (UINT1*)tftpInfo.flocal, &len);
        if (upgradeInfo.ipType == DDP_UPGRADE_IPV4_ADDR) {
            inAddr = (struct sockaddr_in*)&tftpInfo.serv;
            inAddr->sin_family = AF_INET;
            memcpy(&inAddr->sin_addr, upgradeInfo.ipAddr.ipv4Addr, IPV4_ADDRLEN);
            inAddr->sin_port = htons(DDP_TFTP_PORT);
        }
        else if (upgradeInfo.ipType == DDP_UPGRADE_IPV6_ADDR) {
            inAddr6 = (struct sockaddr_in6*)&tftpInfo.serv;
            inAddr6->sin6_family = AF_INET6;
            memcpy(&inAddr6->sin6_addr, upgradeInfo.ipAddr.ipv6Addr, IPV6_ADDRLEN);
            inAddr6->sin6_port = htons(DDP_TFTP_PORT);
        }
        /* verify no tftp thread running -> start tftp */
        if (ddp_upgrade_get_flag() == 0) {
            if (ddp_upgrade_set_flag(1) == 0) {
                memset(&g_tftpInfo, 0, sizeof(g_tftpInfo));
                memcpy(&g_tftpInfo, &tftpInfo, sizeof(tftpInfo));
                if (tftp_cli(&g_tftpInfo) != 0) {
                    ret = -8;
                    outHdr.retCode = DDP_RETCODE_FAIL;
                    *(outBody + pos) = DDP_UPGRADE_FAIL;
                    pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                    strncpy((INT1*)outBody + pos, g_tftpInfo.statusStr, DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                    goto process_fw_upgrade_over;
                }
            }
            else {
                ret = -7;
                outHdr.retCode = DDP_RETCODE_FAIL;
                *(outBody + pos) = DDP_UPGRADE_FAIL;
                pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                strncpy((INT1*)outBody + pos, "Tftp is running already", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                goto process_fw_upgrade_over;
            }
        }
    }
    /* query */
    else if (upgradeInfo.subServiceType == DDP_UPGRADE_QUERY) {
        pos = 0;
        *outBody = DDP_UPGRADE_QUERY;
        pos += DDP_FIELD_LEN_SUB_SERVICE_TYPE;
        /* if tftp thread is running */
        if (ddp_upgrade_get_flag() == 1) {
            *(outBody + pos) = g_tftpInfo.status;
            pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
            if (g_tftpInfo.status == DDP_UPGRADE_FAIL) {
                if (strlen(g_tftpInfo.statusStr)) {
                    strncpy((INT1*)outBody + pos, g_tftpInfo.statusStr, DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                }
            }
        }
        else {
            if (g_tftpInfo.status == DDP_UPGRADE_FAIL) {
                *(outBody + pos) = g_tftpInfo.status;
                pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                if (strlen(g_tftpInfo.statusStr)) {
                    strncpy((INT1*)outBody + pos, g_tftpInfo.statusStr, DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                }
            } else {
                *(outBody + pos) = g_tftpInfo.status;
            }
        }
    }

process_fw_upgrade_over:
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

/* ddp_proto_upgrade_cfg_backup (101)
 *   function to process configuration backup request
 *
 *   ifs : interface
 *   inHdr : input header generated from input message
 *   inMsg : input message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_upgrade_cfg_backup
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = DDP_ALL_FIELD_SUCCESS;
    UINT1* inBody = NULL;
    struct ddp_header outHdr;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    struct ddp_message pkt;
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    INT4 len = 0;
    INT4 pos = 0;
    struct ddp_upgrade_info upgradeInfo;
    struct tftp_info tftpInfo;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }

    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V1;
    outHdr.bodyLen = DDP_REPLY_LEN_CFG_BACKUP;
    if (inHdr->ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = IPV4_ADDRLEN;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = IPV6_ADDRLEN;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("Fail to allocate output message\n");
        return -3;
    }
    memset(outMsg, 0, outMsgLen);
    if (outHdr.ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    /* get field value */
    memset(&tftpInfo, 0, sizeof(tftpInfo));
    memset(&upgradeInfo, 0, sizeof(struct ddp_upgrade_info));
    if (inBody) {
        pos = 0;
        /* retrieve username and password from msg body and verify them */
        if (ddp_proto_check_op_authen(DDP_OP_CFG_BACKUP) == DDP_OP_AUTHEN_ON) {
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
                goto process_cfg_backup_over;
            }
            pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
        }
        extract_upgrade_info(&upgradeInfo, inBody + pos);
    }
    else {
        ret = -4;
        outHdr.retCode = DDP_RETCODE_FAIL;
        goto process_cfg_backup_over;
    }
    /* config */
    if (upgradeInfo.subServiceType == DDP_UPGRADE_CONFIG) {
        pos = 0; *outBody = DDP_UPGRADE_CONFIG;
        pos += DDP_FIELD_LEN_SUB_SERVICE_TYPE;
        if (inHdr->ipVer == IPV4_FLAG) {
            if (inHdr->bodyLen != DDP_REQ_LEN_CFG_BACKUP_IPV4) {
                ret = -5;
                outHdr.retCode = DDP_RETCODE_FAIL;
                *(outBody + pos) = DDP_UPGRADE_FAIL;
                pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                strncpy((INT1*)outBody + pos, "Req body len is not correct", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                goto process_cfg_backup_over;
            }
        } else if (inHdr->ipVer == IPV6_FLAG) {
            if (inHdr->bodyLen != DDP_REQ_LEN_CFG_BACKUP_IPV6) {
                ret = -5;
                outHdr.retCode = DDP_RETCODE_FAIL;
                *(outBody + pos) = DDP_UPGRADE_FAIL;
                pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                strncpy((INT1*)outBody + pos, "Req body len is not correct", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                goto process_cfg_backup_over;
            }
        } else {
            ret = -5;
            outHdr.retCode = DDP_RETCODE_FAIL;
            *(outBody + pos) = DDP_UPGRADE_FAIL;
            pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
            strncpy((INT1*)outBody + pos, "IP address family is not supported", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
            goto process_cfg_backup_over;
        }
        /* set tftp info */
        tftpInfo.action = DDP_TFTP_PUT;
        tftpInfo.mode = g_tftpMode_bin;
        strcpy(tftpInfo.fremote, (INT1*)upgradeInfo.url);
        len = DDP_FIELD_LEN_CFG_BACKUP_LOCAL_PATH;
        ddp_platform_get_field(ifs, DDP_FIELD_CFG_BACKUP_LOCAL_PATH, (UINT1*)tftpInfo.flocal, &len);
        if (upgradeInfo.ipType == DDP_UPGRADE_IPV4_ADDR) {
            inAddr = (struct sockaddr_in*)&tftpInfo.serv;
            inAddr->sin_family = AF_INET;
            memcpy(&inAddr->sin_addr, upgradeInfo.ipAddr.ipv4Addr, IPV4_ADDRLEN);
            inAddr->sin_port = htons(DDP_TFTP_PORT);
        }
        else if (upgradeInfo.ipType == DDP_UPGRADE_IPV6_ADDR) {
            inAddr6 = (struct sockaddr_in6*)&tftpInfo.serv;
            inAddr6->sin6_family = AF_INET6;
            memcpy(&inAddr6->sin6_addr, upgradeInfo.ipAddr.ipv6Addr, IPV6_ADDRLEN);
            inAddr6->sin6_port = htons(DDP_TFTP_PORT);
        }
        /* verify no tftp thread running -> start tftp */
        if (ddp_upgrade_get_flag() == 0) {
            if (ddp_upgrade_set_flag(1) == 0) {
                memset(&g_tftpInfo, 0, sizeof(g_tftpInfo));
                memcpy(&g_tftpInfo, &tftpInfo, sizeof(tftpInfo));
                if (tftp_cli(&g_tftpInfo) != 0) {
                    ret = -8;
                    outHdr.retCode = DDP_RETCODE_FAIL;
                    *(outBody + pos) = DDP_UPGRADE_FAIL;
                    pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                    strncpy((INT1*)outBody + pos, g_tftpInfo.statusStr, DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                    goto process_cfg_backup_over;
                }
            }
            else {
                ret = -7;
                outHdr.retCode = DDP_RETCODE_FAIL;
                *(outBody + pos) = DDP_UPGRADE_FAIL;
                pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                strncpy((INT1*)outBody + pos, "Tftp is running already", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                goto process_cfg_backup_over;
            }
        }
    }
    /* query */
    else if (upgradeInfo.subServiceType == DDP_UPGRADE_QUERY) {
        pos = 0;
        *outBody = DDP_UPGRADE_QUERY;
        pos += DDP_FIELD_LEN_SUB_SERVICE_TYPE;
        *(outBody + pos) = g_tftpInfo.status;
        pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
        if (g_tftpInfo.status == DDP_UPGRADE_FAIL) {
            if (strlen(g_tftpInfo.statusStr)) {
                strncpy((INT1*)outBody + pos, g_tftpInfo.statusStr, DDP_FIELD_LEN_UPGRADE_FAIL_STR);
            }
        }
    }

process_cfg_backup_over:
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

/* ddp_proto_upgrade_cfg_restore (102)
 *   function to process configuration restore / upgrade
 *
 *   ifs : interface
 *   inHdr : input header generated from input message
 *   inMsg : input message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_proto_upgrade_cfg_restore
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = DDP_ALL_FIELD_SUCCESS;
    UINT1* inBody = NULL;
    struct ddp_header outHdr;
    UINT1* outMsg = NULL;
    UINT1* outBody = NULL;
    INT4 outMsgLen = 0;
    struct ddp_message pkt;
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    INT4 len = 0;
    INT4 pos = 0;
    struct ddp_upgrade_info upgradeInfo;
    struct tftp_info tftpInfo;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];

    if (ifs == NULL || inHdr == NULL || inMsg == NULL) { return -1; }

    memset(&outHdr, 0, sizeof(outHdr));
    outHdr.ipVer = inHdr->ipVer;
    outHdr.seq = inHdr->seq;
    outHdr.opcode = inHdr->opcode;
    memcpy(outHdr.macAddr, ifs->macAddr, MAC_ADDRLEN);
    outHdr.protoVer = DDP_PROTO_V1;
    outHdr.bodyLen = DDP_REPLY_LEN_CFG_RESTORE;
    if (inHdr->ipVer == IPV4_FLAG) {
        outHdr.identifier = IPV4_REPLY;
        len = IPV4_ADDRLEN;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IP_ADDR, outHdr.ipAddr.ipv4Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V4_OFFSET; }
        outMsgLen = HDR_END_V4_OFFSET + outHdr.bodyLen;
    } else {
        outHdr.identifier = IPV6_REPLY;
        len = IPV6_ADDRLEN;
        ddp_platform_get_field(ifs, DDP_FIELD_DEVICE_IPV6_ADDR, outHdr.ipAddr.ipv6Addr, &len);
        if (inHdr->bodyLen > 0) { inBody = inMsg->payload + HDR_END_V6_OFFSET; }
        outMsgLen = HDR_END_V6_OFFSET + outHdr.bodyLen;
    }
    /* allocate output message */
    outMsg = (UINT1*)malloc(outMsgLen);
    if (outMsg == NULL) {
        DDP_DEBUG("Fail to allocate output message\n");
        return -3;
    }
    memset(outMsg, 0, outMsgLen);
    if (outHdr.ipVer == IPV4_FLAG) {
        outBody = outMsg + HDR_END_V4_OFFSET;
    } else {
        outBody = outMsg + HDR_END_V6_OFFSET;
    }
    /* get field value */
    memset(&tftpInfo, 0, sizeof(tftpInfo));
    memset(&upgradeInfo, 0, sizeof(struct ddp_upgrade_info));
    if (inBody) {
        pos = 0;
        /* retrieve username and password from msg body and verify them */
        if (ddp_proto_check_op_authen(DDP_OP_CFG_RESTORE) == DDP_OP_AUTHEN_ON) {
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
                goto process_cfg_restore_over;
            }
            pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
        }
        extract_upgrade_info(&upgradeInfo, inBody + pos);
    }
    else {
        ret = -4;
        outHdr.retCode = DDP_RETCODE_FAIL;
        goto process_cfg_restore_over;
    }
    /* config */
    if (upgradeInfo.subServiceType == DDP_UPGRADE_CONFIG) {
        pos = 0; *outBody = DDP_UPGRADE_CONFIG;
        pos += DDP_FIELD_LEN_SUB_SERVICE_TYPE;
        if (inHdr->ipVer == IPV4_FLAG) {
            if (inHdr->bodyLen != DDP_REQ_LEN_CFG_RESTORE_IPV4) {
                ret = -5;
                outHdr.retCode = DDP_RETCODE_FAIL;
                *(outBody + pos) = DDP_UPGRADE_FAIL;
                pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                strncpy((INT1*)outBody + pos, "Req body len is not correct", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                goto process_cfg_restore_over;
            }
        } else if (inHdr->ipVer == IPV6_FLAG) {
            if (inHdr->bodyLen != DDP_REQ_LEN_CFG_RESTORE_IPV6) {
                ret = -5;
                outHdr.retCode = DDP_RETCODE_FAIL;
                *(outBody + pos) = DDP_UPGRADE_FAIL;
                pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                strncpy((INT1*)outBody + pos, "Req body len is not correct", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                goto process_cfg_restore_over;
            }
        } else {
            ret = -5;
            outHdr.retCode = DDP_RETCODE_FAIL;
            *(outBody + pos) = DDP_UPGRADE_FAIL;
            pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
            strncpy((INT1*)outBody + pos, "IP address family is not supported", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
            goto process_cfg_restore_over;
        }
        /* set tftp info */
        tftpInfo.action = DDP_TFTP_GET;
        tftpInfo.mode = g_tftpMode_bin;
        strcpy(tftpInfo.fremote, (INT1*)upgradeInfo.url);
        len = DDP_FIELD_LEN_CFG_RESTORE_LOCAL_PATH;
        ddp_platform_get_field(ifs, DDP_FIELD_CFG_RESTORE_LOCAL_PATH, (UINT1*)tftpInfo.flocal, &len);
        if (upgradeInfo.ipType == DDP_UPGRADE_IPV4_ADDR) {
            inAddr = (struct sockaddr_in*)&tftpInfo.serv;
            inAddr->sin_family = AF_INET;
            memcpy(&inAddr->sin_addr, upgradeInfo.ipAddr.ipv4Addr, IPV4_ADDRLEN);
            inAddr->sin_port = htons(DDP_TFTP_PORT);
        }
        else if (upgradeInfo.ipType == DDP_UPGRADE_IPV6_ADDR) {
            inAddr6 = (struct sockaddr_in6*)&tftpInfo.serv;
            inAddr6->sin6_family = AF_INET6;
            memcpy(&inAddr6->sin6_addr, upgradeInfo.ipAddr.ipv6Addr, IPV6_ADDRLEN);
            inAddr6->sin6_port = htons(DDP_TFTP_PORT);
        }
        /* verify no tftp thread running -> start tftp */
        if (ddp_upgrade_get_flag() == 0) {
            if (ddp_upgrade_set_flag(1) == 0) {
                memset(&g_tftpInfo, 0, sizeof(g_tftpInfo));
                memcpy(&g_tftpInfo, &tftpInfo, sizeof(tftpInfo));
                if (tftp_cli(&g_tftpInfo) != 0) {
                    ret = -8;
                    outHdr.retCode = DDP_RETCODE_FAIL;
                    *(outBody + pos) = DDP_UPGRADE_FAIL;
                    pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                    strncpy((INT1*)outBody + pos, g_tftpInfo.statusStr, DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                    goto process_cfg_restore_over;
                }
            }
            else {
                ret = -7;
                outHdr.retCode = DDP_RETCODE_FAIL;
                *(outBody + pos) = DDP_UPGRADE_FAIL;
                pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
                strncpy((INT1*)outBody + pos, "Tftp is running already", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
                goto process_cfg_restore_over;
            }
        }
    }
    /* query */
    else if (upgradeInfo.subServiceType == DDP_UPGRADE_QUERY) {
        pos = 0;
        *outBody = DDP_UPGRADE_QUERY;
        pos += DDP_FIELD_LEN_SUB_SERVICE_TYPE;
        *(outBody + pos) = g_tftpInfo.status;
        pos += DDP_FIELD_LEN_UPGRADE_REPLY_STATUS;
        if (g_tftpInfo.status == DDP_UPGRADE_FAIL) {
            if (strlen(g_tftpInfo.statusStr)) {
                strncpy((INT1*)outBody + pos, g_tftpInfo.statusStr, DDP_FIELD_LEN_UPGRADE_FAIL_STR);
            }
        }
    }

process_cfg_restore_over:
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
