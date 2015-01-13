/*
 * switch_test.c
 *
 *  Created on: 2012/9/7
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

//172.17.2.151
//#define ksRcvIP "172.17.2.151"
//UINT1 gau1TargetMac[MAC_ADDRLEN]={0x00, 0x0c, 0x29, 0xc6, 0x1b, 0xa6};

//172.17.2.159
//#define ksRcvIP "172.17.2.159"
//UINT1 gau1TargetMac[MAC_ADDRLEN]={0x1C, 0x7E, 0xE5, 0x29, 0xF0, 0xF7};

//172.17.2.36
#define ksRcvIP "172.17.2.36"
UINT1 gau1TargetMac[MAC_ADDRLEN] = {0x0, 0x50, 0x56, 0x25, 0xf4, 0x09};

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

    if(i == 0)
    {
        selNum = i;
    }
    else
    {
        printf("Select an interface to send message (0 ~ %d): ", i);
        scanf("%d", &selNum);
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
    INT1 buf[40];

    memset(&g_recv, 0, sizeof(g_recv));
    g_recv.sin_family = AF_INET;
    g_recv.sin_port = htons(UDP_PORT_CLIENT);
    memset(buf, 0, sizeof(buf));

    printf("\nChange Rcv IP, curent<%s>. (y/n)?", ksRcvIP);
    scanf("%s", buf);

    if (strcmp(buf, "n") == 0)
    {
        strcpy(buf, ksRcvIP);
        if(inet_pton(AF_INET, buf, &g_recv.sin_addr.s_addr) != 1) {
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
get_snmp
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("Get snmp start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto get_snmp_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_SNMP_GET;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_SNMP_GET;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = 0;

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_SNMP_GET, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto get_snmp_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
        }
        count++;
    }

get_snmp_over:
    printf("get snmp test over\n");
}

void
set_snmp
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("Get snmp start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto set_snmp_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_SNMP_CFG;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_SNMP_CFG;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_SNMP_CFG) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    *(pkt + pos) = 0;
    pos += DDP_FIELD_LEN_SNMP_GLOBAL_STATE;
    strcpy((INT1*)(pkt + pos), "NEW COMM RO");

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_SNMP_CFG, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto set_snmp_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto set_snmp_over;
        }
        count++;
    }

set_snmp_over:
    printf("set snmp test over\n");
}

void
fw_upgrade
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("Fw upgrade start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto fw_upgrade_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_FW_UPGRADE;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_FW_UPGRADE_IPV4;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_FW_UPGRADE) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    *(pkt + pos) = 1;
    pos += 1;
    *(pkt + pos) = 1;
    pos += 1;
    *(pkt + pos) = 1;
    pos += 1;
    *(pkt + pos) = 0xAC; *(pkt + pos + 1) = 0x11; *(pkt + pos + 2) = 0x02; *(pkt + pos + 3) = 0xAC;
    pos += 4;
    strcpy((INT1*)(pkt + pos), "/test.c");

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_FW_UPGRADE_IPV4, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto fw_upgrade_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto fw_upgrade_over;
        }
        count++;
    }

fw_upgrade_over:
    printf("fw upgrade test over\n");
}

void
cfg_backup
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("Cfg backup start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto cfg_backup_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_CFG_BACKUP;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_CFG_BACKUP_IPV4;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_CFG_BACKUP) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    *(pkt + pos) = 1;
    pos += 1;
    *(pkt + pos) = 1;
    pos += 1;
    *(pkt + pos) = 1;
    pos += 1;
    *(pkt + pos) = 0xAC; *(pkt + pos + 1) = 0x11; *(pkt + pos + 2) = 0x02; *(pkt + pos + 3) = 0xAC;
    pos += 4;
    strcpy((INT1*)(pkt + pos), "/test.c");

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_CFG_BACKUP_IPV4, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto cfg_backup_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto cfg_backup_over;
        }
        count++;
    }

cfg_backup_over:
    printf("cfg backup test over\n");
}

void
cfg_restore
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("Cfg restore start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto cfg_restore_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_CFG_RESTORE;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_CFG_RESTORE_IPV4;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_CFG_RESTORE) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    *(pkt + pos) = 1;
    pos += 1;
    *(pkt + pos) = 1;
    pos += 1;
    *(pkt + pos) = 1;
    pos += 1;
    *(pkt + pos) = 0xAC; *(pkt + pos + 1) = 0x11; *(pkt + pos + 2) = 0x02; *(pkt + pos + 3) = 0xAC;
    pos += 4;
    strcpy((INT1*)(pkt + pos), "/test.c");

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_CFG_RESTORE_IPV4, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto cfg_restore_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto cfg_restore_over;
        }
        count++;
    }

cfg_restore_over:
    printf("cfg restore test over\n");
}

void
set_ipv4_addr
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;
    INT1 buf[5];

    memset(buf, 0, 5);
    printf("Set ipv4 addr start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto set_ipv4_addr_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_SET_IPV4_ADDR;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_SET_IPV4_ADDR) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    printf("0: IPv4 address\n1: IPv4 DNS\n");
    scanf("%s", buf);
    if (strcmp(buf, "0") == 0) {
        hdr.bodyLen = 142;
        *(pkt + pos) = 0; pos += 1;
        *(pkt + pos) = 0; pos += 1;
        *(pkt + pos) = 0xAC; *(pkt + pos + 1) = 0x11; *(pkt + pos + 2) = 0x02; *(pkt + pos + 3) = 0x30;
        pos += 4;
        *(pkt + pos) = 0xFF; *(pkt + pos + 1) = 0xFF; *(pkt + pos + 2) = 0xFF; *(pkt + pos + 3) = 0;
        pos += 4;
        *(pkt + pos) = 0xAC; *(pkt + pos + 1) = 0x11; *(pkt + pos + 2) = 0x02; *(pkt + pos + 3) = 0x1;
        pos += 4;
    }
    else if (strcmp(buf, "1") == 0) {
        hdr.bodyLen = 138;
        *(pkt + pos) = 1; pos += 1;
        *(pkt + pos) = 0; pos += 1;
        *(pkt + pos) = 0xAC; *(pkt + pos + 1) = 0x11; *(pkt + pos + 2) = 0x02; *(pkt + pos + 3) = 0x30;
        pos += 4;
        *(pkt + pos) = 0xAC; *(pkt + pos + 1) = 0x11; *(pkt + pos + 2) = 0x02; *(pkt + pos + 3) = 3;
        pos += 4;
    }
    else {
        printf("Wrong message type\n");
        return;
    }
    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + hdr.bodyLen, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto set_ipv4_addr_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto set_ipv4_addr_over;
        }
        count++;
    }

set_ipv4_addr_over:
    printf("Set ipv4 addr test over\n");
}

void
set_ipv6_addr
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;
    INT1 buf[5];

    memset(buf, 0, 5);
    printf("Set ipv6 addr start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto set_ipv6_addr_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_SET_IPV6_ADDR;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_SET_IPV6_ADDR) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    printf("0: IPv6 address\n1: IPv6 DNS\n");
    scanf("%s", buf);
    if (strcmp(buf, "0") == 0) {
        hdr.bodyLen = 163;
        *(pkt + pos) = 0; pos += 1;
        *(pkt + pos) = 1; pos += 1;
        for (tmp = 0; tmp < 16; tmp++) {
            *(pkt + pos + tmp) = 50 + tmp;
        }
        pos += 16;
        *(pkt + pos) = 2; pos += 1;
        for (tmp = 0; tmp < 16; tmp++) {
           *(pkt + pos + tmp) = 100 + tmp;
        }
        pos += 16;
    }
    else if (strcmp(buf, "1") == 0) {
        hdr.bodyLen = 161;
        *(pkt + pos) = 1; pos += 1;
        for (tmp = 0; tmp < 16; tmp++) {
            *(pkt + pos + tmp) = 150 + tmp;
            *(pkt + pos + 16 + tmp) = 200 + tmp;
        }
    }
    else {
        printf("Wrong message type\n");
        return;
    }
    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + hdr.bodyLen, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto set_ipv6_addr_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto set_ipv6_addr_over;
        }
        count++;
    }

set_ipv6_addr_over:
    printf("Set ipv6 addr test over\n");
}

void
set_device_info
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;
    INT1 buf[5];
    UINT2 tmpU2 = 100;

    memset(buf, 0, 5);
    printf("Set device info start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto set_device_info_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_SET_DEVICE_INFO;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_SET_DEVICE_INFO) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    printf("0: Device name\n1: Web port\n");
    scanf("%s", buf);
    if (strcmp(buf, "0") == 0) {
        hdr.bodyLen = 193;
        *(pkt + pos) = 0; pos += 1;
        strcpy((INT1*)(pkt + pos), "NEW DEV NAME");
    }
    else if (strcmp(buf, "1") == 0) {
        hdr.bodyLen = 131;
        *(pkt + pos) = 1; pos += 1;
        tmpU2 = DDP_HTONS(tmpU2);
        memcpy(pkt + pos, &tmpU2, 2);
    }
    else {
        printf("Wrong message type\n");
        return;
    }
    
    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + hdr.bodyLen, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto set_device_info_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto set_device_info_over;
        }
        count++;
    }

set_device_info_over:
    printf("Set device info test over\n");
}

void
set_date_time
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;
    INT1 buf[5];

    memset(buf, 0, 5);
    printf("Set date time start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto set_date_time_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_SET_SYS_DATE_TIME;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_SET_SYS_DATE_TIME) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    hdr.bodyLen = 228;
    strcpy((INT1*)(pkt + pos), "20130731010203"); pos += 16;
	*(pkt + pos) = 02; pos += 2;
    *(pkt + pos) = 0; pos += 1;
	*(pkt + pos) = 1; pos += 1;
	//strcpy(outBody + pos, "www.new.ntp.com"); pos += 64;
	*(pkt + pos) = 172; *(pkt + pos + 1) = 17; *(pkt + pos + 2) = 2; *(pkt + pos + 3) = 200; pos += 64;
	pos += 13;
	*(pkt + pos) = 1;
    
    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + hdr.bodyLen, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto set_date_time_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto set_date_time_over;
        }
        count++;
    }

set_date_time_over:
    printf("Set date time test over\n");
}

void
get_date_time
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;
    INT1 buf[5];

    memset(buf, 0, 5);
    printf("Get date time start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto get_date_time_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_GET_SYS_DATE_TIME;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_GET_SYS_DATE_TIME) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    hdr.bodyLen = 128;
    
    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + hdr.bodyLen, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto get_date_time_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto get_date_time_over;
        }
        count++;
    }

get_date_time_over:
    printf("Get date time test over\n");
}

void
reboot_delay
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;
    INT1 buf[5];

    memset(buf, 0, 5);
    printf("Reboot delay start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto reboot_delay_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_REBOOT;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_REBOOT) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    hdr.bodyLen = 132;
	tmp = 15;
	tmp = DDP_HTONL(tmp);
   	memcpy(pkt + pos, &tmp, sizeof(tmp)); pos += sizeof(tmp);

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + hdr.bodyLen, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto reboot_delay_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto reboot_delay_over;
        }
        count++;
    }

reboot_delay_over:
    printf("Reboot delay test over\n");
}

void
reset_delay
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;
    INT1 buf[5];

    memset(buf, 0, 5);
    printf("Reset delay start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto reset_delay_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_RESET;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_RESET) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    hdr.bodyLen = 132;
	tmp = 5;
	tmp = DDP_HTONL(tmp);
   	memcpy(pkt + pos, &tmp, sizeof(tmp)); pos += sizeof(tmp);

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + hdr.bodyLen, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto reset_delay_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto reset_delay_over;
        }
        count++;
    }

reset_delay_over:
    printf("Reset delay test over\n");
}

void
factory_reset_delay
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1200];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;
    INT1 buf[5];

    memset(buf, 0, 5);
    printf("Factory reset delay start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto factory_reset_delay_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_FACTORY_RESET;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = HDR_END_V4_OFFSET;

    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_FACTORY_RESET) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(pkt + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }

    hdr.bodyLen = 132;
	tmp = 8;
	tmp = DDP_HTONL(tmp);
   	memcpy(pkt + pos, &tmp, sizeof(tmp)); pos += sizeof(tmp);

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + hdr.bodyLen, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto factory_reset_delay_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1200, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto factory_reset_delay_over;
        }
        count++;
    }

factory_reset_delay_over:
    printf("Factory reset delay test over\n");
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
        printf("\nBind at port %d\n", g_sender.sin_port);
    }

    ddp_get_if_list();

    select_ifs();

    change_recv_ip();

    while (1) {
        printf("Select test item:\n");
        printf("  0: test over\n");
        printf("  1: change interface\n");
        printf("  2: change receiver IP\n");
        printf("  3: get snmp\n");
        printf("  4: set snmp\n");
        printf("  5: fw upgrade\n");
        printf("  6: cfg backup\n");
        printf("  7: cfg restore\n");
        printf("  8: set ipv4 addr\n");
        printf("  9: set ipv6 addr\n");
        printf(" 10: set device info\n");
        printf(" 11: get date/time\n");
        printf(" 12: set date/time\n");
        printf(" 13: reboot delay\n");
        printf(" 14: reset delay\n");
        printf(" 15: factory reset delay\n");

        scanf("%d", &item);

        switch (item) {
            case 0:
                goto switch_test_over;
                break;
            case 1:
                select_ifs();
                break;
            case 2:
                change_recv_ip();
                break;
            case 3:
                get_snmp();
                break;
            case 4:
                set_snmp();
                break;
            case 5:
                fw_upgrade();
                break;
            case 6:
                cfg_backup();
                break;
            case 7:
                cfg_restore();
                break;
            case 8:
                set_ipv4_addr();
                break;
            case 9:
                set_ipv6_addr();
                break;
            case 10:
                set_device_info();
                break;
            case 11:
                get_date_time();
                break;
            case 12:
                set_date_time();
                break;
            case 13:
                reboot_delay();
                break;
            case 14:
                reset_delay();
                break;
            case 15:
                factory_reset_delay();
                break;
            default:
                printf("item is not available\n");
                break;
        }
        sleep(1);
    }

switch_test_over:
    if (g_iSockfd) { close(g_iSockfd); g_iSockfd = 0; }
    return 0;
}
