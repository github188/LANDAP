/*
 * relay_test.c
 *   Although relay packet would not be implemented at final,
 *     relay packet is used to test the architecture of DDP server.
 *
 *   The test flow is:
 *   Test program sends relay packet to DDP server.
 *   DDP server recognizes the relay packet and transforms relay packet to request packet.
 *   DDP client responds to request packet and send reply packet back to DDP server.
 *   DDP server transforms reply packet to relay packet and sends it back to test program.
 *
 *   Setting:
 *   Test program waits reply packet at port 60000.
 *   Main thread controls the flow and send packet to DDP server.
 *   Recv thread receives packets sent by DDP packet.
 */


#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "ddp.h"
#include "ddp_platform.h"

extern void ddp_detect_endian(void);
INT4 ddp_get_if_list(void);

#define TEST_PROGRAM_PORT 60000

struct ddp_interface* g_ifSelect = NULL;
struct sockaddr_in g_sender;
struct sockaddr_in g_recv;
INT4 loop = 1;


void
print_menu
(
    void
)
{
    printf("Select test item:\n");
    printf("  0: test over\n");
    printf("  1: change interface\n");
    printf("  2: change recv ip\n");
    printf("  3: send relay msg\n");
    printf("  h: display this menu\n");
}

void
select_ifs
(
    void
)
{
    INT4 i = 0;
    INT4 selNum = 0;
    struct ddp_interface* ifs = NULL;
    ifs = g_ifList->next;
    while (ifs) {
        if (ifs->next) { i++; }
        ifs = ifs->next;
    }
    printf("Select an interface to send message (0 ~ %d): ", i);
    scanf("%d", &selNum);
    i = 0;
    ifs = g_ifList->next;
    while (ifs) {
        if (i == selNum) {
            g_ifSelect = ifs;
            printf("if name %s\n",ifs->name);
            return;
        }
        i++;
        ifs = ifs->next;
    }
    if (ifs == NULL) {
        printf("No interface is selected\n");
    }
}

void
change_recv_ip
(
    void
)
{
    INT1 buf[40];

    memset(&g_recv, 0, sizeof(g_recv));
    g_recv.sin_family = AF_INET;
    g_recv.sin_port = htons(UDP_PORT_SERVER);
    memset(buf, 0, sizeof(buf));
    printf("Type in receiver's IP: ");
    scanf("%s", buf);
    if (inet_pton(AF_INET, buf, &g_recv.sin_addr.s_addr) != 1) {
        printf("convert ip fail\n");
    }
}

void
send_discover_msg
(
    void
)
{
    UINT1 buf[100];
    struct ddp_header hdr;
    
    memset(buf, 0, sizeof(buf));
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_RELAY;
    hdr.seq = 12;
    hdr.opcode = DDP_OP_DISCOVERY;
    memcpy(hdr.macAddr, MAC_ALL, MAC_ADDRLEN);
    hdr.protoVer = DDP_PROTO_V2;
    pack_header(buf, &hdr);
    
    if (sendto(g_iSockfd, buf, HDR_END_V4_OFFSET, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("ERR %d\n",errno);
    }
}

/* recv thread */
void
*test_recv_thread
(
    void* ptr
)
{
    UINT1 buf[UDP_CONTAINER_SIZE];
    INT4 tmp = 0;
    INT4 pos = 0;

    while (1) {
        if (loop != 1) { break; }

        memset(buf, 0, sizeof(buf));
        tmp = recv(g_iSockfd, buf, 1000, 0);
        if (tmp <= 0) { sleep(1); }
        else {
            printf("===== response =====\n");
            for (pos = 0; pos < tmp; pos++) {
                printf("%02X ", buf[pos]);
                if ((pos + 1) % 16 == 0) { printf("\n"); }
            }
            printf("\n====================\n");
        }
    }

    printf("Recv thread exit\n");
    return ptr;
}

/* main thread */
int
main
(
    INT4 argc,
    INT1** argv
)
{
    INT1 item = 0;
    INT4 opt = 0;
    pthread_t recvThreadId = 0;

    printf("Srv test starts\n");
    ddp_detect_endian();

    memset(&g_sender, 0, sizeof(g_sender));
    g_sender.sin_family = AF_INET;
    g_sender.sin_addr.s_addr = htonl(INADDR_ANY);
    g_sender.sin_port = htons(TEST_PROGRAM_PORT);
    g_iSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_iSockfd <= 0) {
        printf("Create v4 socket fail\n");
        return -1;
    }
    opt = 1;
    if (fcntl(g_iSockfd, F_SETFL, O_NONBLOCK, opt) != 0) {
        printf("%s (%d) : set sock nonblock fail\n", __FILE__, __LINE__);
    }
    opt = 1;
    if (setsockopt(g_iSockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        printf("%s (%d) : set cli v6 reuseaddr fail\n", __FILE__, __LINE__);
    }
    if (bind(g_iSockfd, (struct sockaddr*)&g_sender, sizeof(g_sender)) != 0) {
        printf("bind fail\n");
    }
    ddp_get_if_list();
    select_ifs();
    change_recv_ip();
    
    if (pthread_create(&recvThreadId, NULL, &test_recv_thread, (void*)"Test Recv thread") != 0) {
        printf("%s (%d) : fail to create recv thread\n", __FILE__, __LINE__);
        goto switch_test_over; 
    }
    print_menu();
    /* main thread loop */
    while (1) {
        item = 0;
        scanf("%c", &item);

        switch (item) {
            case '0':
                loop = 0;
                goto switch_test_over;
                break;
            case '1':
                select_ifs();
                break;
            case '2':
                change_recv_ip();
                break;
            case '3':
                send_discover_msg();
                break;
            case 'h':
                print_menu();
                break;
            default:
                //printf("item is not available\n");
                break;
        }
        sleep(1);
    }

switch_test_over:
    if (recvThreadId) { pthread_join(recvThreadId, NULL); recvThreadId = 0; }
    if (g_iSockfd) { close(g_iSockfd); g_iSockfd = 0; }
    printf("Srv test stops\n");
    return 0;
}
