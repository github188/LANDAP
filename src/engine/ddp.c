
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>


#include "version.h"
#include "ddp.h"
#include "ddp_platform.h"
#include "msg_queue.h"
#include "tftp_cli.h"


/* Global variables */
/* debug flag */
UINT4 g_debugFlag = DDP_DEBUG_NONE
                  | DDP_DEBUG_GENERAL
                  //| DDP_DEBUG_PRINT_RECV_MSG_HEX
                  | DDP_DEBUG_PRINT_IN_MSG_HDR
                  //| DDP_DEBUG_PRINT_IN_MSG_HEX
                  | DDP_DEBUG_PRINT_SRV_MSG_HDR
                  //| DDP_DEBUG_PRINT_CMD
                  | DDP_DEBUG_PRINT_PLATFORM
                  | DDP_DEBUG_PRINT_OUT_MSG_HDR
                  //| DDP_DEBUG_PRINT_OUT_MSG_HEX
                  ;
/* process running flag */
INT4 g_iLoop = 0;
static pthread_mutex_t g_loopMutex = PTHREAD_MUTEX_INITIALIZER;
/* timer id */
static timer_t g_reportTimer = 0;
INT4 g_reportTimerInterval = REPORT_PACKET_INTERVAL_DEFAULT;
/* socket id */
INT4 g_iSockfd = 0;
INT4 g_iSockfdV6 = 0;
/* IPv4 support (0 -> not yet, 1 -> ready) */
INT4 g_ipv4Ready = 1;
/* IPv6 support (0 -> not yet, 1 -> ready) */
INT4 g_ipv6Ready = 1;
/* endian of machine (0 -> big, 1 -> little) */
INT4 g_iMachEndian = 0;
/* message queue */
struct msg_queue* g_mq = NULL;
INT4 g_maxQueueLen = MSG_QUEUE_LEN_UNLIMIT;
/* list to keep information of network interface in local machine */
struct ddp_interface* g_ifList = NULL;
/* mac address with all 0xFF */
UINT1 MAC_ALL[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
/* localhost address in ipv4 */
UINT1 IPV4_LOCAL[] = { 127, 0, 0, 1 };
/* broadcast address in ipv4 */
UINT1 IPV4_BRCAST[] = { 0xFF, 0xFF, 0xFF, 0xFF };
INT1* DDP_IPV6_MCAST = DDP_IPV6_MULTI_ADDR;
/* seq counter */
UINT2 g_uiSeqCounter = MIN_SEQ;
/* role of ddp */
UINT4 g_role = DDP_ROLE_NONE;
/* srv msg queue */
struct msg_queue* g_srvMq = NULL;
INT4 g_srvSockfd = 0;
INT4 g_srvSockfdV6 = 0;
/* process run state */
INT4 g_runState = DDP_RUN_STATE_RUN;
/* op support list
 *   When opcode is supported, add an entry here.
 */
struct op_info g_opList[] = {
    { DDP_OP_GENERAL_REPORT,      DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_DISCOVERY,           DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_SET_BASIC_INFO,      DDP_OP_DISABLE, DDP_OP_AUTHEN_ON  },
    { DDP_OP_USER_VERIFY,         DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_CHANGE_ID_PSW,       DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },

    { DDP_OP_QUERY_SUPPORT_OPT,   DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_DEVICE_ALERT_INFO,   DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_SNMP_CFG,            DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_SNMP_GET,            DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_FW_UPGRADE,          DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_CFG_BACKUP,          DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_CFG_RESTORE,         DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },

    { DDP_OP_REBOOT,              DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_GET_SYS_DATE_TIME,   DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_SET_SYS_DATE_TIME,   DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },

    { DDP_OP_QUERY_NEIGHBOR_INFO, DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_VLAN_CFG,            DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_VLAN_GET,            DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_ASV_CFG,             DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_ASV_GET,             DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_ASV_USR_MAC_CFG,     DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_ASV_USR_MAC_GET,     DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_GET_SYS_PORT_NUM,    DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    
    { DDP_OP_SET_MULTI_SSID,      DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_GET_MULTI_SSID,      DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    
    { DDP_OP_RESET,               DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_FACTORY_RESET,       DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_DDP_INFO,            DDP_OP_ENABLE,  DDP_OP_AUTHEN_OFF },
    { DDP_OP_SET_IPV4_ADDR,       DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { DDP_OP_SET_IPV6_ADDR,       DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON, },
    { DDP_OP_SET_DEVICE_INFO,     DDP_OP_ENABLE,  DDP_OP_AUTHEN_ON  },
    { 0, 0, 0 } /* end */
};


/* ddp_get_op_number
 *   function to return total number of supported opcode
 *
 *   return : total number of supported opcode
 */
INT4
ddp_get_op_number
(
    void
)
{
    return (sizeof(g_opList) / sizeof(g_opList[0]));
}

/* ddp_run_state
 *   function to get and set run state.
 *
 *   action : DDP_RUN_STATE_GET or DDP_RUN_STATE_SET
 *   value  : new run state if action = DDP_RUN_STATE_SET
 *            0 if action = DDP_RUN_STATE_GET
 *
 *   return : the current state if action = DDP_RUN_STATE_GET
 *            the previous state if action = DDP_RUN_STATE_SET
 *            -1 -> error
 */
INT4
ddp_run_state
(
    INT4 action,
    INT4 value
)
{
    INT4 tmp = DDP_RUN_STATE_HALT;

    if (action == DDP_RUN_STATE_SET) {
        if (value == DDP_RUN_STATE_HALT || value == DDP_RUN_STATE_RUN) {
            tmp = g_runState;
            g_runState = value;
        } else {
            tmp = -1;
        }
        return tmp;
    }
    else {
        return g_runState;
    }
}

/* ddp_get_seq_count
 *   function to get next sequence number
 *
 *   return : sequence number
 */
UINT2
ddp_get_seq_count
(
    void
)
{
    if (g_uiSeqCounter == MAX_SEQ) {
        g_uiSeqCounter = MIN_SEQ;
    } else {
        g_uiSeqCounter++;
    }

    return g_uiSeqCounter;
}

/* ddp_detect_endian
 *   function to check the endian of the local machine
 *
 *   return 0: big, 1: little
 */
void
ddp_detect_endian
(
    void
)
{
    UINT4 i;
    UINT4 endianness = 0x1;

    UINT1* uc = (unsigned char*)&endianness;
    for (i = 0; i < sizeof(UINT4); i++) {
        DDP_DEBUG("%02X ", *(uc + i));
    }

    if (*(UINT1*)&endianness == 0x1) {
        g_iMachEndian = 1;
    }
    if (g_iMachEndian) {
        DDP_DEBUG("-> Little Endian machine\n");
    } else {
        DDP_DEBUG("-> Big Endian machine\n");
    }
}

/* ddp_stop_report_timer
 *   function to stop timer to generate general report
 *
 *   return : none
 */
void
ddp_stop_report_timer
(
    void
)
{
    if (g_reportTimer) {
        timer_delete(g_reportTimer);
        g_reportTimer = 0;
    }
}

/* ddp_get_loop_flag
 *   function to get the flag that represent process is running or not
 *
 *   return : 0 -> stop, 1 -> running
 */
INT4
ddp_get_loop_flag
(
    void
)
{
    INT4 iTmp = 0;
    pthread_mutex_lock(&g_loopMutex);
    iTmp = g_iLoop;
    pthread_mutex_unlock(&g_loopMutex);
    return iTmp;
}

/* ddp_terminate
 *   function to terminate process
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_terminate
(
    void
)
{
    pthread_mutex_lock(&g_loopMutex);
    g_iLoop = (g_iLoop | DDP_OVER_SIG);
    pthread_mutex_unlock(&g_loopMutex);
    tftp_terminate();
    ddp_stop_report_timer();
    ddp_shell_stop();
    return 0;
}

/* ddp_signal_handler
 *   function to handle signal from system
 *
 *   signo : signal id
 *
 *   return : none
 */
void
ddp_signal_handler
(
    INT4 signo
)
{
    if (signo == SIGUSR1) {
        ddp_terminate();
    }
}

/* ddp_report_timer_handler
 *   function to handle report timer signal
 *
 *   i : nothing but unify api
 *
 *   return : none
 */
void
ddp_report_timer_handler
(
    INT4 signo
)
{
    struct ddp_message* pkt = NULL;
    struct ddp_message* pkt6 = NULL;
    INT4 iLoop = ddp_get_loop_flag();
    UINT4 ifindex = 0xFFFFFFFF;
    UINT2 ident = 0;
    UINT2 opcode = DDP_OP_GENERAL_REPORT;
    UINT2 pver = DDP_PROTO_V2;
    UINT2 seq = 0;
    INT1 addr[16];

    if (signo != SIGUSR2) { return; }
    // if it is not client mode, do not send report.
    if ((g_role & DDP_ROLE_CLIENT) == 0) { return; }

    if ((iLoop & DDP_OVER_SIG) != 0) {
        ddp_stop_report_timer();
    } else {
        if (ddp_run_state(DDP_RUN_STATE_GET, 0) == DDP_RUN_STATE_HALT) { return; }
        if (g_ipv4Ready == 1) {
            pkt = (struct ddp_message*)malloc(sizeof(struct ddp_message));
            if (pkt) {
                memset(pkt, 0, sizeof(struct ddp_message));
                pkt->payload = (UINT1*)malloc(HDR_END_V4_OFFSET * sizeof(UINT1));
                if (pkt->payload) {
                    memset(pkt->payload, 0, HDR_END_V4_OFFSET * sizeof(UINT1));
                    ident = IPV4_REQ;
                    ident = DDP_HTONS(ident);
                    memcpy(pkt->payload + HDR_IDENT_OFFSET, &ident, sizeof(ident));

                    seq = ddp_get_seq_count();
                    seq = DDP_HTONS(seq);
                    memcpy(pkt->payload + HDR_SEQ_OFFSET, &seq, sizeof(seq));
                    opcode = DDP_OP_GENERAL_REPORT;
                    opcode = DDP_HTONS(opcode);
                    memcpy(pkt->payload + HDR_OPCODE_OFFSET, &opcode, sizeof(opcode));

                    memcpy(pkt->payload + HDR_MAC_OFFSET, MAC_ALL, MAC_ADDRLEN);
                    pver = DDP_PROTO_V2;
                    pver = DDP_HTONS(pver);
                    memcpy(pkt->payload + HDR_PVER_V4_OFFSET, &pver, sizeof(pver));

                    memcpy(pkt->payload + HDR_IP_OFFSET, IPV4_BRCAST, IPV4_ADDRLEN);
                
                    pkt->size = HDR_END_V4_OFFSET;
                    memcpy(&pkt->ifindex, &ifindex, sizeof(pkt->ifindex));
                    msg_queue_insert_msg(g_mq, (UINT1*)pkt);
                } else {
                    DDP_DEBUG("%s (%d) : allocate payload in v4 report req message failed\n", __FILE__, __LINE__);
                }
            } else {
                DDP_DEBUG("%s (%d) : allocate report v4 req message failed\n", __FILE__, __LINE__);
            } 
        } /* ipv4 */
        if (g_ipv6Ready == 1) {
            pkt6 = (struct ddp_message*)malloc(sizeof(struct ddp_message));
            if (pkt6) {
                memset(pkt6, 0, sizeof(struct ddp_message));
                pkt6->payload = (UINT1*)malloc(HDR_END_V6_OFFSET * sizeof(UINT1));
                if (pkt6->payload) {
                    memset(pkt6->payload, 0, HDR_END_V6_OFFSET * sizeof(UINT1));
                    ident = IPV6_REQ;
                    ident = DDP_HTONS(ident);
                    memcpy(pkt6->payload + HDR_IDENT_OFFSET, &ident, sizeof(ident));

                    seq = ddp_get_seq_count();
                    seq = DDP_HTONS(seq);
                    memcpy(pkt6->payload + HDR_SEQ_OFFSET, &seq, sizeof(seq));
                    opcode = DDP_OP_GENERAL_REPORT;
                    opcode = DDP_HTONS(opcode);
                    memcpy(pkt6->payload + HDR_OPCODE_OFFSET, &opcode, sizeof(opcode));

                    memcpy(pkt6->payload + HDR_MAC_OFFSET, MAC_ALL, MAC_ADDRLEN);
                    pver = DDP_PROTO_V2;
                    pver = DDP_HTONS(pver);
                    memcpy(pkt6->payload + HDR_PVER_V6_OFFSET, &pver, sizeof(pver));

                    //memcpy(pkt->payload + HDR_IP_OFFSET, IPV4_BRCAST, IPV4_ADDRLEN);
                    if (inet_pton(AF_INET6, DDP_IPV6_MCAST, addr) != 1) {
                        DDP_DEBUG("%s (%d) : set ipv6 multicast address fail (error %d)\n", __FILE__, __LINE__, errno);
                    }
                    memcpy(pkt6->payload + HDR_IP_OFFSET, addr, sizeof(addr));
                    pkt6->size = HDR_END_V6_OFFSET;
                    memcpy(&pkt6->ifindex, &ifindex, sizeof(pkt6->ifindex));
                    msg_queue_insert_msg(g_mq, (UINT1*)pkt6);
                } else {
                    DDP_DEBUG("%s (%d) : allocate payload in v6 report req message failed\n", __FILE__, __LINE__);
                }
            } else {
                DDP_DEBUG("%s (%d) : allocate report v6 req message failed\n", __FILE__, __LINE__);
            } 
        } /* ipv6 */
    } /* if DDP_OVER_SIG */
}

/* ddp_create_report_timer
 *   function to create the report timer
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_create_report_timer
(
    void
)
{
    struct sigevent sev;
    struct itimerspec its;

    // If it is not client mode, do not send report.
    if ((g_role & DDP_ROLE_CLIENT) == 0) { return 0; }

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGUSR2;
    sev.sigev_value.sival_ptr = &g_reportTimer;
    if (timer_create(CLOCK_REALTIME, &sev, &g_reportTimer) != 0) {
        DDP_DEBUG("%s (%d) : report timer create failed\n", __FILE__, __LINE__);
        return -1;
    }

    its.it_value.tv_sec = g_reportTimerInterval;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = g_reportTimerInterval;
    its.it_interval.tv_nsec = 0;
    if (timer_settime(g_reportTimer, 0, &its, NULL) != 0) {
        DDP_DEBUG("%s (%d) : start report timer failed (%d)\n", __FILE__, __LINE__, errno);
        return -2;
    }

    return 0;
}

/* ddp_get_if_list
 *   function to retrieve interface list form platform
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_get_if_list
(
    void
)
{
    INT4 len = 0;
    struct ddp_interface* n = NULL;
    INT4 count = 0;
    INT4 i = 0;
    UINT1 uc;
    ddp_platform_get_if_list(&g_ifList, &len);
    if (g_ifList == NULL) {
        DDP_DEBUG("%s (%d) : get interface fail\n", __FILE__, __LINE__);
        return -1;
    }

    DDP_DEBUG("----- IF list: -----\n");
    n = g_ifList->next;
    while (n) {
        DDP_DEBUG("IF %d (index %d): ", count, n->ifindex);
        for (i = 0; i < 6; i++) {
            uc = *(n->macAddr + i);
            DDP_DEBUG(" %02X ", uc);
        }
        DDP_DEBUG("\n");
        count++;
        n = n->next;
    }
    DDP_DEBUG("--------------------\n");

    return 0;
}

/* ddp_create_udp_socket
 *   function to create sockets for ipv4 and ipv6 to listen ddp packet from server
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_create_udp_socket
(
    void
)
{
    struct sockaddr_in sa_v4;
    struct sockaddr_in6 sa_v6;
    INT4 ret = 0;
    INT4 opt = 1;

    /* IPv6 */
    if (g_ipv6Ready == 1) {
        memset(&sa_v6, 0, sizeof(sa_v6));
        sa_v6.sin6_family = AF_INET6;
        sa_v6.sin6_addr = in6addr_any;
        /* Cli IPv6 */
        if ((g_role & DDP_ROLE_CLIENT) != 0) {
            DDP_DEBUG("Create IPv6 socket for Client mode\n");
            sa_v6.sin6_port = htons(UDP_PORT_CLIENT);
            g_iSockfdV6 = socket(AF_INET6, SOCK_DGRAM, 0);
            if (g_iSockfdV6 <= 0) {
                DDP_DEBUG("%s (%d) : create v6 socket fail\n", __FILE__, __LINE__);
                ret = -2;
                goto create_sock_over;
            }
            opt = 1;
            if (fcntl(g_iSockfdV6, F_SETFL, O_NONBLOCK, opt) != 0) {
                DDP_DEBUG("%s (%d) : set v6 sock nonblock fail\n", __FILE__, __LINE__);
            }
            opt = 1;
            if (setsockopt(g_iSockfdV6, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) != 0) {
                DDP_DEBUG("%s (%d) : set ipv6 only fail\n", __FILE__, __LINE__);
            }
            opt = 1;
#ifdef __CYGWIN__
            if (setsockopt(g_iSockfdV6, IPPROTO_IPV6, IPV6_PKTINFO, &opt, sizeof(opt)) != 0) {
#else
            if (setsockopt(g_iSockfdV6, IPPROTO_IPV6, IPV6_RECVPKTINFO, &opt, sizeof(opt)) != 0) {
#endif
                DDP_DEBUG("%s (%d) : set v6 IP_PKTINFO fail (%d)\n", __FILE__, __LINE__, errno);
            }
            opt = 1;
            if (setsockopt(g_iSockfdV6, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
                DDP_DEBUG("%s (%d) : set v6 reuseaddr fail\n", __FILE__, __LINE__);
            }
            if (bind(g_iSockfdV6, (struct sockaddr*)&sa_v6, sizeof(sa_v6)) != 0) {
                DDP_DEBUG("%s (%d) : v6 socket bind err\n", __FILE__, __LINE__);
                ret = -3;
            }
        }
        /* Srv IPv6 */
        if ((g_role & DDP_ROLE_SERVER) != 0) {
            DDP_DEBUG("Create IPv6 socket for Server mode\n");
            sa_v6.sin6_port = htons(UDP_PORT_SERVER);
            g_srvSockfdV6 = socket(AF_INET6, SOCK_DGRAM, 0);
            if (g_srvSockfdV6 <= 0) {
                DDP_DEBUG("%s (%d) : create srv v6 socket fail\n", __FILE__, __LINE__);
                ret = -2;
                goto create_sock_over;
            }
            opt = 1;
            if (fcntl(g_srvSockfdV6, F_SETFL, O_NONBLOCK, opt) != 0) {
                DDP_DEBUG("%s (%d) : set srv v6 sock nonblock fail\n", __FILE__, __LINE__);
            }
            opt = 1;
            if (setsockopt(g_srvSockfdV6, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) != 0) {
                DDP_DEBUG("%s (%d) : set srv ipv6 only fail\n", __FILE__, __LINE__);
            }
            opt = 1;
#ifdef __CYGWIN__
            if (setsockopt(g_srvSockfdV6, IPPROTO_IPV6, IPV6_PKTINFO, &opt, sizeof(opt)) != 0) {
#else
            if (setsockopt(g_srvSockfdV6, IPPROTO_IPV6, IPV6_RECVPKTINFO, &opt, sizeof(opt)) != 0) {
#endif
                DDP_DEBUG("%s (%d) : set srv v6 IP_PKTINFO fail (%d)\n", __FILE__, __LINE__, errno);
            }
            opt = 1;
            if (setsockopt(g_srvSockfdV6, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
                DDP_DEBUG("%s (%d) : set srv v6 reuseaddr fail\n", __FILE__, __LINE__);
            }
            if (bind(g_srvSockfdV6, (struct sockaddr*)&sa_v6, sizeof(sa_v6)) != 0) {
                DDP_DEBUG("%s (%d) : srv v6 socket bind err\n", __FILE__, __LINE__);
                ret = -3;
            }
        }
    }
    /* IPv4 */
    if (g_ipv4Ready == 1) {
        memset(&sa_v4, 0, sizeof(sa_v4));
        sa_v4.sin_family = AF_INET;
        sa_v4.sin_addr.s_addr = htonl(INADDR_ANY);
        /* Cli IPv4 */
        if ((g_role & DDP_ROLE_CLIENT) != 0) {
            DDP_DEBUG("Create IPv4 socket for Client mode\n");
            sa_v4.sin_port = htons(UDP_PORT_CLIENT);
            g_iSockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (g_iSockfd <= 0) {
                DDP_DEBUG("%s (%d) : create v4 socket fail\n", __FILE__, __LINE__);
                ret = -3;
                goto create_sock_over;
            }
            opt = 1;
            if (fcntl(g_iSockfd, F_SETFL, O_NONBLOCK, opt) != 0) {
                DDP_DEBUG("%s (%d) : set v4 sock nonblock fail\n", __FILE__, __LINE__);
            }
            opt = 1;
            if (setsockopt(g_iSockfd, IPPROTO_IP, IP_PKTINFO, &opt, sizeof(opt)) != 0) {
                DDP_DEBUG("%s (%d) : set v4 IP_PKTINFO fail\n", __FILE__, __LINE__);
            }
            opt = 1;
            if (setsockopt(g_iSockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) != 0) {
                DDP_DEBUG("%s (%d) : set v4 broadcast fail\n", __FILE__, __LINE__);
            }
            opt = 1;
            if (setsockopt(g_iSockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
                DDP_DEBUG("%s (%d) : set v4 reuseaddr fail\n", __FILE__, __LINE__);
            }
            if (bind(g_iSockfd, (struct sockaddr*)&sa_v4, sizeof(sa_v4)) != 0) {
                DDP_DEBUG("%s (%d) : v4 socket bind err (%d)\n", __FILE__, __LINE__, errno);
                ret = -4;
            }
        }
        /* Srv IPv4 */
        if ((g_role & DDP_ROLE_SERVER) != 0) {
            DDP_DEBUG("Create IPv4 socket for Server mode\n");
            sa_v4.sin_port = htons(UDP_PORT_SERVER);
            g_srvSockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (g_srvSockfd <= 0) {
                DDP_DEBUG("%s (%d) : create srv v4 socket fail\n", __FILE__, __LINE__);
                ret = -3;
                goto create_sock_over;
            }
            opt = 1;
            if (fcntl(g_srvSockfd, F_SETFL, O_NONBLOCK, opt) != 0) {
                DDP_DEBUG("%s (%d) : set srv v4 sock nonblock fail\n", __FILE__, __LINE__);
            }
            opt = 1;
            if (setsockopt(g_srvSockfd, IPPROTO_IP, IP_PKTINFO, &opt, sizeof(opt)) != 0) {
                DDP_DEBUG("%s (%d) : set srv v4 IP_PKTINFO fail\n", __FILE__, __LINE__);
            }
            opt = 1;
            if (setsockopt(g_srvSockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) != 0) {
                DDP_DEBUG("%s (%d) : set srv v4 broadcast fail\n", __FILE__, __LINE__);
            }
            opt = 1;
            if (setsockopt(g_srvSockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
                DDP_DEBUG("%s (%d) : set srv v4 reuseaddr fail\n", __FILE__, __LINE__);
            }
            if (bind(g_srvSockfd, (struct sockaddr*)&sa_v4, sizeof(sa_v4)) != 0) {
                DDP_DEBUG("%s (%d) : srv v4 socket bind err (%d)\n", __FILE__, __LINE__, errno);
                ret = -4;
            }
        }
    }

create_sock_over:
    return ret;
}

/* ddp_close_udp_socket
 *   function to close udp sockets created at startup
 *
 *   return : none
 */
void
ddp_close_udp_socket
(
    void
)
{
    if (g_iSockfd > 0) {
        close(g_iSockfd);
        g_iSockfd = 0;
    }
    if (g_iSockfdV6 > 0) {
        close(g_iSockfdV6);
        g_iSockfdV6 = 0;
    }
    if (g_srvSockfd > 0) {
        close(g_srvSockfd);
        g_srvSockfd = 0;
    }
    if (g_srvSockfdV6 > 0) {
        close(g_srvSockfdV6);
        g_srvSockfdV6 = 0;
    }
}

/* ddp_ipv6_multicast
 *   There is no broadcast in IPv6. Multicast is the replacement.
 *   DDP_IPV6_MCAST is defined to be the multicast address.
 *
 *   return: none
 */
void
ddp_ipv6_multicast
(
    void
)
{
    INT4 ret = 0;
    struct ddp_interface* ifs  = NULL;
    struct ipv6_mreq mreq;

    if (inet_pton(AF_INET6, DDP_IPV6_MCAST, &mreq.ipv6mr_multiaddr) != 1) {
        DDP_DEBUG("Set IPv6 multicast address fail\n");
    }
//    ifs = g_ifList->next;
//    while (ifs) {
//        mreq.ipv6mr_interface = ifs->ifindex;
        mreq.ipv6mr_interface = 0;
        if (g_iSockfdV6 > 0) {
            ret = setsockopt(g_iSockfdV6, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
            if (ret != 0) {
                DDP_DEBUG("cli add member (if index %d) fail (error %d)\n", ifs->ifindex, errno);
            }
        }
        if (g_srvSockfdV6 > 0) {
            ret = setsockopt(g_srvSockfdV6, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
            if (ret != 0) {
                DDP_DEBUG("srv add member (if index %d) fail (error %d)\n", ifs->ifindex, errno);
            }
        }
//        ifs = ifs->next;
//    } /* while */
}

/* proc_thread_func
 *   the function that proc thread is running on
 *
 *   ptr : thread name
 *
 *   return : none
 */
void
*proc_thread_func
(
    void * ptr
)
{
    INT1* strThreadName = (INT1*)ptr;

    if (strThreadName) {
        DDP_DEBUG("%s runs\n", strThreadName);
    }
    g_iLoop = (g_iLoop | 0x02);
    ddp_thread_proc_process(strThreadName);

    if (strThreadName) {
        DDP_DEBUG("%s exits\n", strThreadName);
    }
    // stop signal to recv thread
    g_iLoop = (g_iLoop & ~0x02);
    return ptr;
}

/* srv_thread_func
 *   the function that srv thread is running on
 *
 *   ptr : thread name
 *
 *   return : none
 */
void
*srv_thread_func
(
    void * ptr
)
{
    INT1* strThreadName = (INT1*)ptr;

    if (strThreadName) {
        DDP_DEBUG("%s runs\n", strThreadName);
    }
    g_iLoop = (g_iLoop | 0x04);
    ddp_thread_srv_process(strThreadName);

    if (strThreadName) {
        DDP_DEBUG("%s exits\n", strThreadName);
    }
    // stop signal to recv thread
    g_iLoop = (g_iLoop & ~0x04);
    return ptr;
}

/* shell_thread_func
 *   the function that shell thread is running on
 *
 *   ptr : thread name
 *
 *   return : none
 */
void
*shell_thread_func
(
    void * ptr
)
{
    INT1* strThreadName = (INT1*)ptr;

    if (strThreadName) {
        DDP_DEBUG("%s runs\n", strThreadName);
    }
    g_iLoop = (g_iLoop | 0x08);
    ddp_shell_thread(strThreadName);
    if (strThreadName) {
        DDP_DEBUG("%s exits\n", strThreadName);
    }
    // stop signal to recv thread
    g_iLoop = (g_iLoop & ~0x08);
    return ptr;
}

/* alrm_queue_thread_func
 *   the function that alrm_queue thread is running on
 *
 *   ptr : thread name
 *
 *   return : none
 */
void
*alrm_queue_thread_func
(
    void * ptr
)
{
    INT1* strThreadName = (INT1*)ptr;

    if (strThreadName) {
        DDP_DEBUG("%s runs\n", strThreadName);
    }
    g_iLoop = (g_iLoop | 0x10);
    ddp_alrm_queue_thread(strThreadName);
    if (strThreadName) {
        DDP_DEBUG("%s exits\n", strThreadName);
    }
    // stop signal to recv thread
    g_iLoop = (g_iLoop & ~0x10);
    return ptr;
}

/* recv_thread_func
 *   the function that recv thread is running on
 *
 *   ptr : thread name
 *
 *   return : none
 */
void
*recv_thread_func
(
    void * ptr
)
{
    pthread_t procThreadId = 0;
    pthread_t srvThreadId = 0;
    pthread_t shellThreadId = 0;
    pthread_t alrmThreadId = 0;
    INT1* strThreadName = (INT1*)ptr;

    if (g_role & DDP_ROLE_CLIENT) {
        if (pthread_create(&procThreadId, NULL, &proc_thread_func, (void*)"Proc thread") != 0) {
            DDP_DEBUG("%s (%d) : fail to create proc thread\n", __FILE__, __LINE__);
            //ret = -7;
            goto recv_thread_over;
        }
    }

    if (g_role & DDP_ROLE_SERVER) {
        if (pthread_create(&srvThreadId, NULL, &srv_thread_func, (void*)"Srv thread") != 0) {
            DDP_DEBUG("%s (%d) : fail to create srv thread\n", __FILE__, __LINE__);
            //ret = -7;
            goto recv_thread_over;
        }
    }
    if (pthread_create(&shellThreadId, NULL, &shell_thread_func, (void*)"Shell thread") != 0) {
        DDP_DEBUG("%s (%d) : fail to create shell thread\n", __FILE__, __LINE__);
        goto recv_thread_over;
    }
    if (pthread_create(&alrmThreadId, NULL, &alrm_queue_thread_func, (void*)"Alarm thread") != 0) {
        DDP_DEBUG("%s (%d) : fail to create alarm queue thread\n", __FILE__, __LINE__);
        goto recv_thread_over;
    }

    if (strThreadName) {
        DDP_DEBUG("%s runs\n", strThreadName);
    }
    g_iLoop = (g_iLoop | 0x01);
    ddp_thread_recv_process(strThreadName);

recv_thread_over:
    //if (procThreadId) { pthread_join(procThreadId, NULL); procThreadId = 0; }
    //if (srvThreadId) { pthread_join(srvThreadId, NULL); srvThreadId = 0; }
    // wait for threads stopped
    while ((g_iLoop & 0x02) || (g_iLoop & 0x04) || (g_iLoop & 0x08) || g_iLoop & 0x10) {
        sleep(1);
    }

    if (strThreadName) {
        DDP_DEBUG("%s exits\n", strThreadName);
    }
    g_iLoop = (g_iLoop & ~0x01);
    return ptr;
}

/* ddp_print_menu
 *   function to display commandline menu.
 *
 * return:  none
 */
void
ddp_print_menu
(
    void
)
{
    printf("ddpd [-h] [-c] [-s <file>] [-m <file>]\n");
    printf("    -h: display this menu.\n");
    printf("    -c: client mode only.\n");
    printf("    -s: server mode only.\n");
    printf("    -m: client / server mixed mode\n");
    printf("    <file>: server config file, which contains proxy address\n");
}

/* ddp_entrance
 *   entry point of ddp engine
 *
 *   return : 0 -> success, others -> error
 */
int
ddp_entrance
(
    INT4 argc,
    INT1** argv
)
{
    INT4 ret = 0;
    pthread_t recvThreadId = 0;
    INT4 opt;

    ddp_detect_endian();

    g_role = DDP_ROLE_CLIENT; // set default role
    while ((opt = getopt(argc, argv, "hcs:m:")) != -1) {
        switch (opt) {
            case 's':
                if (optarg) {
                    DDP_DEBUG("Config file : %s\n", optarg);
                    g_role = DDP_ROLE_SERVER;
                    if (ddp_srv_process_config(optarg) != 0) {
                        ret = -1;
                        DDP_DEBUG("%s (%d) : process config file fail\n", __FILE__, __LINE__);
                    }
                } else {
                    DDP_DEBUG("%s (%d) : No config file\n", __FILE__, __LINE__);
                }
                break;
            case 'm':
                if (optarg) {
                    DDP_DEBUG("Config file : %s\n", optarg);
                    g_role = (DDP_ROLE_CLIENT | DDP_ROLE_SERVER);
                    if (ddp_srv_process_config(optarg) != 0) {
                        ret = -2;
                        DDP_DEBUG("%s (%d) : process config file fail\n", __FILE__, __LINE__);
                    } 
                } else {
                    DDP_DEBUG("%s (%d) : No config file\n", __FILE__, __LINE__);
                }
                break;
            case 'h':
                ddp_print_menu();
                ret = -1;
                goto ddp_cleanup;
                break;
            case 'c':
            default:
                break;
        }
    }
    if (ret != 0) { goto ddp_cleanup; }
    switch (g_role) {
        case DDP_ROLE_CLIENT:
            DDP_DEBUG("Client mode\n");
            break;
        case DDP_ROLE_SERVER:
            DDP_DEBUG("Server mode\n");
            break;
        case (DDP_ROLE_CLIENT | DDP_ROLE_SERVER):
            DDP_DEBUG("Cli/Srv mixed mode\n");
            break;
        case DDP_ROLE_NONE:
        default:
            DDP_DEBUG("No role is assigned\n");
            ret = -3;
            goto ddp_cleanup;
            break;
    }

    /* init msg queue */
    if (g_role & DDP_ROLE_CLIENT) {
        g_mq = msg_queue_init((INT1*)"MSG QUEUE");
        if (g_mq == NULL) {
            DDP_DEBUG("%s (%d) : init msg queue fail\n", __FILE__, __LINE__);
            ret = -4;
            goto ddp_cleanup;
        }
        ret = msg_queue_set_length(g_mq, g_maxQueueLen);
        if (ret < 0) {
            DDP_DEBUG("%s (%d) : fail to set queue length %d (error %d)\n", __FILE__, __LINE__, g_maxQueueLen, ret);
            goto ddp_cleanup;
        }
    }
    if (g_role & DDP_ROLE_SERVER) {
        g_srvMq = msg_queue_init((INT1*)"SRV MSG QUEUE");
        if (g_srvMq == NULL) {
            DDP_DEBUG("%s (%d) : init srv msg queue fail\n", __FILE__, __LINE__);
            ret = -5;
            goto ddp_cleanup;
        }
        ret = msg_queue_set_length(g_srvMq, g_maxQueueLen);
        if (ret < 0) {
            DDP_DEBUG("%s (%d) : fail to set srv queue length %d (error %d)\n", __FILE__, __LINE__, g_maxQueueLen, ret);
            goto ddp_cleanup;
        }
    }

    /* handler USR1 to exit process safely */
    signal(SIGUSR1, ddp_signal_handler);
    /* handler USR2 to send report */
    signal(SIGUSR2, ddp_report_timer_handler);

    if (ddp_create_udp_socket() != 0) {
        DDP_DEBUG("%s (%d) : init socket fail\n", __FILE__, __LINE__);
        ret = -6;
        goto ddp_cleanup;
    }
    
    /* init platform-dependent setting */
    if (ddp_platform_init() != 0) {
        ret = -7;
        goto ddp_cleanup;
    }
    /* retrieve interface list */
    if (ddp_get_if_list() != 0) {
        ret = -8;
        goto ddp_cleanup;
    }
    /* set multicst for ipv6 */
    ddp_ipv6_multicast();
    /* create and start timer to generate report packet */
    if (ddp_create_report_timer() != 0) {
        ret = -9;
        goto ddp_cleanup;
    }
    /* create unxi socket for ddp shell */
    ddp_shell_start();

    if (pthread_create(&recvThreadId, NULL, &recv_thread_func, (void*)"Recv thread") != 0) {
        DDP_DEBUG("%s (%d) : fail to create recv thread\n", __FILE__, __LINE__);
        ret = -10;
        goto ddp_cleanup; 
    }

ddp_cleanup:
    if (ret != 0) { ddp_terminate(); }
    if (recvThreadId) { pthread_join(recvThreadId, NULL); }
    /* close unix socket */
    ddp_shell_stop();
    /* close udp sockets */
    ddp_close_udp_socket();
    /* release msg queue */
    if (g_mq) { msg_queue_free(g_mq); g_mq = NULL; }
    if (g_srvMq) { msg_queue_free(g_srvMq); g_srvMq = NULL; }
    /* clean up platform */
    ddp_platform_free();
    /* release interface list */
    ddp_platform_free_if_list(g_ifList);
    /* Since tftp thread is detached thread,
     * main thread check upgrade flag to confirm no tftp thread is running.
     */
    while (ddp_upgrade_get_flag() != 0) {
        sleep(1);
    }
    pthread_mutex_destroy(&g_upgradeMutex);
    pthread_mutex_destroy(&g_loopMutex);

    return 0;
}
