/*
 * ap_test.c
 *
 *  Created on: 2012/12/14
 *      Author: 07771
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ddp.h"
#include "ddp_platform.h"

extern void ddp_detect_endian(void);
INT4 ddp_get_if_list(void);


struct ddp_interface* g_ifSelect = NULL;
struct sockaddr_in g_sender;
struct sockaddr_in g_recv;

INT1* targetDefIp = "172.17.2.172";
UINT1 targetMac[MAC_ADDRLEN] = {0x0, 0x50, 0x56, 0x25, 0xf4, 0x09};

struct ssid_entry test_table_24g[SSID_NUMBER];
struct ssid_entry test_table_5g[SSID_NUMBER];

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

    if (i == 0)
    {
        selNum = i;
    }
    else
    {
        printf("Select an interface to send message (0 ~ %d): ", i);
        scanf(" %d", &selNum);
    }
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
    char cTmp = 0;
    INT1 buf[40];

    memset(&g_recv, 0, sizeof(g_recv));
    g_recv.sin_family = AF_INET;
    g_recv.sin_port = htons(UDP_PORT_CLIENT);
    memset(buf, 0, sizeof(buf));

    printf("\nChange Rcv IP, curent<%s>. (y/n)? ", targetDefIp);
    scanf(" %c", &cTmp);

    if (cTmp == 'n')
    {
        if(inet_pton(AF_INET, targetDefIp, &g_recv.sin_addr.s_addr) != 1) {
            printf("convert ip fail\n");
        }
    }
    else
    {
        printf("Type in receiver's IP: ");
        scanf("%s", buf);
        if (inet_pton(AF_INET, buf, &g_recv.sin_addr.s_addr) != 1) {
            printf("convert ip fail\n");
        }
    }
}

void
set_multi_ssid
(
    int band
)
{
    struct ddp_header hdr;
    UINT1 pkt[1000];
    UINT1* outBody = NULL;
    INT4 pos = 0;
    INT4 count = 0;
    INT4 index = 0;
    INT4 tmp = 0;

    if (band == DDP_WL_BAND_24GHZ) {
        printf("Set multi ssid (2.4G) start\n");
    } else if (band == DDP_WL_BAND_5GHZ) {
        printf("Set multi ssid (5G) start\n");
    } else {
        printf("Frequency band is unknown\n");
        goto set_multi_ssid_over;
    }
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto set_multi_ssid_over;
    }

    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.retCode = REQ_MODE_UNICAST;
    hdr.seq = 6;
    hdr.opcode = DDP_OP_SET_MULTI_SSID;
    memcpy(hdr.macAddr, targetMac, MAC_ADDRLEN);
    memcpy(hdr.ipAddr.ipv4Addr, &g_recv.sin_addr.s_addr, IPV4_ADDRLEN);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_SET_MULTI_SSID;

    memset(pkt, 0, sizeof(pkt));
    outBody = pkt + HDR_END_V4_OFFSET;
    pos = 0;
    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_SET_MULTI_SSID) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(outBody + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(outBody + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }
    *(outBody + pos) = band;
    pos += 1;
    for (index = 0; index < SSID_NUMBER; index++) {
        sprintf((INT1*)(outBody + pos), "SSID-%c-%d", (band + '0'), (index + 1));
        pos += DDP_FIELD_LEN_SSID_NAME;
        tmp = (12 + index);
        tmp = DDP_HTONL(tmp);
        memcpy((INT1*)(outBody + pos), (INT1*)&tmp, sizeof(tmp));
        pos += DDP_FIELD_LEN_VLAN_ID;
        tmp = 1;
        tmp = DDP_HTONL(tmp);
        memcpy((INT1*)(outBody + pos), (INT1*)&tmp, sizeof(tmp));
        pos += DDP_FIELD_LEN_SSID_SEC_TYPE;
        sprintf((INT1*)(outBody + pos), "PASS-%c-p%d", (band + '0'), (index + 2));
        pos += DDP_FIELD_LEN_SSID_SEC_PASSWD;
    }
    
    pack_header(pkt, &hdr);

    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_SET_MULTI_SSID, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto set_multi_ssid_over;
    }

    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1000, 0);
        if (tmp <= 0) { sleep(1); }
        else {
            printf("set multi ssid response\n");
            for (pos = 0; pos < tmp; pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n");
            goto set_multi_ssid_over;
        }
        count++;
    }

set_multi_ssid_over:
    if (band == DDP_WL_BAND_24GHZ) {
        printf("Set multi ssid (2.4G) test over\n");
    } else if (band == DDP_WL_BAND_5GHZ) {
        printf("Set multi ssid (5G) test over\n");
    } else {
        printf("Set multi ssid test over\n");
    }
}

void
get_multi_ssid
(
    int band
)
{
    struct ddp_header hdr;
    UINT1 pkt[1000];
    UINT1* outBody = NULL;
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;

    if (band == DDP_WL_BAND_24GHZ) {
        printf("Get multi ssid (2.4G) start\n");
    } else if (band == DDP_WL_BAND_5GHZ) {
        printf("Get multi ssid (5G) start\n");
    } else if (band == DDP_WL_BAND_ALL) {
        printf("Get multi ssid (all) start\n");
    } else {
        printf("Frequency band is unknown\n");
        goto get_multi_ssid_over;
    }
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto get_multi_ssid_over;
    }

    memset(&hdr, 0, sizeof(hdr));

    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.retCode = REQ_MODE_UNICAST;
    hdr.seq = 6;
    hdr.opcode = DDP_OP_GET_MULTI_SSID;
    memcpy(hdr.macAddr, targetMac, MAC_ADDRLEN);
    memcpy(hdr.ipAddr.ipv4Addr, &g_recv.sin_addr.s_addr, IPV4_ADDRLEN);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_GET_MULTI_SSID;

    memset(pkt, 0, sizeof(pkt));
    outBody = pkt + HDR_END_V4_OFFSET;
    pos = 0;
    *(outBody + pos) = band;
    
    pack_header(pkt, &hdr);

    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_GET_MULTI_SSID, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto get_multi_ssid_over;
    }

    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, sizeof(pkt), 0);
        if (tmp <= 0) { sleep(1); }
        else {
            printf("get multi ssid response\n");
            for (pos = 0; pos < tmp; pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n");
            if (band != DDP_WL_BAND_ALL) { goto get_multi_ssid_over; }
        }
        count++;
    }

get_multi_ssid_over:
    if (band == DDP_WL_BAND_24GHZ) {
        printf("Get multi ssid (2.4G) test over\n");
    } else if (band == DDP_WL_BAND_5GHZ) {
        printf("Get multi ssid (5G) test over\n");
    } else if (band == DDP_WL_BAND_ALL) {
        printf("Get multi ssid (all) test over\n");
    } else {
        printf("Set multi ssid test over\n");
    }
}


int
main
(
    INT4 argc,
    INT1** argv
)
{
    INT4 item;
    INT4 opt;

    ddp_detect_endian();
    g_debugFlag |= DDP_DEBUG_PRINT_PLATFORM;
    memset(&g_sender, 0, sizeof(g_sender));
    g_sender.sin_family = AF_INET;
    g_sender.sin_addr.s_addr = htonl(INADDR_ANY);
    g_sender.sin_port = htons(UDP_PORT_SERVER);
    g_iSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_iSockfd <= 0) {
        printf("Create v4 socket fail\n");
        return -1;
    }
    opt = 1;
    if (fcntl(g_iSockfd, F_SETFL, O_NONBLOCK, opt) != 0) {
        printf("%s (%d) : set sock nonblock fail\n", __FUNCTION__, __LINE__);
    }
    if (bind(g_iSockfd, (struct sockaddr*)&g_sender, sizeof(g_sender)) != 0) {
        printf("bind fail\n");
    }
    else
    {
        printf("\nBind at port %X\n", g_sender.sin_port);
    }

    ddp_get_if_list();

    select_ifs();

    change_recv_ip();

    while (1) {
        printf("Select test item:\n");
        printf("  0: test over\n");
        printf("  1: change interface\n");
        printf("  2: change receiver IP\n");
        printf("  3: set multi ssid (2.4G)\n");
        printf("  4: get multi ssid (2.4G)\n");
        printf("  5: set multi ssid (5G)\n");
        printf("  6: get multi ssid (5G)\n");
        printf("  7: get multi ssid (all)\n");

        scanf("%d", &item);

        switch (item) {
            case 0:
                goto ap_test_over;
                break;
            case 1:
                select_ifs();
                break;
            case 2:
                change_recv_ip();
                break;
            case 3:
                set_multi_ssid(DDP_WL_BAND_24GHZ);
                break;
            case 4:
                get_multi_ssid(DDP_WL_BAND_24GHZ);
                break;
            case 5:
                set_multi_ssid(DDP_WL_BAND_5GHZ);
                break;
            case 6:
                get_multi_ssid(DDP_WL_BAND_5GHZ);
                break;
            case 7:
                get_multi_ssid(DDP_WL_BAND_ALL);
                break;
            default:
                printf("item is not available\n");
                break;
        }
        sleep(1);
    }

ap_test_over:
    if (g_iSockfd) { close(g_iSockfd); g_iSockfd = 0; }
    return 0;
}
