
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ddp.h"
#include "ddp_platform.h"
#include "base64.h"


/* pack_exe_status
 *   pack execution status into buffer in order to be sent
 *
 *   pkt : buffer to be sent
 *   st : ddp_exe_status struct
 *
 *   return : 0 -> success, others -> error
 */
INT4
pack_exe_status
(
    UINT1*pkt,
    struct ddp_exe_status* st
)
{
    UINT4 tmp = 0;

    if (pkt == NULL || st == NULL) { return -1; }
    tmp = st->errNo; tmp = DDP_HTONL(tmp);
    memcpy(pkt, &tmp, sizeof(tmp));
    tmp = st->opStatus; tmp = DDP_HTONL(tmp);
    memcpy(pkt + sizeof(st->errNo), &tmp, sizeof(tmp));
    tmp = st->waitTimer; tmp = DDP_HTONL(tmp);
    memcpy(pkt + sizeof(st->errNo) + sizeof(st->opStatus), &tmp, sizeof(tmp));
    tmp = st->extension; tmp = DDP_HTONL(tmp);
    memcpy(pkt + sizeof(st->errNo) + sizeof(st->opStatus) + sizeof(st->waitTimer), &tmp, sizeof(tmp));

    return 0;
}

/* pack_header
 *   pack ddp_header into buffer in order to be sent
 *
 *   msg : buffer to be snet
 *   hdr : header which contains data
 *
 *   return : 0 -> success, others -> error
 */
INT4
pack_header
(
    UINT1* pkt,
    struct ddp_header* hdr
)
{
    UINT2 tmp = 0;

    if (pkt == NULL || hdr == NULL) { return -1; }

    tmp = hdr->identifier; tmp = DDP_HTONS(tmp);
    memcpy(pkt + HDR_IDENT_OFFSET, &tmp, HDR_SEQ_OFFSET - HDR_IDENT_OFFSET);
    tmp = hdr->seq; tmp = DDP_HTONS(tmp);
    memcpy(pkt + HDR_SEQ_OFFSET, &tmp, HDR_OPCODE_OFFSET - HDR_SEQ_OFFSET);
    tmp = hdr->opcode; tmp = DDP_HTONS(tmp);
    memcpy(pkt + HDR_OPCODE_OFFSET, &tmp, HDR_MAC_OFFSET - HDR_OPCODE_OFFSET);
    memcpy(pkt + HDR_MAC_OFFSET, hdr->macAddr, HDR_IP_OFFSET - HDR_MAC_OFFSET);
    if (hdr->ipVer == IPV4_FLAG) {
        memcpy(pkt + HDR_IP_OFFSET, hdr->ipAddr.ipv4Addr, HDR_RETCODE_V4_OFFSET - HDR_IP_OFFSET);
        tmp = hdr->retCode; tmp = DDP_HTONS(tmp);
        memcpy(pkt + HDR_RETCODE_V4_OFFSET, &tmp, HDR_PVER_V4_OFFSET - HDR_RETCODE_V4_OFFSET);
        tmp = hdr->protoVer; tmp = DDP_HTONS(tmp);
        memcpy(pkt + HDR_PVER_V4_OFFSET, &tmp, HDR_BDLEN_V4_OFFSET - HDR_PVER_V4_OFFSET);
        tmp = hdr->bodyLen; tmp = DDP_HTONS(tmp);
        memcpy(pkt + HDR_BDLEN_V4_OFFSET, &tmp, HDR_END_V4_OFFSET - HDR_BDLEN_V4_OFFSET);
    } else {
        memcpy(pkt + HDR_IP_OFFSET, hdr->ipAddr.ipv6Addr, HDR_RETCODE_V6_OFFSET - HDR_IP_OFFSET);
        tmp = hdr->retCode; tmp = DDP_HTONS(tmp);
        memcpy(pkt + HDR_RETCODE_V6_OFFSET, &tmp, HDR_PVER_V6_OFFSET - HDR_RETCODE_V6_OFFSET);
        tmp = hdr->protoVer; tmp = DDP_HTONS(tmp);
        memcpy(pkt + HDR_PVER_V6_OFFSET, &tmp, HDR_BDLEN_V6_OFFSET - HDR_PVER_V6_OFFSET);
        tmp = hdr->bodyLen; tmp = DDP_HTONS(tmp);
        memcpy(pkt + HDR_BDLEN_V6_OFFSET, &tmp, HDR_END_V6_OFFSET - HDR_BDLEN_V6_OFFSET);
    }
    return 0;
}

/* extract_header
 *   retrieve values from received packet
 *
 *   data : received packet
 *   dataLen : length of received packet
 *   hdr : header to be fillin
 *
 *   return : 0 -> success, others -> error
 */
INT4
extract_header
(
    UINT1* data,
    INT4 dataLen,
    struct ddp_header* hdr
)
{
    if (data == NULL) { return -1; }
    if (dataLen < HDR_END_V4_OFFSET) { return -2; }
    if (hdr == NULL) { return -3; }

    memset(hdr, 0, sizeof(struct ddp_header));

    memcpy(&hdr->identifier, data + HDR_IDENT_OFFSET, HDR_SEQ_OFFSET - HDR_IDENT_OFFSET);
    hdr->identifier = DDP_NTOHS(hdr->identifier);
    if (hdr->identifier & 0xF000) { hdr->ipVer = IPV4_FLAG; }
    else { hdr->ipVer = IPV6_FLAG; }

    memcpy(&hdr->seq, data + HDR_SEQ_OFFSET, HDR_OPCODE_OFFSET - HDR_SEQ_OFFSET);
    hdr->seq = DDP_NTOHS(hdr->seq);

    memcpy(&hdr->opcode, data + HDR_OPCODE_OFFSET, HDR_MAC_OFFSET - HDR_OPCODE_OFFSET);
    hdr->opcode = DDP_NTOHS(hdr->opcode);

    memcpy(hdr->macAddr, data + HDR_MAC_OFFSET, HDR_IP_OFFSET - HDR_MAC_OFFSET);

    if (hdr->ipVer == IPV4_FLAG) {
        memcpy(&hdr->ipAddr.ipv4Addr, data + HDR_IP_OFFSET, HDR_RETCODE_V4_OFFSET - HDR_IP_OFFSET);

        memcpy(&hdr->retCode, data + HDR_RETCODE_V4_OFFSET, HDR_PVER_V4_OFFSET - HDR_RETCODE_V4_OFFSET);
        hdr->retCode = DDP_NTOHS(hdr->retCode);

        memcpy(&hdr->protoVer, data + HDR_PVER_V4_OFFSET, HDR_BDLEN_V4_OFFSET - HDR_PVER_V4_OFFSET);
        hdr->protoVer = DDP_NTOHS(hdr->protoVer);

        memcpy(&hdr->bodyLen, data + HDR_BDLEN_V4_OFFSET, HDR_END_V4_OFFSET - HDR_BDLEN_V4_OFFSET);
        hdr->bodyLen = DDP_NTOHS(hdr->bodyLen);
    }
    else if (hdr->ipVer == IPV6_FLAG) {
        memcpy(&hdr->ipAddr.ipv6Addr, data + HDR_IP_OFFSET, HDR_RETCODE_V6_OFFSET - HDR_IP_OFFSET);

        memcpy(&hdr->retCode, data + HDR_RETCODE_V6_OFFSET, HDR_PVER_V6_OFFSET - HDR_RETCODE_V6_OFFSET);
        hdr->retCode = DDP_NTOHS(hdr->retCode);

        memcpy(&hdr->protoVer, data + HDR_PVER_V6_OFFSET, HDR_BDLEN_V6_OFFSET - HDR_PVER_V6_OFFSET);
        hdr->protoVer = DDP_NTOHS(hdr->protoVer);

        memcpy(&hdr->bodyLen, data + HDR_BDLEN_V6_OFFSET, HDR_END_V6_OFFSET - HDR_BDLEN_V6_OFFSET);
        hdr->bodyLen = DDP_NTOHS(hdr->bodyLen);
    }

    return 0;
}

/* print_unpack_header
 *   display values which contains in header
 *
 *   hdr : header to be displayed
 *
 *   return : none
 */
void
print_unpack_header
(
    struct ddp_header *hdr
)
{
    INT1 addr[60];
    INT4 idx = 0;

    if (hdr == NULL) {
        printf("No header input\n");
        return;
    }

    printf("Header content:\n");
    printf("  ident    : %04X\n", hdr->identifier);
    printf("  seq      : %d\n", hdr->seq);
    printf("  op code  : 0x%04X (%d)\n", hdr->opcode, hdr->opcode);
    printf("  mac      : %02X-%02X-%02X-%02X-%02X-%02X\n", *hdr->macAddr, *(hdr->macAddr + 1), *(hdr->macAddr + 2),
                                              *(hdr->macAddr + 3), *(hdr->macAddr + 4), *(hdr->macAddr + 5));
    printf("  ip       : ");
    if (hdr->ipVer == IPV4_FLAG) {
        printf("%d.%d.%d.%d", *hdr->ipAddr.ipv4Addr, *(hdr->ipAddr.ipv4Addr + 1),
                                *(hdr->ipAddr.ipv4Addr + 2), *(hdr->ipAddr.ipv4Addr + 3));
    }
    else if (hdr->ipVer == IPV6_FLAG) {
        for (idx = 0; idx < IPV6_ADDRLEN; idx++) {
            if (idx > 0) { printf(":"); }
            printf("%02X", *(hdr->ipAddr.ipv6Addr + idx));
        }
    }
    printf("\n");
    printf("  ret      : %04X\n", hdr->retCode);
    printf("  proto ver: %d\n", hdr->protoVer);
    printf("  body len : %d\n", hdr->bodyLen);
}

/* print_pack_header
 *   display values directly from the raw data
 *
 *   data : original packet
 *   dataLen : length of data
 *
 *   return : none
 */
void
print_pack_header
(
    UINT1* data,
    INT4 dataLen
)
{
    struct ddp_header hdr;

    if (data == NULL || dataLen < HDR_END_V4_OFFSET) {
        return;
    }

    if (extract_header(data, dataLen, &hdr) != 0) {
        printf("%s (%d) : extract header fail\n", __FILE__, __LINE__);
        return;
    }
    print_unpack_header(&hdr);
}

/* print_exe_status
 *   function to print out the content of execution status
 *
 *   st : pointer to struct ddp_exe_status
 *
 *   return : none
 */
void
print_exe_status
(
    struct ddp_exe_status* st
)
{
    if (st == NULL) { return; }

    printf("Exe status :\n");
    printf("  Error no   : %d\n", st->errNo);

    printf("  Op status  : %d\n", st->opStatus);
    printf("  Wait timer : %d msec\n", st->waitTimer);
    printf("  Extsion    : %d\n", st->extension);
}

/* print_message_hex
 *   display raw data in hex format
 *
 *   data : raw data
 *   dataLe : length of data
 *
 *   return : none
 */
void
print_message_hex
(
    UINT1* data,
    INT4 dataLen
)
{
    INT4 i = 0;
    if (data == NULL) { return; }

    for (i = 0; i < dataLen; i++) {
        printf("%02X ", *(data + i));
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

/* print_upgrade_info
 *   display information kept in ddp_upgade_info
 *
 *   info : ddp_upgrade_info struct
 *
 *   return : none
 */
void
print_upgrade_info
(
    struct ddp_upgrade_info* info
)
{
    UINT1 i = 0;
    if (info == NULL) { return; }

    printf("Upgrade info :\n");
    printf("  Sub service type : ");
    if (info->subServiceType == DDP_UPGRADE_CONFIG) {
        printf("Parameter configure\n");
        printf("  Server type : ");
        if (info->serverType == DDP_UPGRADE_TYPE_TFTP) {
            printf("TFTP\n");
        } else if (info->serverType == DDP_UPGRADE_TYPE_HTTP) {
            printf("HTTP\n");
        } else {
            printf("Not supported ( %d )\n", info->serverType);
        }
        printf("  Server ip type : ");
        if (info->ipType == DDP_UPGRADE_IPV4_ADDR) {
            printf("IPV4\n");
        } else if (info->ipType == DDP_UPGRADE_IPV6_ADDR) {
            printf("IPV6\n");
        } else {
            printf("Not supported ( %d )\n", info->ipType);
        }
        printf("  Server ip : ");
        if (info->ipType == DDP_UPGRADE_IPV4_ADDR) {
            printf("%d.%d.%d.%d", info->ipAddr.ipv4Addr[0], info->ipAddr.ipv4Addr[1], info->ipAddr.ipv4Addr[2], info->ipAddr.ipv4Addr[3]);
        } else if (info->ipType == DDP_UPGRADE_IPV6_ADDR) {
            for (i = 0; i < IPV6_ADDRLEN; i++) {
                printf("%02x", *(info->ipAddr.ipv6Addr + i));
                if (i < IPV6_ADDRLEN - 1) { printf(":"); }
            }
        } else {
        }
        printf("\n");
        printf("  URL : %s\n", info->url);
    }
    else if (info->subServiceType == DDP_UPGRADE_QUERY) {
        printf("Status query\n");
    }
}

/* create_delay_msg
 *   create a delay messags from input arguments for delay timer
 *
 *   ifs : input interface
 *   inHdr : inbound ddp header
 *   inmsg : inbound ddp message
 *
 *   return : a message for delay timer
 */
struct ddp_message*
create_delay_msg
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    UINT2 tmp = 0;
    INT4 ret = 0;
    struct ddp_message *msg = NULL;
    if (inHdr == NULL || inMsg == NULL || ifs == NULL) {
        return NULL;
    }
    
    msg = (struct ddp_message*)malloc(sizeof(struct ddp_message));
    if (msg == NULL) {
        return NULL;
    }
    memset(msg, 0, sizeof(struct ddp_message));

    if (inHdr->ipVer == IPV4_FLAG) {
        msg->size = HDR_END_V4_OFFSET + DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD + DDP_FIELD_LEN_DELAY_TIME;
        msg->payload = (UINT1*)malloc(msg->size * sizeof(UINT1));
        if (msg->payload) {
            memset(msg->payload, 0, msg->size * sizeof(UINT1));
            tmp = IPV4_REQ;
            tmp = DDP_HTONS(tmp);
            memcpy(msg->payload + HDR_IDENT_OFFSET, &tmp, sizeof(tmp));
            tmp = inHdr->seq;
            tmp = DDP_HTONS(tmp);
            memcpy(msg->payload + HDR_SEQ_OFFSET, &tmp, sizeof(tmp));
            tmp = inHdr->opcode;
            tmp = DDP_HTONS(tmp);
            memcpy(msg->payload + HDR_OPCODE_OFFSET, &tmp, sizeof(tmp));
            memcpy(msg->payload + HDR_MAC_OFFSET, MAC_ALL, MAC_ADDRLEN);
            memcpy(msg->payload + HDR_IP_OFFSET, inHdr->ipAddr.ipv4Addr, IPV4_ADDRLEN);
            tmp = inHdr->protoVer;
            tmp = DDP_HTONS(tmp);
            memcpy(msg->payload + HDR_PVER_V4_OFFSET, &tmp, sizeof(tmp));
            tmp = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD + DDP_FIELD_LEN_DELAY_TIME;
            tmp = DDP_NTOHS(tmp);
            memcpy(msg->payload + HDR_BDLEN_V4_OFFSET, &tmp, sizeof(tmp));
            memcpy(msg->payload + HDR_END_V4_OFFSET, inMsg->payload + HDR_END_V4_OFFSET, DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD);
            msg->ifindex = ifs->ifindex;
        } else { ret = -3; }
    }
    else {
        msg->size = HDR_END_V6_OFFSET + DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD + DDP_FIELD_LEN_DELAY_TIME;
        msg->payload = (UINT1*)malloc(msg->size * sizeof(UINT1));
        if (msg->payload) {
            memset(msg->payload, 0, msg->size * sizeof(UINT1));
            tmp = IPV6_REQ;
            tmp = DDP_HTONS(tmp);
            memcpy(msg->payload + HDR_IDENT_OFFSET, &tmp, sizeof(tmp));
            tmp = inHdr->seq;
            tmp = DDP_HTONS(tmp);
            memcpy(msg->payload + HDR_SEQ_OFFSET, &tmp, sizeof(tmp));
            tmp = inHdr->opcode;
            tmp = DDP_HTONS(tmp);
            memcpy(msg->payload + HDR_OPCODE_OFFSET, &tmp, sizeof(tmp));
            memcpy(msg->payload + HDR_MAC_OFFSET, MAC_ALL, MAC_ADDRLEN);
            memcpy(msg->payload + HDR_IP_OFFSET, inHdr->ipAddr.ipv6Addr, IPV6_ADDRLEN);
            tmp = inHdr->protoVer;
            tmp = DDP_HTONS(tmp);
            memcpy(msg->payload + HDR_PVER_V6_OFFSET, &tmp, sizeof(tmp));
            tmp = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD + DDP_FIELD_LEN_DELAY_TIME;
            tmp = DDP_NTOHS(tmp);
            memcpy(msg->payload + HDR_BDLEN_V6_OFFSET, &tmp, sizeof(tmp));
            memcpy(msg->payload + HDR_END_V6_OFFSET, inMsg->payload + HDR_END_V6_OFFSET, DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD);
            msg->ifindex = ifs->ifindex;
        } else { ret = -3; }
    }
    if (ret != 0) {
        if (msg) {
            if (msg->payload) {
                free(msg->payload); msg->payload = NULL;
            }
            free(msg); msg = NULL;
        }
    }
    return msg;
}

/* string_decode
 *   decode input string according to the specified decode algorithm
 *
 *   inStr : input string
 *   outBuf : output buffer
 *   outBufLen : size of output buffer
 *   decMethod : decode algorithm
 *
 *   return : 0 -> success, others -> error
 */
INT4
string_decode
(
    INT1* inStr,
    INT1* outBuf,
    INT4  outBufLen,
    INT4  decMethod
)
{
    INT4 res = 0;
    if (inStr == NULL || outBuf == NULL) { return -1; }

    switch (decMethod) {
        case DDP_ENC_BASE64:
            res = base64_decode(inStr, outBuf, outBufLen);
            break;
        default:
            res = -2;
            printf("%s (%d) : The decode method is unavailable\n", __FILE__, __LINE__);
            break;
    }

    return res;
}

/* string_encode
 *   encode input string according to the specified encode algorithm
 *
 *   inStr : input string
 *   outBuf : output buffer
 *   outBufLen : size of output buffer
 *   encMethod : encode algorithm
 *
 *   return : 0 -> success, others -> error
 */
INT4
string_encode
(
    INT1* inStr,
    INT1* outBuf,
    INT4  outBufLen,
    INT4  encMethod
)
{
    INT4 res = 0;
    INT4 tmp = 0;
    if (inStr == NULL || outBuf == NULL) { return -1; }

    switch (encMethod) {
        case DDP_ENC_BASE64:
            tmp = base64_calculate_encrypted_length(inStr);
            if (tmp > outBufLen) {
                res = -3;
            } else {
                res = base64_encode(inStr, outBuf, outBufLen);
            }
            break;
        default:
            res = -2;
            printf("%s (%d) : The encode method is unavailable\n", __FILE__, __LINE__);
            break;
    }

    return res;
}

/* snedout_msg
 *   subroutine to send pkt
 *
 *   pkt : input packet which has information about receiver address
 *   outPkt : packet to be sent
 *   outPktLen : length of output packet
 *
 *   return : 0 -> success, others -> error
 */
INT4
sendout_msg
(
    struct ddp_message* pkt,
    UINT1* outPkt,
    INT4 outPktLen
)
{
    INT4 ret = 0;
    struct msghdr mh;
    struct cmsghdr* cmsg = NULL;
    UINT1 buf[256];
    struct iovec iov;
    struct in_pktinfo* ptr = NULL;
    struct sockaddr_in* t = NULL;
    struct in6_pktinfo* ptr6 = NULL;
    struct sockaddr_in6* t6 = NULL;
    INT4 opt = 1;
    INT4 outSocket = 0;
    struct ipv6_mreq mreq;
    //INT4 idx = 0;
    //struct ddp_interface* ifs = NULL;

    if (pkt == NULL || outPkt == NULL || outPktLen <= 0) { return -1; }

    if (pkt->sender.ss_family == AF_INET) {
        t = (struct sockaddr_in*)&pkt->sender;
        //t->sin_port = htons(UDP_PORT_SERVER);
        memset(buf, 0, sizeof(buf));
        if (inet_ntop(t->sin_family, (void*)&t->sin_addr, (INT1*)buf, sizeof(buf))) {
            DDP_DEBUG("SENDTO %s %d, through if (index %d)\n", (INT1*)buf, ntohs(t->sin_port), pkt->ifindex);
        }
    } else if (pkt->sender.ss_family == AF_INET6) {
        t6 = (struct sockaddr_in6*)&pkt->sender;
        //t6->sin6_port = htons(UDP_PORT_SERVER);
        memset(buf, 0, sizeof(buf));
        if (inet_ntop(t6->sin6_family, (void*)&t6->sin6_addr, (INT1*)buf, sizeof(buf))) {
            DDP_DEBUG("SENDTO v6 %s %d, through if (index %d)\n", (INT1*)buf, ntohs(t6->sin6_port), pkt->ifindex);
        }
    }
    memset(&mh, 0, sizeof(mh));
    memset(buf, 0, sizeof(buf));
    memset(&iov, 0, sizeof(iov));

    iov.iov_base = outPkt;
    iov.iov_len = outPktLen;

    mh.msg_iov = &iov;
    mh.msg_iovlen = 1;
    mh.msg_control = buf;
    mh.msg_controllen = sizeof(buf);
    cmsg = CMSG_FIRSTHDR(&mh);
    if (pkt->sender.ss_family == AF_INET) {
        mh.msg_name = t;
        mh.msg_namelen = sizeof(struct sockaddr_in);
        cmsg->cmsg_type = IP_PKTINFO;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
        ptr = (struct in_pktinfo*)CMSG_DATA(cmsg);
        cmsg->cmsg_level = IPPROTO_IP;
        ptr->ipi_ifindex = pkt->ifindex;
    } else if (pkt->sender.ss_family == AF_INET6) {
        mh.msg_name = t6;
        mh.msg_namelen = sizeof(struct sockaddr_in6);
        cmsg->cmsg_type = IPV6_PKTINFO;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
        ptr6 = (struct in6_pktinfo*)CMSG_DATA(cmsg);
        cmsg->cmsg_level = IPPROTO_IPV6;
        ptr6->ipi6_ifindex = pkt->ifindex;
    }
    mh.msg_controllen = cmsg->cmsg_len;

    /* sendout through another tmp socket */
    if (pkt->sender.ss_family == AF_INET) {
        outSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (outSocket <= 0) {
            DDP_DEBUG("%s (%d) : create send socket fail\n", __FILE__, __LINE__);
            ret = -2;
            goto sendout_msg_over;
        }
        if (setsockopt(outSocket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) != 0) {
            DDP_DEBUG("%s (%d) : set v4 broadcast fail\n", __FILE__, __LINE__);
            ret = -2;
            goto sendout_msg_over;
        }

//        DDP_DEBUG("LLLLLLLLLLLLLLLLLLLLLLLLLLL\n");
//        //sleep(3);
//        struct sockaddr_in cliaddr;
//        cliaddr.sin_family = AF_INET;
//        cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//        cliaddr.sin_port = htons(0xF000);
//        int u = EINVAL;
//        u = bind(outSocket,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
//        if(u)
//        	DDP_DEBUG("%s (%d) (%d) :socket bind err\n", __FILE__, __LINE__, u);

    } else if (pkt->sender.ss_family == AF_INET6) {
        outSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (outSocket <= 0) {
            DDP_DEBUG("%s (%d) : create v6 send socket fail\n", __FILE__, __LINE__);
            ret = -2;
            goto sendout_msg_over;
        }
        mreq.ipv6mr_interface = pkt->ifindex;
        if (inet_pton(AF_INET6, DDP_IPV6_MCAST, &mreq.ipv6mr_multiaddr) != 1) {
            DDP_DEBUG("%s (%d) : set IPv6 multicast address fail (error %d)\n", __FILE__, __LINE__, errno);
        }
        /*if (setsockopt(outSocket, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq)) != 0) {
            DDP_DEBUG("%s (%d) : join v6 multicast fail (error %d)\n", __FILE__, __LINE__, errno);
        }*/
        if (setsockopt(outSocket, IPPROTO_IPV6, IPV6_MULTICAST_IF, &pkt->ifindex, sizeof(pkt->ifindex)) != 0) {
            DDP_DEBUG("%s (%d) : set v6 multicast fail (error %d)\n", __FILE__, __LINE__, errno);
        }
    } else {
        DDP_DEBUG("%s (%d) : ip address family is not supported\n", __FILE__, __LINE__);
        ret = -2;
        goto sendout_msg_over;
    }

    if (outPktLen > 0) {
        ret = sendmsg(outSocket, &mh, 0);
        //ret = sendmsg(g_iSockfd, &mh, 0);
        //ret = sendto(g_iSockfd, outPkt, outPktLen, 0, (struct sockaddr*)t, sizeof(struct sockaddr));
    }
    if (ret < 0) {
        DDP_DEBUG("%s (%d) : sendmsg err (%d) \n", __FILE__, __LINE__, errno);
    }

sendout_msg_over:
    if (outSocket > 0) {
        if (pkt->sender.ss_family == AF_INET6) {
            /*if (setsockopt(outSocket, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) != 0) {
                DDP_DEBUG("%s (%d) : leave v6 group fail (error %d)\n", __FILE__, __LINE__, errno);
            }*/
        }
        close(outSocket); outSocket = 0;
    }

    return ret;
}
