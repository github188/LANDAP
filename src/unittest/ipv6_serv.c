/*
 * ipv6_serv.c
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ddp.h"
#include "ddp_platform.h"

void ddp_detect_endian(void);
INT4 ddp_get_if_list(void);

int
discover_test
(
    struct sockaddr_in6* sa
)
{
    UINT1 buf[1000];
    UINT2 count = 0;
    INT4 tmp = 0;
    UINT2 pos = 0;
    struct ddp_header hdr;
    inet_ntop(AF_INET6, &sa->sin6_addr, (char*)buf, 1000);
    printf("send discover req to ip %s\n", buf);

    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV6_FLAG;
    hdr.identifier = IPV6_REQ;
    hdr.seq = 8;
    hdr.opcode = DDP_OP_DISCOVERY;
    memcpy(hdr.macAddr, MAC_ALL, 6);
    hdr.protoVer = DDP_PROTO_V2;
    pack_header(buf, &hdr);
    for (pos = 0; pos < HDR_END_V6_OFFSET; pos++) {
        printf("%02X ",buf[pos]);
        if ((pos + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
    if (sendto(g_iSockfd, buf, HDR_END_V6_OFFSET, 0, (struct sockaddr*)sa, sizeof(*sa)) < 0) {
        printf("ERR %d\n",errno);
    }
    while (count < 10) {
        tmp = recv(g_iSockfd,buf, 1000, 0);
        if (tmp <= 0) { sleep(1); }
        else {
            printf("response\n");
            for (pos = 0; pos < tmp; pos++) {
                printf("%02X ", buf[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n");
            goto discover_test_over;
        }
        count++;
    }

discover_test_over:
    printf("discover test over\n");
    return 0;
}

void
get_ipv6
(
    void)
{
    int fd = 0;
    int i, bytesRead = 0;
    unsigned char buf[200];
    fd = open("/proc/net/if_inet6", O_RDONLY, 0);
    if (fd) {
        bytesRead = read(fd, buf, 200);
        for (i = 0; i < bytesRead; i++) {
            if (buf[i] == 0x20) printf("\nSPACE\n");
            else if (buf[i] == 0x09) printf("\nTAB\n");
            else printf("%02X", buf[i]);
        }
        printf("\n");
    }
}

int
main
(
    INT4 argc,
    INT1** argv
)
{
    struct sockaddr_in6 sa;
    struct sockaddr_in6 rv;
    INT4 input;
    INT4 opt;

    ddp_detect_endian();
    g_debugFlag |= DDP_DEBUG_PRINT_PLATFORM;
    memset(&sa, 0, sizeof(sa));
    sa.sin6_family = AF_INET6;
    sa.sin6_addr = in6addr_any;
    sa.sin6_port = htons(UDP_PORT_SERVER);
    g_iSockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (g_iSockfd <= 0) {
        printf("Create v6 socket fail\n");
        return -1;
    }
    opt = 1;
    if (fcntl(g_iSockfd, F_SETFL, O_NONBLOCK, opt) != 0) {
        printf("%s (%d) : set sock nonblock fail\n", __FILE__, __LINE__);
    }
    if (bind(g_iSockfd, (struct sockaddr*)&sa,sizeof(sa)) != 0) {
        printf("bind fail\n");
    }
    ddp_get_if_list();
    memset(&rv, 0, sizeof(rv));
    rv.sin6_family = AF_INET6;
    //rv.sin6_addr = in6addr_loopback;
    inet_pton(AF_INET6, "fe80:0000:0000:0000:020c:29ff:fe4e:2f14", &rv.sin6_addr);
    rv.sin6_port = htons(UDP_PORT_CLIENT);
    while (1) {
        scanf("%d", &input);
        printf("opt %d\n", input);

        switch (input) {
            case 0:
                goto ipv6_test_over;
                break;
            case 1:
                discover_test(&rv);
                break;
            case 2:
                get_ipv6();
                break;
            default:
                printf("opt is not supported\n");
                break;
        }
        sleep(1);
    }

ipv6_test_over:
    if (g_iSockfd) { close(g_iSockfd); g_iSockfd = 0; }
    return 0;
}
