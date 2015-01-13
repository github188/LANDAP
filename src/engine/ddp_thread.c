
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/select.h>
//#include <netinet/ip.h>
//#include <arpa/inet.h>

#include "ddp.h"
#include "ddp_platform.h"
#include "msg_queue.h"


/* time interval to check queue */
INT4 g_iIntervalCheckQueue = PROC_THREAD_CHECK_QUEUE_INTERVAL_DEFAULT;
/* time interval to read socket */
INT4 g_iIntervalReadSocket = RECV_THREAD_CHECK_PKT_INTERVAL_DEFAULT;


/* ddp_thread_pre_filter
 *   function to filter out messages which are not accepted or processed by device
 *
 *   pkt: received message
 *   pktLen: message length
 *   ifindex: interface index
 *
 *   return: 0 -> reject,
 *           1 -> accept and insert message into cli msg queue
 *           2 -> accept and insert message into srv msg queue
 */
INT4
ddp_thread_pre_filter
(
    UINT1* pkt,
    INT4 pktLen,
    UINT4 ifindex
)
{
    INT4 ret = 0;
    struct ddp_header hdr;
    struct ddp_interface* ifs = NULL;

    if (pkt == NULL || pktLen == 0) {
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_RECV_MSG_HEX, "Pre-Filter: NULL pkt, pkt len<%d>\n", pktLen);
        return 0;
    }
    ifs = ddp_platform_search_if_by_index(g_ifList, ifindex);
    if (ifs == NULL) {
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_RECV_MSG_HEX, "Pre-Filter: Invalid inf indx<%d>\n", ifindex);
        return 0;
    }
    memset(&hdr, 0, sizeof(hdr));
    extract_header(pkt, pktLen, &hdr);
    if (g_debugFlag & DDP_DEBUG_PRINT_RECV_MSG_HEX) {
        printf("Header of input message\n");
        print_unpack_header(&hdr);
    }
    /* TRADEOFF : utility may send packet in proto v1 */
    //if (hdr->protoVer != DDP_PROTO_V2) { return -2; }
    if (hdr.identifier == IPV4_REPORT || hdr.identifier == IPV6_REPORT) {
        /* check whether it is capable of processing general report */
        if (ddp_platform_capable_process_report(ifs) != DDP_PROCESS_REPORT_YES) {
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_RECV_MSG_HEX, "Device is not able to process report\n");
            return 0;
        }
        /* avoid report message back to itself */
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_RECV_MSG_HEX, "msg mac : %02X-%02X-%02X-%02X-%02X-%02X\n", *hdr.macAddr, *(hdr.macAddr + 1), *(hdr.macAddr + 2),
                                              *(hdr.macAddr + 3), *(hdr.macAddr + 4), *(hdr.macAddr + 5));
        ifs = g_ifList->next;
        ret = 1;
        while (ifs) {
            if (memcmp((void*)ifs->macAddr, (void*)hdr.macAddr, MAC_ADDRLEN) == 0) {
                ret = 0;
                DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_RECV_MSG_HEX, " if mac : %02X-%02X-%02X-%02X-%02X-%02X\n", *ifs->macAddr, *(ifs->macAddr + 1), *(ifs->macAddr + 2),
                                              *(ifs->macAddr + 3), *(ifs->macAddr + 4), *(ifs->macAddr + 5));
                DDP_DEBUG("Report message comes back to itself -> Drop it\n");
                break;
            }
            ifs = ifs->next;
        }
    }
    else if (hdr.identifier == IPV4_REQ || hdr.identifier == IPV6_REQ) {
        if (hdr.retCode == REQ_MODE_UNICAST) {
            DDP_DEBUG("\nReqPkt for all or specific device, check the MAC\n");
            if (ifs) {
                if (memcmp((void*)hdr.macAddr, (void*)ifs->macAddr, MAC_ADDRLEN) == 0) {
                    ret = 1;
                }
                else{
                     DDP_DEBUG("ReqTarget[%x:%x:%x:%x:%x:%x] != ReceiverMac[%x:%x:%x:%x:%x:%x]",
                         hdr.macAddr[0], hdr.macAddr[1], hdr.macAddr[2],
                         hdr.macAddr[3], hdr.macAddr[4], hdr.macAddr[5],
                         ifs->macAddr[0], ifs->macAddr[1], ifs->macAddr[2],
                         ifs->macAddr[3], ifs->macAddr[4], ifs->macAddr[5]);
                }
            }
        }
        else if (hdr.retCode == 0x0000) {
            DDP_DEBUG("\nReqPkt for all or specific device, check the MAC\n");
            if (memcmp((void*)hdr.macAddr, (void*)&MAC_ALL, MAC_ADDRLEN) == 0) {
                ret = 1;
            } else {
                if (ifs) {
                    if (memcmp((void*)hdr.macAddr, (void*)ifs->macAddr, MAC_ADDRLEN) == 0) {
                        ret = 1;
                    }
                    else{
                        DDP_DEBUG("ReqTarget[%x:%x:%x:%x:%x:%x] != ReceiverMac[%x:%x:%x:%x:%x:%x]",
                             hdr.macAddr[0], hdr.macAddr[1], hdr.macAddr[2],
                             hdr.macAddr[3], hdr.macAddr[4], hdr.macAddr[5],
                             ifs->macAddr[0], ifs->macAddr[1], ifs->macAddr[2],
                             ifs->macAddr[3], ifs->macAddr[4], ifs->macAddr[5]);
                    }
                }
            }
        }
        if (ret == 1) {
            ret = ddp_platform_op_support(ifs, hdr.opcode);
            if (ret != 1) {
                DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_RECV_MSG_HEX, "op %d is not support by device\n", hdr.opcode);
            }
        }
    }
    else if (hdr.identifier == IPV4_RELAY || hdr.identifier == IPV6_RELAY) {
        if (g_role & DDP_ROLE_SERVER) {
            ret = 2;
        }
    }

    return ret;
}

/* ddp_thread_recv_process
 *   major task of recv thread
 *
 *   strThreadName : thread name
 *
 *   return : none
 */
void
ddp_thread_recv_process
(
    INT1* strThreadName
)
{
#define CMBUFSIZE 256
    INT4 result = 0;
    INT4 iLoop = 0;
    UINT1 buf[UDP_CONTAINER_SIZE];
    UINT1* payload = NULL;
    INT4 iBytesRecv = 0;
    struct ddp_message* pkt = NULL;
    fd_set rfds;
    struct timeval tv;
    INT4 max_fd = 0;
    INT4 iSelectReturn = 0;
    UINT4 ifindex = 0;

    struct sockaddr_storage peer;
    struct sockaddr_in* in_v4 = NULL;
    struct sockaddr_in* out_v4 = NULL;
    struct sockaddr_in6* in_v6 = NULL;
    struct sockaddr_in6* out_v6 = NULL;

    struct in6_pktinfo* pi6 = NULL;
    struct cmsghdr* cmsg = NULL;
    struct in_pktinfo* pi = NULL;
    INT1 cmbuf[CMBUFSIZE];
    INT4 flag = MSG_DONTWAIT;
    struct iovec iov;
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    struct msghdr mh;
    memset(&mh, 0, sizeof(mh));
    mh.msg_name = &peer;
    mh.msg_namelen = sizeof(peer);
    mh.msg_control = cmbuf;
    mh.msg_controllen = sizeof(cmbuf);
    mh.msg_iov = &iov;
    mh.msg_iovlen = 1;

    while (1) {
        iLoop = ddp_get_loop_flag();
        if (iLoop & DDP_OVER_SIG) { break; }

        /* select */
        tv.tv_sec = g_iIntervalReadSocket;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        max_fd = 0;

        if (g_ipv4Ready == 1) {
            if ((g_role & DDP_ROLE_CLIENT) != 0) {
                FD_SET(g_iSockfd, &rfds);
            }
            if ((g_role & DDP_ROLE_SERVER) != 0) {
                FD_SET(g_srvSockfd, &rfds);
            }
            if (g_iSockfd > max_fd) { max_fd = g_iSockfd; }
            if (g_srvSockfd > max_fd) { max_fd = g_srvSockfd; }
        }
        if (g_ipv6Ready == 1) {
            if ((g_role & DDP_ROLE_CLIENT) != 0) {
                FD_SET(g_iSockfdV6, &rfds);
            }
            if ((g_role & DDP_ROLE_SERVER) != 0) {
                FD_SET(g_srvSockfdV6, &rfds);
            }
            if (g_iSockfdV6 > max_fd) { max_fd = g_iSockfdV6; }
            if (g_srvSockfdV6 > max_fd) { max_fd = g_srvSockfdV6; }
        }
        if (max_fd) { max_fd++; }
        else {
            DDP_DEBUG("RecvThread: no socket is set\n");
            continue;
        }
        iSelectReturn = select(max_fd, &rfds, NULL, NULL, &tv);
        if (iSelectReturn <= 0) {
            //DDP_DEBUG("select result %d\n", iSelectReturn);
            continue;
        }
        /* Cli IPV6 */
        if (FD_ISSET(g_iSockfdV6, &rfds)) {
            if (g_ipv6Ready != 1) { goto cli_ipv6_over; }
            /* packet comes in */
            ifindex = 0;
            mh.msg_namelen = sizeof(peer);
            mh.msg_controllen = sizeof(cmbuf);
            memset(cmbuf, 0, CMBUFSIZE);
            memset(buf, 0, UDP_CONTAINER_SIZE);
            iBytesRecv = recvmsg(g_iSockfdV6, &mh, flag);
            if (ddp_run_state(DDP_RUN_STATE_GET, 0) == DDP_RUN_STATE_HALT) { goto cli_ipv6_over; }
            if (iBytesRecv < 0) { printf("Client IPv6 recvmsg err %d\n", errno); }
            /* ddp packet smallest size is 34 bytes */
            if (iBytesRecv >= HDR_END_V6_OFFSET) {
                DDP_DEBUG("\n<<<<<<<<< Recv v6 packet (size %d bytes) >>>>>>>>>\n", iBytesRecv);
                /* get the interface where the packet comes */
                for (cmsg = CMSG_FIRSTHDR(&mh); cmsg != NULL; cmsg = CMSG_NXTHDR(&mh, cmsg)) {
                    if (cmsg->cmsg_level != IPPROTO_IPV6 || cmsg->cmsg_type != IPV6_PKTINFO) { continue; }
                    pi6 = (struct in6_pktinfo*)CMSG_DATA(cmsg);
                    ifindex = pi6->ipi6_ifindex; // require -D_GNU_SOURCE in CFLAGS
                } // for
                result = ddp_thread_pre_filter(buf, iBytesRecv, ifindex);

                if (result != 1) {
                    DDP_DEBUG("Drop v6 msg, pre-filter result:%d\n", result);
                    goto cli_ipv6_over;
                }
                /* allocate memory to store received content */
                payload = (UINT1*)malloc(iBytesRecv * sizeof(UINT1));
                if (payload == NULL) {
                    goto cli_ipv6_over;
                }
                memset(payload, 0, iBytesRecv);
                memcpy(payload, iov.iov_base, iBytesRecv);
                /* allocate packet node to be inserted into msg queue */
                pkt = (struct ddp_message*)malloc(sizeof(struct ddp_message));
                if (pkt == NULL) {
                    if (payload) { free(payload); payload = NULL; }
                    goto cli_ipv6_over;
                }
                memset(pkt, 0, sizeof(struct ddp_message));
                pkt->size = iBytesRecv;
                pkt->payload = payload;
                /* extract src addr */
                in_v6 = (struct sockaddr_in6*)&peer;
                out_v6 = (struct sockaddr_in6*)&pkt->sender;
                out_v6->sin6_family = in_v6->sin6_family;
                memcpy(&out_v6->sin6_addr, &in_v6->sin6_addr, IPV6_ADDRLEN);
                out_v6->sin6_port = in_v6->sin6_port;
                pkt->ifindex = ifindex;
                if (g_debugFlag & DDP_DEBUG_PRINT_RECV_MSG_HEX) {
                    printf("\n***************** RECV V6 MSG *****************\n");
                    printf("if index %d\n", ifindex);
                    if(pkt != NULL){
                        if(pkt->payload != NULL)
                        {
                            DDP_DEBUG("pkt size<%d>, Payload:\n", pkt->size);
                            print_message_hex(pkt->payload, pkt->size);
                        } else { DDP_DEBUG("NULL payload\n"); }
                    } else { DDP_DEBUG("NULL pkt\n"); }
                    printf("\n**********************************************\n");
                }

                /* insert msg to queue */
                if (g_mq) {
                    result = msg_queue_insert_msg(g_mq, (UINT1*)pkt);
                    if (result < 0) {
                        if (pkt) { free(pkt); pkt = NULL; }
                        DDP_DEBUG("Insert v6 msg into queue fail (error %d)\n", result);
                    }
                } else {
                    if (pkt) { free(pkt); pkt = NULL; }
                }
            }
        }
cli_ipv6_over:
        /* Cli IPV4 */
        if (FD_ISSET(g_iSockfd, &rfds)) {
            if (g_ipv4Ready != 1) { goto cli_ipv4_over; }
            /* packet comes in */
            ifindex = 0;
            mh.msg_namelen = sizeof(peer);
            mh.msg_controllen = sizeof(cmbuf);
            memset(cmbuf, 0, CMBUFSIZE);
            memset(buf, 0, UDP_CONTAINER_SIZE);
            iBytesRecv = recvmsg(g_iSockfd, &mh, flag);
            if (ddp_run_state(DDP_RUN_STATE_GET, 0) == DDP_RUN_STATE_HALT) { goto cli_ipv4_over; }
            if (iBytesRecv < 0) { printf("Client IPv4 recvmsg err %d\n", errno); }
            /* ddp packet smallest size is 22 bytes */
            if (iBytesRecv >= HDR_END_V4_OFFSET) {
                DDP_DEBUG("\n<<<<<<<<< Recv v4 packet (size %d bytes) >>>>>>>>>\n", iBytesRecv);
                /* get the interface where the packet comes */
                for (cmsg = CMSG_FIRSTHDR(&mh); cmsg != NULL; cmsg = CMSG_NXTHDR(&mh, cmsg)) {
                    if (cmsg->cmsg_level != IPPROTO_IP || cmsg->cmsg_type != IP_PKTINFO) { continue; }
                    pi = (struct in_pktinfo*)CMSG_DATA(cmsg);
                    ifindex = pi->ipi_ifindex;
                } // for
                result = ddp_thread_pre_filter(buf, iBytesRecv, ifindex);

                if (result != 1) {
                    DDP_DEBUG("Drop v4 msg, pre-filter result:%d, ifindex<%d>\n", result, ifindex);
                    goto cli_ipv4_over;
                }
                /* allocate memory to store received content */
                payload = (UINT1*)malloc(iBytesRecv * sizeof(UINT1));
                if (payload == NULL) {
                    goto cli_ipv4_over;
                }
                memset(payload, 0, iBytesRecv);
                memcpy(payload, iov.iov_base, iBytesRecv);
                /* allocate packet node to be inserted into msg queue */
                pkt = (struct ddp_message*)malloc(sizeof(struct ddp_message));
                if (pkt == NULL) {
                    if (payload) { free(payload); payload = NULL; }
                    goto cli_ipv4_over;
                }
                memset(pkt, 0, sizeof(struct ddp_message));
                pkt->size = iBytesRecv;
                pkt->payload = payload;
                /* extract src addr */
                in_v4 = (struct sockaddr_in*)&peer;
                out_v4 = (struct sockaddr_in*)&pkt->sender;
                out_v4->sin_family = in_v4->sin_family;
                memcpy(&out_v4->sin_addr, &in_v4->sin_addr, IPV4_ADDRLEN);
                out_v4->sin_port = in_v4->sin_port;
                pkt->ifindex = ifindex;
                if (g_debugFlag & DDP_DEBUG_PRINT_RECV_MSG_HEX) {
                    printf("\n***************** RECV V4 MSG *****************\n");
                    printf("if index %d\n", ifindex);
                    if (pkt != NULL){
                        if (pkt->payload != NULL)
                        {
                            DDP_DEBUG("pkt size<%d>, Payload:\n", pkt->size);
                            print_message_hex(pkt->payload, pkt->size);
                        } else { DDP_DEBUG("NULL payload\n"); }
                    } else { DDP_DEBUG("NULL pkt\n"); }
                    printf("\n**********************************************\n");
                }

                /* insert msg into queue */
                if (g_mq) {
                    result = msg_queue_insert_msg(g_mq, (UINT1*)pkt);
                    if (result < 0) {
                        if (pkt) { free(pkt); pkt = NULL; }
                        DDP_DEBUG("Insert v4 msg into queue fail (error %d)\n", result);
                    }
                } else {
                    if (pkt) { free(pkt); pkt = NULL; }
                }
            }
        }
cli_ipv4_over:
        /* Srv IPv6 */
        if (FD_ISSET(g_srvSockfdV6, &rfds)) {
            if (g_ipv6Ready != 1) { goto srv_ipv6_over; }
            /* packet comes in */
            ifindex = 0;
            mh.msg_namelen = sizeof(peer);
            mh.msg_controllen = sizeof(cmbuf);
            memset(cmbuf, 0, CMBUFSIZE);
            memset(buf, 0, UDP_CONTAINER_SIZE);
            iBytesRecv = recvmsg(g_srvSockfdV6, &mh, flag);
            if (ddp_run_state(DDP_RUN_STATE_GET, 0) == DDP_RUN_STATE_HALT) { goto srv_ipv6_over; }
            if (iBytesRecv < 0) { printf("Server IPv6 recvmsg err %d\n", errno); }
            /* ddp packet smallest size is 34 bytes */
            if (iBytesRecv >= HDR_END_V6_OFFSET) {
                DDP_DEBUG("\n<<<<<<<<< Srv recv v6 packet (size %d bytes) >>>>>>>>>\n", iBytesRecv);
                /* get the interface where the packet comes */
                for (cmsg = CMSG_FIRSTHDR(&mh); cmsg != NULL; cmsg = CMSG_NXTHDR(&mh, cmsg)) {
                    if (cmsg->cmsg_level != IPPROTO_IPV6 || cmsg->cmsg_type != IPV6_PKTINFO) { continue; }
                    pi6 = (struct in6_pktinfo*)CMSG_DATA(cmsg);
                    ifindex = pi6->ipi6_ifindex; // require -D_GNU_SOURCE in CFLAGS
                } // for
                /*result = ddp_thread_pre_filter(buf, iBytesRecv, ifindex);
                if (result != 1) {
                    DDP_DEBUG("Drop srv v6 msg, pre-filter result:%d\n", result);
                    goto srv_ipv6_over;
                }*/
                /* allocate memory to store received content */
                payload = (UINT1*)malloc(iBytesRecv * sizeof(UINT1));
                if (payload == NULL) {
                    goto srv_ipv6_over;
                }
                memset(payload, 0, iBytesRecv);
                memcpy(payload, iov.iov_base, iBytesRecv);
                /* allocate packet node to be inserted into msg queue */
                pkt = (struct ddp_message*)malloc(sizeof(struct ddp_message));
                if (pkt == NULL) {
                    if (payload) { free(payload); payload = NULL; }
                    goto srv_ipv6_over;
                }
                memset(pkt, 0, sizeof(struct ddp_message));
                pkt->size = iBytesRecv;
                pkt->payload = payload;
                /* extract src addr */
                in_v6 = (struct sockaddr_in6*)&peer;
                out_v6 = (struct sockaddr_in6*)&pkt->sender;
                out_v6->sin6_family = in_v6->sin6_family;
                memcpy(&out_v6->sin6_addr, &in_v6->sin6_addr, IPV6_ADDRLEN);
                out_v6->sin6_port = in_v6->sin6_port;
                pkt->ifindex = ifindex;
                if (g_debugFlag & DDP_DEBUG_PRINT_RECV_MSG_HEX) {
                    printf("\n***************** SRV RECV V6 MSG *****************\n");
                    printf("if index %d\n", ifindex);
                    if (pkt != NULL){
                        if (pkt->payload != NULL)
                        {
                            DDP_DEBUG("pkt size<%d>, Payload:\n", pkt->size);
                            print_message_hex(pkt->payload, pkt->size);
                        } else { DDP_DEBUG("NULL payload\n"); }
                    } else { DDP_DEBUG("NULL pkt\n"); }
                    printf("\n**********************************************\n");
                }

                /* insert msg to queue */
                if (g_srvMq) {
                    result = msg_queue_insert_msg(g_srvMq, (UINT1*)pkt);
                    if (result < 0) {
                        if (pkt) { free(pkt); pkt = NULL; }
                        DDP_DEBUG("Insert v6 msg into srv queue fail (error %d)\n", result);
                    }
                } else {
                    if (pkt) { free(pkt); pkt = NULL; }
                }
            }
        }
srv_ipv6_over:
        /* Srv IPv4 */
        if (FD_ISSET(g_srvSockfd, &rfds)) {
            if (g_ipv4Ready != 1) { continue; }
            /* packet comes in */
            ifindex = 0;
            mh.msg_namelen = sizeof(peer);
            mh.msg_controllen = sizeof(cmbuf);
            memset(cmbuf, 0, CMBUFSIZE);
            memset(buf, 0, UDP_CONTAINER_SIZE);
            iBytesRecv = recvmsg(g_srvSockfd, &mh, flag);
            if (ddp_run_state(DDP_RUN_STATE_GET, 0) == DDP_RUN_STATE_HALT) { continue; }
            if (iBytesRecv < 0) { printf("Server IPv4 recvmsg err %d\n", errno); }
            /* ddp packet smallest size is 22 bytes */
            if (iBytesRecv >= HDR_END_V4_OFFSET) {
                DDP_DEBUG("\n<<<<<<<<< Srv recv v4 packet (size %d bytes) >>>>>>>>>\n", iBytesRecv);
                /* get the interface where the packet comes */
                for (cmsg = CMSG_FIRSTHDR(&mh); cmsg != NULL; cmsg = CMSG_NXTHDR(&mh, cmsg)) {
                    if (cmsg->cmsg_level != IPPROTO_IP || cmsg->cmsg_type != IP_PKTINFO) { continue; }
                    pi = (struct in_pktinfo*)CMSG_DATA(cmsg);
                    ifindex = pi->ipi_ifindex;
                } // for
                /*result = ddp_thread_pre_filter(buf, iBytesRecv, ifindex);
                if (result != 1) {
                    DDP_DEBUG("Drop v4 msg, pre-filter result:%d, ifindex<%d>\n", result, ifindex);
                    continue;
                }*/
                /* allocate memory to store received content */
                payload = (UINT1*)malloc(iBytesRecv * sizeof(UINT1));
                if (payload == NULL) {
                    continue;
                }
                memset(payload, 0, iBytesRecv);
                memcpy(payload, iov.iov_base, iBytesRecv);
                /* allocate packet node to be inserted into msg queue */
                pkt = (struct ddp_message*)malloc(sizeof(struct ddp_message));
                if (pkt == NULL) {
                    if (payload) { free(payload); payload = NULL; }
                    continue;
                }
                memset(pkt, 0, sizeof(struct ddp_message));
                pkt->size = iBytesRecv;
                pkt->payload = payload;
                /* extract src addr */
                in_v4 = (struct sockaddr_in*)&peer;
                out_v4 = (struct sockaddr_in*)&pkt->sender;
                out_v4->sin_family = in_v4->sin_family;
                memcpy(&out_v4->sin_addr, &in_v4->sin_addr, IPV4_ADDRLEN);
                out_v4->sin_port = in_v4->sin_port;
                pkt->ifindex = ifindex;
                if (g_debugFlag & DDP_DEBUG_PRINT_RECV_MSG_HEX) {
                    printf("\n***************** SRV RECV V4 MSG *****************\n");
                    printf("if index %d\n", ifindex);
                    if (pkt != NULL){
                        if (pkt->payload != NULL)
                        {
                            DDP_DEBUG("pkt size<%d>, Payload:\n", pkt->size);
                            print_message_hex(pkt->payload, pkt->size);
                        } else { DDP_DEBUG("NULL payload\n"); }
                    } else { DDP_DEBUG("NULL pkt\n"); }
                    printf("\n**********************************************\n");
                }

                /* insert msg into queue */
                if (g_srvMq) {
                    result = msg_queue_insert_msg(g_srvMq, (UINT1*)pkt);
                    if (result < 0) {
                        if (pkt) { free(pkt); pkt = NULL; }
                        DDP_DEBUG("Insert v4 msg into srv queue fail (error %d)\n", result);
                    }
                } else {
                    if (pkt) { free(pkt); pkt = NULL; }
                }
            }
        }
    } /* while loop */
}

/* ddp_thread_proc_process
 *   major task of proc thread
 *
 *   strThreadName : thread name
 *
 *   return : none
 */
void
ddp_thread_proc_process
(
    INT1* strThreadName
)
{
    INT4 iLoop = 0;
    struct ddp_message* pkt = NULL;

    while (1) {
        iLoop = ddp_get_loop_flag();
        if (iLoop & DDP_OVER_SIG) { break; }

        if (g_mq == NULL) { continue; }
        pkt = (struct ddp_message*)msg_queue_remove_msg(g_mq);
        if (pkt) {
            DDP_DEBUG("\n+++++++++++++++++ Proc packet +++++++++++++++++\n");
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "input packet info\n");
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "size : %d bytes\n", pkt->size);
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "content in hex :\n");
            if (g_debugFlag & DDP_DEBUG_PRINT_IN_MSG_HEX) {
                print_message_hex(pkt->payload, pkt->size);
            }
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "\n");
            /* Process each packet here */
            ddp_proto_process_message(pkt);

            // After payload is processed, it should be freed.
            if (pkt) {
                if (pkt->payload) {
                    free(pkt->payload);
                    pkt->payload = NULL;
                }
                free(pkt);
                pkt = NULL;
            }
        }
        /* if no pkt in queue */
        else {
            sleep(g_iIntervalCheckQueue);
        }
    } /* while loop */
}

/* ddp_thread_srv_process
 *   major task of srv thread
 *
 *   strThreadName : thread name
 *
 *   return : none
 */
void
ddp_thread_srv_process
(
    INT1* strThreadName
)
{
    INT4 iLoop = 0;
    struct ddp_message* pkt = NULL;

    while (1) {
        iLoop = ddp_get_loop_flag();
        if (iLoop & DDP_OVER_SIG) { break; }
        
        if (g_srvMq == NULL) { continue; }
        pkt = (struct ddp_message*)msg_queue_remove_msg(g_srvMq);
        if (pkt) {
            DDP_DEBUG("\n+++++++++++++++++ Srv packet +++++++++++++++++\n");
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "input packet info\n");
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "size : %d bytes\n", pkt->size);
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "content in hex :\n");
            if (g_debugFlag & DDP_DEBUG_PRINT_IN_MSG_HEX) {
                print_message_hex(pkt->payload, pkt->size);
            }
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_IN_MSG_HEX, "\n");
            /* Process each packet here */
            ddp_srv_process_message(pkt);

            // After payload is processed, it should be freed.
            if (pkt) {
                if (pkt->payload) {
                    free(pkt->payload);
                    pkt->payload = NULL;
                }
                free(pkt);
                pkt = NULL;
            }
        }
        /* if no pkt in queue */
        else {
            sleep(g_iIntervalCheckQueue);
        }
    } /* while loop */
}
