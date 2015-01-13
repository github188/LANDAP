
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>


#include "ddp.h"
#include "ddp_platform.h"

UINT4 proxy_ifindex = 0;
INT1 g_proxyType = 0;
UINT1 g_proxyIP[16];
INT4 g_proxyPort = 0;

/* ddp_srv_process_config
 *   function to parse config file to get the position of proxy.
 *
 * path : path of config file
 *
 * return : 0 -> success, others -> error
 */
INT4
ddp_srv_process_config
(
    INT1* path
)
{
    INT4 ret = 0;
    FILE* fd = NULL;
    INT1 buf[100];
    INT4 readin = 0;
    INT4 i = 0;
    INT4 len = 0;
    INT4 token_head = -1;
    INT4 sig_separator = 0;
    INT1 key[20];
    INT1 separator = '=';
    INT1 value[40];
    
    fd = fopen(path, "r");
    if (fd == NULL) {
        DDP_DEBUG("%s (%d) : open config file fail\n", __FILE__, __LINE__);
        return -1;
    }
    g_proxyPort = 0;
    memset(g_proxyIP, 0, sizeof(g_proxyIP));
    while (1) {
        memset(buf, 0, sizeof(buf));
        readin = fscanf(fd, "%[^\n]\n", buf);
        //DDP_DEBUG("readin : %s\n", buf);
        if (readin < 0) { goto read_over; }

        memset(key, 0, sizeof(key));
        memset(value, 0, sizeof(value));
        token_head = -1;
        sig_separator = 0;
        len = strlen(buf);
        for (i = 0; i < len; i++) {
            if (buf[i] == '#') { goto read_next_line; }
            if (buf[i] == ' ') { token_head = -1; continue; }
            if (buf[i] == separator) { sig_separator++; }

            if (buf[i] != ' ') {
                if (token_head == -1) { token_head = i; }
                if (sig_separator == 0) {
                    key[i - token_head] = buf[i];
                }
                else if (sig_separator == 1) {
                    value[i - token_head] = buf[i];
                }
            }
        }

read_next_line:
        if (strlen(key)) { printf("%s = %s\n", key, value); }
        if (strcmp(key, "ProxyIP") == 0) {
            if (inet_pton(AF_INET, value, g_proxyIP) != 1) {
                DDP_DEBUG("%s (%d) : get proxy ip fail\n", __FILE__, __LINE__);
                ret = -2;
            }
            g_proxyType = 4;
            //for (i = 0; i < IPV4_ADDRLEN; i++) { printf("%02X ", g_proxyIP[i]); }
            //printf("\n");
        }
        else if (strcmp(key, "ProxyPort") == 0) {
            g_proxyPort = atoi(value);
            if (g_proxyPort == 0) {
                DDP_DEBUG("%s (%d) : get proxy port fail\n", __FILE__, __LINE__);
                ret = -3;
            }
            //printf("Port %d\n", g_proxyPort);
        }
        else if (strcmp(key, "ProxyIPv6") == 0) {
            if (inet_pton(AF_INET6, value, g_proxyIP) != 1) {
                DDP_DEBUG("%s (%d) : get proxy ipv6 fail\n", __FILE__, __LINE__);
                ret = -4;
            }
            g_proxyType = 6;
            //for (i = 0; i < IPV6_ADDRLEN; i++) { printf("%02X ", g_proxyIP[i]); }
            //printf("\n");
        }
    } /* while loop */

read_over:
    if (fd) { fclose(fd); fd = NULL; }
    return ret;
}

INT4
ddp_srv_send_req
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

    if (pkt == NULL || outPkt == NULL || outPktLen <= 0) { return -1; }

    if (pkt->sender.ss_family == AF_INET) {
        t = (struct sockaddr_in*)&pkt->sender;
        memset(buf, 0, sizeof(buf));
        if (inet_ntop(t->sin_family, (void*)&t->sin_addr, (INT1*)buf, sizeof(buf))) {
            DDP_DEBUG("SENDTO %s %d, through if (index %d)\n", (INT1*)buf, ntohs(t->sin_port), pkt->ifindex);
        }
    } else if (pkt->sender.ss_family == AF_INET6) {
        t6 = (struct sockaddr_in6*)&pkt->sender;
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
    if (outPktLen > 0) {
        ret = sendmsg(g_srvSockfd, &mh, 0);
    }
    if (ret < 0) {
        DDP_DEBUG("%s (%d) : sendmsg err (%d) \n", __FILE__, __LINE__, errno);
    }

    return ret;
}

INT4
ddp_srv_send_reply
(
    struct ddp_message* pkt,
    UINT1* buf,
    INT4 bufLen
)
{
    INT4 retVal = 0;
    INT4 outSocket = 0;
    INT4 addrlen = 0;

    if (pkt == NULL || buf == NULL || bufLen == 0) { return -1; }

    if (pkt->sender.ss_family == AF_INET) {
        outSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (outSocket <= 0) {
            DDP_DEBUG("%s (%d) : create send socket fail\n", __FILE__, __LINE__);
            retVal = -2;
            goto ddp_srv_send_reply_over;
        }
        addrlen = sizeof(struct sockaddr_in);
    }
    else if (pkt->sender.ss_family == AF_INET6) {
        outSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (outSocket <= 0) {
            DDP_DEBUG("%s (%d) : create v6 send socket fail\n", __FILE__, __LINE__);
            retVal = -2;
            goto ddp_srv_send_reply_over;
        }
        addrlen = sizeof(struct sockaddr_in6);
    }
    else {
        DDP_DEBUG("%s (%d) : ip address family is not supported\n", __FILE__, __LINE__);
        retVal = -2;
        goto ddp_srv_send_reply_over;
    }
    retVal = sendto(outSocket, buf, bufLen, 0, (struct sockaddr*)&pkt->sender, addrlen);
    
ddp_srv_send_reply_over:
    if (outSocket > 0) { close(outSocket); outSocket = 0; }

    return retVal;
}

/* ddp_srv_search_node
 *   function to search node.
 * 
 *   hdr : message header
 *
 *   return : 0 -> not found, 1 -> found
 */
INT4
ddp_srv_search_node
(
    struct ddp_header* hdr
)
{
    return 1;
}

/* ddp_srv_process_message
 *   function to process message retrieved from srv message queue.
 * 
 *   inMsg : message from queue
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_srv_process_message
(
    struct ddp_message* inMsg
)
{
    INT4 retVal = 0;
    UINT2 tmpU2 = 0;
    INT1 addr[IPV6_ADDRLEN];
    struct ddp_header inHdr;
    struct ddp_interface* ifs = NULL;
    struct ddp_message outMsg;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    INT4 tmp4 = 0;
    

    if (inMsg == NULL) { return -1; }
    /* extract header */
    extract_header(inMsg->payload, inMsg->size, &inHdr);

    /* print out header after parsing */
    if (g_debugFlag & DDP_DEBUG_PRINT_SRV_MSG_HDR) {
        printf("Header of input packet\n");
        print_unpack_header(&inHdr);
    }

    /* relay from WAN
     * Find out whether the destination node exists in LAN.
     * If not found, send it broadcast.
     * If found, send it unicast.
     * If the message requires broadcast, just broadcast it.
     */
    memset(&outMsg, 0, sizeof(outMsg));
    if (inHdr.identifier == IPV4_RELAY || inHdr.identifier == IPV6_RELAY) {
        /* find out whether destination exists in LAN */
        tmp4 = ddp_srv_search_node(&inHdr);
        if (inHdr.ipVer == IPV4_FLAG) {
            tmpU2 = IPV4_REQ;
            tmpU2 = DDP_HTONS(tmpU2);
            memcpy(inMsg->payload, (INT1*)&tmpU2, sizeof(tmpU2));
            outAddr = (struct sockaddr_in*)&outMsg.sender;
            outAddr->sin_family = AF_INET;
            if (memcmp(inHdr.macAddr, MAC_ALL, MAC_ADDRLEN) == 0 || tmp4 == 0) {
                memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
            } else {
                memcpy(&outAddr->sin_addr, inHdr.ipAddr.ipv4Addr, IPV4_ADDRLEN);
            }
            outAddr->sin_port = htons(UDP_PORT_CLIENT);
        } else {
            tmpU2 = IPV6_REQ;
            tmpU2 = DDP_HTONS(tmpU2);
            memcpy(inMsg->payload, (INT1*)&tmpU2, sizeof(tmpU2));
            outAddr6 = (struct sockaddr_in6*)&outMsg.sender;
            outAddr6->sin6_family = AF_INET6;
            if (memcmp(inHdr.macAddr, MAC_ALL, MAC_ADDRLEN) == 0 || tmp4 == 0) {
                if (inet_pton(AF_INET6, DDP_IPV6_MCAST, addr) != 1) {
                    DDP_DEBUG("%s (%d) : set ipv6 multicast address fail (error %d)\n", __FILE__, __LINE__, errno);
                }
                memcpy(&outAddr6->sin6_addr, addr, IPV6_ADDRLEN);
            } else {
                memcpy(&outAddr6->sin6_addr, inHdr.ipAddr.ipv6Addr, IPV6_ADDRLEN);
            }
            outAddr6->sin6_port = htons(UDP_PORT_CLIENT);
        }
        proxy_ifindex = inMsg->ifindex;
        
        if (g_debugFlag & DDP_DEBUG_PRINT_SRV_MSG_HEX) {
            print_message_hex(inMsg->payload, inMsg->size);
        }
        /* send through each interface */
        for (ifs = g_ifList->next; ifs != NULL; ifs = ifs->next) {
            outMsg.ifindex = ifs->ifindex;
            ddp_srv_send_req(&outMsg, inMsg->payload, inMsg->size);
        }
    }
    /* reply from nodes inside LAN */
    else if (inHdr.identifier == IPV4_REPLY || inHdr.identifier == IPV6_RELAY) {
        if (g_debugFlag & DDP_DEBUG_PRINT_SRV_MSG_HEX) {
            print_message_hex(inMsg->payload, inMsg->size);
        }
        if (inHdr.ipVer == IPV4_FLAG) {
            tmpU2 = IPV4_RELAY;
            tmpU2 = DDP_HTONS(tmpU2);
            memcpy(inMsg->payload, (INT1*)&tmpU2, sizeof(tmpU2));
            outAddr = (struct sockaddr_in*)&outMsg.sender;
            outAddr->sin_family = AF_INET;
            memcpy(&outAddr->sin_addr, g_proxyIP, IPV4_ADDRLEN);
            outAddr->sin_port = htons(g_proxyPort);
        } else {
            tmpU2 = IPV6_RELAY;
            tmpU2 = DDP_HTONS(tmpU2);
            memcpy(inMsg->payload, (INT1*)&tmpU2, sizeof(tmpU2));
            outAddr6 = (struct sockaddr_in6*)&outMsg.sender;
            outAddr6->sin6_family = AF_INET6;
            memcpy(&outAddr6->sin6_addr, g_proxyIP, IPV6_ADDRLEN);
            outAddr6->sin6_port = htons(g_proxyPort);
        }
        ddp_srv_send_reply(&outMsg, inMsg->payload, inMsg->size);
    }
    
    return retVal;
}
