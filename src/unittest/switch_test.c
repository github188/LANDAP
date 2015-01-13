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
#define ksRcvIP "172.17.2.172"
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
    char cTmp = 0;
    INT1 buf[40];

    memset(&g_recv, 0, sizeof(g_recv));
    g_recv.sin_family = AF_INET;
    g_recv.sin_port = htons(UDP_PORT_CLIENT);
    memset(buf, 0, sizeof(buf));

    printf("\nChange Rcv IP, curent<%s>. (y/n)?\n", ksRcvIP);
    scanf(" %c", &cTmp);

    if(cTmp == 'n')
    {
        if(inet_pton(AF_INET, ksRcvIP, &g_recv.sin_addr.s_addr) != 1) {
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
vlan_config
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1000];
    UINT1* outBody = NULL;
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("VLAN config start\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto vlan_config_over;
    }

    memset(&hdr, 0, sizeof(hdr));
    memset(&stTargetIP, 0, sizeof(struct in_addr));

    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.retCode = REQ_MODE_UNICAST;
    hdr.seq = 6;
    hdr.opcode = DDP_OP_VLAN_CFG;

    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_VLAN_CFG;

    /* Set target IP */
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    outBody = pkt + HDR_END_V4_OFFSET;
    pos = 0;
    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_VLAN_CFG) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(outBody + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(outBody + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }
    strcpy((INT1*)(outBody + pos), "INFDSS");
    pos += DDP_FIELD_LEN_VLAN_NAME;
    *(UINT4 *)(outBody + pos) = DDP_NTOHL(0x5);
    pos += DDP_FIELD_LEN_VLAN_ID;
    *(outBody + pos) = 0;
    pos += DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE;
    strcpy((INT1*)(outBody + pos), "10,13");
    //*(outBody + pos) = 2048;
    pos += DDP_FIELD_LEN_VLAN_TAG_PLIST;
    *(outBody + pos) = 1;
    pos += DDP_FIELD_LEN_VLAN_UNTAG_LIST_TYPE;
    memset((outBody + pos), 0, DDP_FIELD_LEN_VLAN_UNTAG_PLIST);
    pos += DDP_FIELD_LEN_VLAN_UNTAG_PLIST;
    pack_header(pkt, &hdr);

    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_VLAN_CFG, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto vlan_config_over;
    }

    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1000, 0);
        if (tmp <= 0) { sleep(1); }
        else {
            printf("vlan config response\n");
            for (pos = 0; pos < tmp; pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n");
            goto vlan_config_over;
        }
        count++;
    }

vlan_config_over:
    printf("vlan config test over\n");
}

void
aslan_config
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1000];
    UINT1* outBody = NULL;
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("aslan config start\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto aslan_config_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_ASV_CFG;
    hdr.retCode = REQ_MODE_UNICAST;

    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_ASV_CFG;
    memset(pkt, 0, sizeof(pkt));
    outBody = pkt + HDR_END_V4_OFFSET;
    pos = 0;
    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_ASV_CFG) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(outBody + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(outBody + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }
    *(UINT4 *)(outBody + pos) = DDP_NTOHL(0x0202);
    pos += DDP_FIELD_LEN_ASV_ID;
    *(UINT4 *)(outBody + pos) = DDP_NTOHL(0x0303);
    pos += DDP_FIELD_LEN_ASV_PRI;
    *(outBody + pos) = 1;
    pos += DDP_FIELD_LEN_ASV_STATE;
    *(outBody + pos) = 1;
    pos += DDP_FIELD_LEN_ASV_TAG_LIST_TYPE;
    memset(outBody + pos, 0x10, DDP_FIELD_LEN_ASV_TAG_PLIST);
    pos += DDP_FIELD_LEN_ASV_TAG_PLIST;

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_ASV_CFG, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto aslan_config_over;
    }

    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1000, 0);
        if (tmp <= 0) { sleep(1); }
        else {
            printf("aslan config response\n");
            for (pos = 0; pos < tmp; pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n");
            goto aslan_config_over;
        }
        count++;
    }

aslan_config_over:
    printf("aslan config test over\n");
}

void
get_asv
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1000];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("Get asv test start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto get_asv_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_ASV_GET;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_ASV_GET;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = 0;

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_ASV_GET, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto get_asv_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1000, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto get_asv_over;
        }
        count++;
    }

get_asv_over:
    printf("get asv test over\n");
    return;
}

void
user_defined_asv_mac
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1000];
    UINT1 mac[MAC_ADDRLEN]={0x0e, 0x1e, 0x34, 0x2d, 0x43,0x23};
    UINT1* outBody = NULL;
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("ASV user-defined MAC config start\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto asv_usr_mac_config_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_ASV_USR_MAC_CFG;
    hdr.retCode = REQ_MODE_UNICAST;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_ASV_USR_DEF_MAC_CFG;

    /* Ask for user input */
    tmp = 0;
    printf("\nAdd or del a MAC entry. 0 for add, 1 for delete: ");
    scanf("%d", &tmp);

    memset(pkt, 0, sizeof(pkt));
    outBody = pkt + HDR_END_V4_OFFSET;
    pos = 0;
    /* add authentication */
    if (ddp_proto_check_op_authen(DDP_OP_ASV_USR_MAC_CFG) == DDP_OP_AUTHEN_ON) {
        string_encode("admin", (INT1*)(outBody + pos), DDP_FIELD_LEN_USERNAME, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_USERNAME;
        string_encode("admin", (INT1*)(outBody + pos), DDP_FIELD_LEN_PASSWORD, DDP_ENC_BASE64);
        pos += DDP_FIELD_LEN_PASSWORD;
    }
    if((tmp == 0) || (tmp == 1))
    {
        *(outBody + pos) = tmp;
    }
    else
    {
        *(outBody + pos) = 0;
    }

    pos += DDP_FIELD_LEN_ASV_USR_DEF_MAC_SRV_TYPE;

    *(outBody + pos) = 0;
    pos += DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_TYPE;

    *(outBody + pos) = 1;
    pos += DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_CATG;

    tmp = 0;
    printf("\nCustom mac (0) or Auto (1): ");
    scanf("%d", &tmp);

    if(tmp == 0)
    {
        count = 0;
        while(count < 6 && count >= 0)
        {
            printf("\nEnter HEX(0x..) for mac[%d]: ", count);
            scanf("%x", &tmp);
            mac[count] = tmp;
            count++;
        }
    }

    printf("\nmac[0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x]\n",
            mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    memcpy(outBody + pos, mac, MAC_ADDRLEN);
    pos += DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR;

    strcpy((INT1*) outBody+pos, "Default Desc");

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_ASV_USR_DEF_MAC_CFG, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto asv_usr_mac_config_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1000, 0);
        if (tmp <= 0) { sleep(1); }
        else {
            printf("asv usr defined mac config response, size<%d>\n", tmp);
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n");
            goto asv_usr_mac_config_over;
        }
        count++;
    }

asv_usr_mac_config_over:
    printf("ASV user-defined MAC config over\n");
    return;
}

void
get_port_number
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1000];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("Get port number start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto get_port_number_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_GET_SYS_PORT_NUM;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_GET_SYS_PORT_NUM;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = 0;

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_GET_SYS_PORT_NUM, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto get_port_number_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1000, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto get_port_number_over;
        }
        count++;
    }

get_port_number_over:
    printf("get port number test over\n");
    return;
}

void
get_user_defined_asv_mac
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1000];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("Get asv usr define mac start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto get_user_defined_asv_mac_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_ASV_USR_MAC_GET;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_ASV_USR_DEF_MAC_GET;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = 0;

    pack_header(pkt, &hdr);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_ASV_USR_DEF_MAC_GET, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto get_user_defined_asv_mac_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1000, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto get_user_defined_asv_mac_over;
        }
        count++;
    }

get_user_defined_asv_mac_over:
    printf("Get asv usr define mac over\n");
    return;
}

void
get_vlan_list
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1000];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;

    printf("Get vlan list start\n");
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto get_vlan_list_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_VLAN_GET;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_VLAN_GET;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = 0;
    pack_header(pkt, &hdr);
    /* body content: type = 0
     */
    *(pkt + HDR_END_V4_OFFSET) = 0;
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_VLAN_GET, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto get_vlan_list_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1000, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto get_vlan_list_over;
        }
        count++;
    }

get_vlan_list_over:
    printf("get vlan list test over\n");
    return;
}

void
get_vlan_info
(
    void
)
{
    struct ddp_header hdr;
    UINT1 pkt[1000];
    INT4 pos = 0;
    INT4 count = 0;
    INT4 tmp = 0;
    struct in_addr stTargetIP;
    UINT4 vlan_id = 1;

    printf("Get vlan %d info start\n", vlan_id);
    printf("\n==============================\n");
    if (g_ifSelect == NULL) {
        printf("No interface is selected. Please select item 1 to assign interface\n");
        goto get_vlan_info_over;
    }
    memset(&hdr, 0, sizeof(hdr));
    hdr.ipVer = IPV4_FLAG;
    hdr.identifier = IPV4_REQ;
    hdr.seq = 7;
    hdr.opcode = DDP_OP_VLAN_GET;
    hdr.retCode = REQ_MODE_UNICAST;

//    memcpy(hdr.macAddr, g_ifSelect->macAddr, MAC_ADDRLEN);
    memcpy(hdr.macAddr, gau1TargetMac, MAC_ADDRLEN);
    tmp = IPV4_ADDRLEN;
    ddp_platform_get_field(g_ifSelect, DDP_FIELD_DEVICE_IP_ADDR, pkt + HDR_IP_OFFSET, &tmp);
    hdr.protoVer = DDP_PROTO_V2;
    hdr.bodyLen = DDP_REQ_LEN_VLAN_GET;
    /* Set target IP */
    memset(&stTargetIP, 0, sizeof(struct in_addr));
    if(inet_aton(ksRcvIP, &stTargetIP) != 0)
    {
        memcpy(hdr.ipAddr.ipv4Addr, &stTargetIP.s_addr, MAC_ADDRLEN);
    }

    memset(pkt, 0, sizeof(pkt));
    pos = 0;
    pack_header(pkt, &hdr);
    /* body content: type = 1, vlan_id = 
     */
    *(pkt + HDR_END_V4_OFFSET) = 1;
    vlan_id = DDP_HTONL(vlan_id);
    memcpy(pkt + HDR_END_V4_OFFSET + 1, &vlan_id, DDP_FIELD_LEN_VLAN_ID);
    if (sendto(g_iSockfd, pkt, HDR_END_V4_OFFSET + DDP_REQ_LEN_VLAN_GET, 0, (struct sockaddr*)&g_recv, sizeof(g_recv)) < 0) {
        printf("sendto err %d\n", errno);
        goto get_vlan_info_over;
    }

    count = 0;
    memset(pkt, 0, sizeof(pkt));
    while (count < 10) {
        tmp = recv(g_iSockfd, pkt, 1000, 0);
        if (tmp <= 0) {sleep(1); }
        else {
            printf("Received reply packet:\n");
            for (pos = 0; pos < tmp && pos < sizeof(pkt)/sizeof(UINT1); pos++) {
                printf("%02X ", pkt[pos]);
                if ((pos + 1) % 16 == 0) printf("\n");
            }
            printf("\n==============================\n");
            goto get_vlan_info_over;
        }
        count++;
    }

get_vlan_info_over:
    printf("get vlan info %d test over\n", DDP_NTOHL(vlan_id));
    return;
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
        printf("  3: VLAN config\n");
        printf("  4: AS VLAN config\n");
        printf("  5; get ASV mac info\n");
        printf("  6; user-defined mac AS VLAN\n");
        printf("  7: get user-defined mac AS VLAN\n");
        printf("  8; get device port number\n");
        printf("  9: get vlan list\n");
        printf(" 10: get vlan info\n");

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
                vlan_config();
                break;
            case 4:
                aslan_config();
                break;
            case 5:
                get_asv();
                break;
            case 6:
            	user_defined_asv_mac();
                break;
            case 7:
                get_user_defined_asv_mac();
                break;
            case 8:
                get_port_number();
                break;
            case 9:
                get_vlan_list();
                break;
            case 10:
                get_vlan_info();
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
