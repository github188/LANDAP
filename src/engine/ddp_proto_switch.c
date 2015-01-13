/********************************
 * ddp_proto_switch.c
 *
 *  Created on: 2012/9/5
 *      Author: Nathan Chen
 *
 *******************************/

/************************************************
 * Ref: DLINK Discover Protocol
 * - VLAN configuration.
 * - Auto Surveillance VLAN configuration.
 * - User Define MAC for Auto Surveillance VLAN.
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "ddp.h"
#include "ddp_platform.h"

/* Array of Neighbor Information List: 1-based array  */
/* Each list consists of entries that connect the same switch port. */
/* The list is initialized at init stage. */
/* The list is port-based, so list size depends on number of ports. */
/*
 * E.g.
 * One switch has following neighbor nodes.
 *
 * port 1: node a, node b
 * port 2: node c
 *
 * Then the switch has an array of database list
 *
 * list[1]: stDDP_Neighb_Node a, stDDP_Neighb_Node b
 * list[2]: stDDP_Neighb_Node c
 *
 *
 * */
static struct stDDP_Neighb_List* gpstDDP_Neighb_List;

/* i4fnDDP_proto_switch_pack_reply_msg
 *   function to pack up replay message
 *
 *   pstDDP_Hdr_out : pointer to the ddp output header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan cfg msg structure
 *   pstDDP_ErrStatus_out: pointer to error status
 *   pstDDP_Msg_pkt: pointer to packet
 *   pu1outMsg: pointer to output message
 *   i4outMsgLen: length of output message
 *   pu1outMsg: pointer to the body of message
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_pack_reply_msg(
    struct ddp_header* pstDDP_Hdr_out,
    struct ddp_message* pstDDP_Msg_in,
    struct ddp_exe_status* pstDDP_ErrStatus_out,
    struct ddp_message* pstDDP_Msg_pkt,
    UINT1* pu1outMsg,
    INT4 i4outMsgLen,
    UINT1* pu1outBody
);

/* i4fnDDP_proto_switch_vlan_cfg_check_header
 *   function to check the content of vlan cfg header
 *
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan cfg msg structure
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_vlan_cfg_check_header(
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
;

/* i4fnDDP_proto_switch_asv_cfg_check_header
 *   function to check the content of asv cfg header
 *
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan cfg msg structure
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_asv_cfg_check_header(
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
);

/* i4fnDDP_proto_switch_asv_usr_mac_cfg_check_header
 *   function to check the content of asv user defined mac cfg header
 *
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan cfg msg structure
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_asv_usr_mac_cfg_check_header(
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
);

/* i4fnDDP_proto_switch_asv_usr_mac_get_check_header
 *   function to check the content of asv user defined mac get header
 *
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input usr asv mac get msg structure
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_asv_usr_mac_get_check_header(
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
);

/* i4fnDDP_proto_switch_NeighbDB_delete_all
 *   function to delete neighbor information list starting from the given node.
 *
 *   pstDDP_Neighb_Node : pointer to the starting node.
 *   This function will do the following thins:
 *   1. free the node on the list.
 *   2. set the pointer to null.
 *
 *   return : DDP_ALL_FIELD_SUCCESS -> success, others -> error
 */
INT4 i4fnDDP_proto_switch_NeighbDB_delete_all(struct stDDP_Neighb_Node* pstDDP_Neighb_Node);

/* i4fnDDP_proto_switch_get_port_number (OP: 0xD7 (215))
 *   function to process get device port number packet and
 *               send through the incoming interfaces
 *
 *   pstDDP_ifs : pointer to the ddp interface structure
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input get msg structure
 *
 *   return : 0 -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_get_port_number
(
    struct ddp_interface* pstDDP_ifs,
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
);

/* i4fnDDP_proto_switch_get_port_number_check_header
 *   function to check the content of get port number header
 *
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input get port number msg structure
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_get_port_number_check_header(
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
);

/* i4fnDDP_proto_switch_get_port_number (OP: 0xD7 (215))
 *   function to process get device port number packet and
 *               send through the incoming interfaces
 *
 *   pstDDP_ifs : pointer to the ddp interface structure
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input asv mac get msg structure
 *
 *   return : 0 -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_asv_usr_mac_get
(
    struct ddp_interface* pstDDP_ifs,
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
);

/**********************
 * Constant
 **********************/
#define kDDP_VLAN_CFG_STR_LEN (DDP_FIELD_LEN_VLAN_NAME + 1)
#define kDDP_VLAN_CFG_PLIST_LEN (DDP_FIELD_LEN_VLAN_TAG_PLIST + 1)
#define kDDP_ASV_CFG_STR_LEN (DDP_FIELD_LEN_ASV_NAME + 1)
#define kDDP_ASV_CFG_PLIST_LEN (DDP_FIELD_LEN_ASV_TAG_PLIST + 1)
#define kDDP_ASV_USR_MAC_CFG_DESC_LEN (DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC + 1)

/* i4fnDDP_proto_switch_Init
 *   Initial function for switch api, called by vfnDDP_proto_switch_Init
 *   - Initialize database for neighbor information maintained by switch.
 *
 */
INT4 i4fnDDP_proto_switch_Init(void)
{
    if( i4fnDDP_proto_switch_NeighbDB_Init() != DDP_RET_SWITCH_OK){
        return DDP_RET_SWITCH_ERROR_INIT;
    }

    DDP_DEBUG_SWITCH(" %s: Switch init ok. \n", __FUNCTION__);

    return DDP_RET_SWITCH_OK;
}

/* i4fnDDP_proto_switch_NeighbDB_Init
 *   function to initialize neighbor information database
 *
 *   gpstDDP_Neighb_List : pointer to the db list, current glob list: gstDDP_Neighb_List
 *   pstNewNode : pointer to the new node to insert.
 *
 *   return : DDP_RET_SWITCH_OK -> success, others -> error
 */
INT4 i4fnDDP_proto_switch_NeighbDB_Init(void)
{
    if(gpstDDP_Neighb_List != NULL){
        /* Abnormal case, log here */
       gpstDDP_Neighb_List = NULL;
       DDP_DEBUG_SWITCH("\n%s: line<%d> Init error.\n", __FUNCTION__, __LINE__);
    }

    gpstDDP_Neighb_List = malloc((kDDP_SWITCH_PORT_NUMBER+1)*sizeof(struct stDDP_Neighb_List));

    if(gpstDDP_Neighb_List == NULL){
        return DDP_RET_SWITCH_ERROR_INIT_DB;
    }

    memset((INT1 *)gpstDDP_Neighb_List, 0, (kDDP_SWITCH_PORT_NUMBER+1)*sizeof(struct stDDP_Neighb_List));

    return DDP_RET_SWITCH_OK;
}

/* i4fnDDP_proto_switch_Fill_QueryNeighb_Pkt
 *   function to fill the reply message for Query Neighbor
 *
 *   The reply neighbor info is port-based.
 *   Each packet contains entries which belongs to the same switch port.
 *   The maximum entries in one packet is defined as kDDP_SWITCH_MAX_ENTRY_PER_PACKET.
 *
 *   This function access the global list gpstDDP_Neighb_List to fill up the message
 *
 *   pi1Msg : pointer to output message.
 *   pu2len : message size, and it will be updated to actual size.
 *
 *   return : DDP_RET_SWITCH_OK -> success, others -> error
 */
INT4 i4fnDDP_proto_switch_Fill_QueryNeighb_Pkt(
        struct ddp_interface* pstDDP_IF,
        INT1* pi1Msg, UINT2* pu2len)
{
    UINT4 u4IfIdx = 0;
    UINT4 u4Offset = 0;
    UINT4 u4TmpLen = 0;
    UINT4 u4Counter = 0;
    struct stDDP_Neighb_Node* pstDDP_Neighb_CurNode = NULL;


    if( (pi1Msg == NULL)
        || (pstDDP_IF == NULL)
        || (gpstDDP_Neighb_List == NULL)
        || (pu2len == NULL)){
       DDP_DEBUG_SWITCH("\n%s: line<%d> NULL pointer.\n", __FUNCTION__, __LINE__);
       return DDP_RET_SWITCH_FAIL;
    }

    u4IfIdx = pstDDP_IF->ifindex;

    if(u4IfIdx > kDDP_SWITCH_PORT_NUMBER){
        DDP_DEBUG_SWITCH("\n%s: line<%d> Invalid port number<%d>, max <%d>\n",
                  __FUNCTION__, __LINE__, pstDDP_IF->ifindex, kDDP_SWITCH_PORT_NUMBER);
        return DDP_RET_SWITCH_ERROR_PARAMETER;
    }

    if(*pu2len > DDP_REPLY_LEN_QUERY_NEIGHBOR_INFO+1){
        DDP_DEBUG_SWITCH("\n%s: line<%d> Invalid reply length<%d>, max <%d>\n",
                  __FUNCTION__, __LINE__, *pu2len, DDP_REPLY_LEN_QUERY_NEIGHBOR_INFO+1);
        return DDP_RET_SWITCH_ERROR_PARAMETER;
    }

    DDP_DEBUG_SWITCH("\n==== %s: line<%d>\n Generate Reply Pkt for port<%d>, len<%d>\n",
                      __FUNCTION__, __LINE__, u4IfIdx, *pu2len);

    memset(pi1Msg, 0, *pu2len);

    if(gpstDDP_Neighb_List[u4IfIdx].u4TotalNodes > kDDP_SWITCH_MAX_ENTRY_PER_PACKET){
        DDP_DEBUG_SWITCH("\n%s: line<%d> Total entry<%d> exceeds limit<%d>, info will be truncated.\n",
                  __FUNCTION__, __LINE__,
                  gpstDDP_Neighb_List[u4IfIdx].u4TotalNodes,
                  kDDP_SWITCH_MAX_ENTRY_PER_PACKET);
    }

    /* Fill in the entries. */
    pstDDP_Neighb_CurNode = gpstDDP_Neighb_List[u4IfIdx].pstFirstNode;

    u4Offset = 0;
    u4TmpLen = 0;

    /* Number of entries in this packet. */
    u4TmpLen = DDP_FIELD_LEN_QUERY_NBR_ENTRY_NUM;

    *(pi1Msg+u4Offset) = (INT1)gpstDDP_Neighb_List[u4IfIdx].u4TotalNodes;
    u4Offset+= u4TmpLen;

    for(u4Counter = 0; u4Counter < kDDP_SWITCH_MAX_ENTRY_PER_PACKET;u4Counter++){
        if(pstDDP_Neighb_CurNode == NULL){
            DDP_DEBUG_SWITCH("\t line<%d> Filled in <%d> entry.\n\n",
                             __LINE__, u4Counter);
            break;
        }

        /* Product Name */
        u4TmpLen = DDP_FIELD_LEN_PRODUCT_NAME;
        if(u4Offset +u4TmpLen > *pu2len){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                      __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Counter+1, *pu2len);
            break;
        }

        memcpy(pi1Msg+u4Offset, pstDDP_Neighb_CurNode->stDDP_Neighb_Entry.ai1Name, u4TmpLen);
        u4Offset+= u4TmpLen;

        /* Product Category */
        u4TmpLen = DDP_FIELD_LEN_PRODUCT_CATEGORY;
        if(u4Offset +u4TmpLen > *pu2len){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                      __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Counter+1, *pu2len);
            break;
        }

        memcpy(pi1Msg+u4Offset, pstDDP_Neighb_CurNode->stDDP_Neighb_Entry.ai1Category, u4TmpLen);
        u4Offset+= u4TmpLen;

        /* Product Version */
        u4TmpLen = DDP_FIELD_LEN_VERSION;
        if(u4Offset +u4TmpLen > *pu2len){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                      __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Counter+1, *pu2len);
            break;
        }

        memcpy(pi1Msg+u4Offset, pstDDP_Neighb_CurNode->stDDP_Neighb_Entry.ai1FwVersion, u4TmpLen);
        u4Offset+= u4TmpLen;

        /* MAC Address */
        u4TmpLen = DDP_FIELD_LEN_MAC_ADDR;
        if(u4Offset +u4TmpLen > *pu2len){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                      __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Counter+1, *pu2len);
            break;
        }

        memcpy(pi1Msg+u4Offset, pstDDP_Neighb_CurNode->stDDP_Neighb_Entry.ai1MAC, u4TmpLen);
        u4Offset+= u4TmpLen;

        /* System Name */
        u4TmpLen = DDP_FIELD_LEN_SYSTEM_NAME;
        if(u4Offset +u4TmpLen > *pu2len){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                      __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Counter+1, *pu2len);
            break;
        }

        memcpy(pi1Msg+u4Offset, pstDDP_Neighb_CurNode->stDDP_Neighb_Entry.ai1SysName, u4TmpLen);
        u4Offset+= u4TmpLen;

        /* Destination IPv4 */
        u4TmpLen = DDP_FIELD_LEN_DEVICE_IP_ADDR;
        if(u4Offset +u4TmpLen > *pu2len){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                      __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Counter+1, *pu2len);
            break;
        }

        memcpy(pi1Msg+u4Offset, pstDDP_Neighb_CurNode->stDDP_Neighb_Entry.ai1IPv4, u4TmpLen);
        u4Offset+= u4TmpLen;

        /* Subnet MASK */
        u4TmpLen = DDP_FIELD_LEN_SUBNET_MASK;
        if(u4Offset +u4TmpLen > *pu2len){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                      __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Counter+1, *pu2len);
            break;
        }

        memcpy(pi1Msg+u4Offset, pstDDP_Neighb_CurNode->stDDP_Neighb_Entry.ai1Mask, u4TmpLen);
        u4Offset+= u4TmpLen;

        /* Destination IPv6 */
        u4TmpLen = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        if(u4Offset +u4TmpLen > *pu2len){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                      __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Counter+1, *pu2len);
            break;
        }

        memcpy(pi1Msg+u4Offset, pstDDP_Neighb_CurNode->stDDP_Neighb_Entry.ai1IPv6, u4TmpLen);
        u4Offset+= u4TmpLen;

        /* Port number */
        u4TmpLen = DDP_FIELD_LEN_INTERFACE_NUMBER;
        if(u4Offset +u4TmpLen > *pu2len){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                      __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Counter+1, *pu2len);
            break;
        }

        memcpy(pi1Msg+u4Offset, pstDDP_Neighb_CurNode->stDDP_Neighb_Entry.ai1PortNumBmp, u4TmpLen);
        u4Offset+= u4TmpLen;

        pstDDP_Neighb_CurNode = pstDDP_Neighb_CurNode->pstNextNode;
    }

    if(u4Offset < *pu2len){
    	*pu2len = u4Offset;
        DDP_DEBUG_SWITCH("\t line<%d>Total entry<%d>, updated size<%d>\n\n",
                          __LINE__, u4Counter+1, *pu2len);
    }

    return DDP_RET_SWITCH_OK;
}

/* i4fnDDP_proto_switch_NeighbDB_Update_AllNode_TTL
 *   function to update nodes' TTL and purge phased-out nodes.
 *   When node's TTL equals to zero, the entry is phased-out and should be purged.
 *   Every time switch sends a general report, it also use this function check
 *   and update nodes' TTL.
 *
 *   And the db array list consists of array[1] ~ array[max port number]
 *   each array has entries that belongs to the same switch port number.
 *
 *
 *   return : DDP_RET_SWITCH_OK -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_NeighbDB_Update_AllNode_TTL(void)
{
    UINT1 u1RetVal = 0;
    UINT4 u4ListIdx = 0;
    UINT4 u4Counter = 0;
    struct stDDP_Neighb_Node* pstNeighb_CurNode = NULL;
    struct stDDP_Neighb_Node* pstNeighb_TmpNode = NULL;

    if(gpstDDP_Neighb_List == NULL)
    {
        DDP_DEBUG_SWITCH("\n%s: line<%d> NULL pointer. It's allocated at init stage.\n", __FUNCTION__, __LINE__);
        return DDP_RET_SWITCH_ERROR_UPDATE_TTL;
    }

    /* Check all list to see whether the node's TTL equals to 0, delete it if any */
    /* Also decrement TTL by 1. */
    for(u4ListIdx = 1; u4ListIdx <= kDDP_SWITCH_PORT_NUMBER; u4ListIdx++){
        pstNeighb_CurNode = gpstDDP_Neighb_List[u4ListIdx].pstFirstNode;
        u4Counter = 0;
        while(pstNeighb_CurNode != NULL){
            u4Counter++;
            pstNeighb_TmpNode = pstNeighb_CurNode->pstNextNode;

            /* Purge the phased out node. */
            if(pstNeighb_CurNode->u2TTL == 0 || pstNeighb_CurNode->u2TTL > kDDP_SWITCH_UPDATE_TTL){
            	DDP_DEBUG_SWITCH("\n====\n %s: line<%d>\n Found phased-out entry<0x%x> at list<%d> idx<%d> TTL<%d>, purge it.\n====\n",
            	                        __FUNCTION__, __LINE__, (unsigned int)pstNeighb_CurNode, u4ListIdx, u4Counter, pstNeighb_CurNode->u2TTL);

                u1RetVal = i4fnDDP_proto_switch_NeighbDB_Purge_FromDB(pstNeighb_CurNode->stDDP_Neighb_Entry.ai1MAC);

                if(u1RetVal != DDP_RET_SWITCH_OK){
                    DDP_DEBUG_SWITCH("\n==== %s: line<%d>\n Failed to purge node whose MAC[",
                                     __FUNCTION__, __LINE__);

                    for(u1RetVal=0; u1RetVal < MAC_ADDRLEN; u1RetVal++)
                        printf("%02x ", pstNeighb_CurNode->stDDP_Neighb_Entry.ai1MAC[u1RetVal]);

                    printf("]\n====\n");
                }
            }
            else{
                DDP_DEBUG_SWITCH("\n==== %s: line<%d>\n Node<%d> on list<%d>, TTL<%d>, decrement\n====\n\n",
                        __FUNCTION__, __LINE__,
                        u4Counter, u4ListIdx, pstNeighb_CurNode->u2TTL);
                (pstNeighb_CurNode->u2TTL)--;
            }

            pstNeighb_CurNode = pstNeighb_TmpNode;
        }
    }

    return DDP_RET_SWITCH_OK;
}

/* i4fnDDP_proto_switch_NeighbDB_Proc_Pkt
 *   function to process general report pkt from neighbor
 *
 *   pstDDP_ifs: pointer to input interface
 *   pstDDP_inHdr: pointer to input pkt header
 *   pstDDP_inMsg: pointer th input pkt
 *
 *   return : DDP_RET_SWITCH_OK -> success, others -> error
 */
INT4 i4fnDDP_proto_switch_NeighbDB_Proc_Pkt
(
    struct ddp_interface* pstDDP_ifs,
    struct ddp_header* pstDDP_inHdr,
    struct ddp_message* pstDDP_inMsg
)
{
    INT4  i4RetVal = 0;
    INT1* pi1PktBody = NULL;

    struct stDDP_neighbor_entry stNewEntry;
    struct stDDP_general_report* pstDDP_GeneralReport = NULL;

    if( (pstDDP_ifs == NULL)
        || (pstDDP_inHdr == NULL)
        || (pstDDP_inMsg == NULL)){
       DDP_DEBUG_SWITCH("\n%s: line<%d> NULL pointer.\n", __FUNCTION__, __LINE__);
       return DDP_RET_SWITCH_FAIL;
    }

    memset((INT1*)&stNewEntry, 0, sizeof(struct stDDP_neighbor_entry));

    if (pstDDP_inHdr->ipVer == IPV4_FLAG) {
        if (pstDDP_inHdr->bodyLen > 0) {
            pi1PktBody = (INT1*)(pstDDP_inMsg->payload + HDR_END_V4_OFFSET);
        }
    } else if (pstDDP_inHdr->ipVer == IPV6_FLAG){
        if (pstDDP_inHdr->bodyLen > 0) {
            pi1PktBody = (INT1*)(pstDDP_inMsg->payload + HDR_END_V6_OFFSET);
        }
    }

    /* Create a new neighbor entry from packet. */
    pstDDP_GeneralReport = (struct stDDP_general_report*)pi1PktBody;

    /* Product Name */
    memcpy(stNewEntry.ai1Name, pstDDP_GeneralReport->ai1ProductName, DDP_FIELD_LEN_PRODUCT_NAME);

    /* Product Category */
    memcpy(stNewEntry.ai1Category, pstDDP_GeneralReport->ai1Category, DDP_FIELD_LEN_PRODUCT_CATEGORY);

    /* Firmware Version */
    memcpy(stNewEntry.ai1FwVersion, pstDDP_GeneralReport->ai1FwVersion, DDP_FIELD_LEN_VERSION);

    /* MAC address */
    memcpy(stNewEntry.ai1MAC, pstDDP_GeneralReport->ai1MAC, DDP_FIELD_LEN_MAC_ADDR);

    /* System Name */
    memcpy(stNewEntry.ai1SysName, pstDDP_GeneralReport->ai1SysName, DDP_FIELD_LEN_SYSTEM_NAME);

    /* Device ipv4 */
    memcpy(stNewEntry.ai1IPv4, pstDDP_GeneralReport->ai1IPv4, DDP_FIELD_LEN_DEVICE_IP_ADDR);

    /* Sub-net mask */
    memcpy(stNewEntry.ai1Mask, pstDDP_GeneralReport->ai1Mask, DDP_FIELD_LEN_SUBNET_MASK);

    /* Device ipv6 */
    memcpy(stNewEntry.ai1IPv6, pstDDP_GeneralReport->ai1IPv6, DDP_FIELD_LEN_DEVICE_IPV6_ADDR);

    /* switch port number which connects to the device */
    memcpy(stNewEntry.ai1PortNumBmp, pstDDP_GeneralReport->ai1PortNumBmp, DDP_FIELD_LEN_INTERFACE_NUMBER);

    /* Insert the new node to the list. */
    i4RetVal = i4fnDDP_proto_switch_NeighbDB_Insert(pstDDP_ifs, &stNewEntry);
    if(i4RetVal != DDP_RET_SWITCH_OK){
        DDP_DEBUG_SWITCH("\n%s: line<%d> Failed to insert node. Error<%d>\n",
                  __FUNCTION__, __LINE__, i4RetVal);
        return i4RetVal;
    }

    return DDP_RET_SWITCH_OK;
}

/* i4fnDDP_proto_switch_NeighbDB_Purge_FromDB
 *   function to purge one specific node from all the neighbor info list db
 *   Currently, no sorting.
 *   And the db array list consists of array[1] ~ array[max port number]
 *   each array has entries that belongs to the same switch port number.
 *
 *   pai1MAC: target MAC
 *
 *   return : DDP_RET_SWITCH_OK -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_NeighbDB_Purge_FromDB(INT1* pai1MAC){
    struct stDDP_Neighb_Node* pstNeighb_TmpNode = NULL;
    struct stDDP_Neighb_Node* pstNeighb_CurNode = NULL;
    UINT4 u4ListIdx = 0;
    UINT4 u4Counter = 0;

    if(gpstDDP_Neighb_List == NULL){
        DDP_DEBUG_SWITCH("\n%s: line<%d> Please Init Switch DB first.\n", __FUNCTION__, __LINE__);
        return DDP_RET_SWITCH_ERROR_INIT_DB;
    }

    if(pai1MAC == NULL)
    {
        DDP_DEBUG_SWITCH("\n%s: line<%d> NULL pointer.\n", __FUNCTION__, __LINE__);
        return DDP_RET_SWITCH_FAIL;
    }

    /* Check all list to see whether the node already exists, delete it if any */
    for(u4ListIdx = 1; u4ListIdx <= kDDP_SWITCH_PORT_NUMBER; u4ListIdx++){
        pstNeighb_CurNode = gpstDDP_Neighb_List[u4ListIdx].pstFirstNode;
        u4Counter = 0;
        while(pstNeighb_CurNode != NULL){
            u4Counter++;
            pstNeighb_TmpNode = pstNeighb_CurNode->pstNextNode;
            if(memcmp(pstNeighb_CurNode->stDDP_Neighb_Entry.ai1MAC,
                    pai1MAC, MAC_ADDRLEN) == 0){

                /* The only one node will be purged, and the list will be empty. */
                if( (pstNeighb_CurNode->pstPreNode == NULL)
                    && (pstNeighb_CurNode->pstNextNode == NULL) ){

                    /* The only node will be purged, set pstFirstNode and pstTailNode. */
                    if(gpstDDP_Neighb_List[u4ListIdx].pstFirstNode != NULL){
                        gpstDDP_Neighb_List[u4ListIdx].pstFirstNode->pstNextNode = NULL;
                        gpstDDP_Neighb_List[u4ListIdx].pstFirstNode->pstPreNode = NULL;
                    }

                    if(gpstDDP_Neighb_List[u4ListIdx].pstTailNode != NULL){
                        gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstNextNode = NULL;
                        gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstPreNode = NULL;
                    }

                    gpstDDP_Neighb_List[u4ListIdx].pstFirstNode = NULL;
                    gpstDDP_Neighb_List[u4ListIdx].pstTailNode = NULL;
                    gpstDDP_Neighb_List[u4ListIdx].u4TotalNodes = 0;

                    goto PURGE;
                }

                if(gpstDDP_Neighb_List[u4ListIdx].pstFirstNode != NULL){

                    /* Handle pstFirstNode if the first node will be purged. */
                    if(pstNeighb_CurNode == gpstDDP_Neighb_List[u4ListIdx].pstFirstNode){
                        DDP_DEBUG_SWITCH("\t line<%d> First node<0x%x> will be purged.\n",
                                         __LINE__, (unsigned int)gpstDDP_Neighb_List[u4ListIdx].pstFirstNode);

                        gpstDDP_Neighb_List[u4ListIdx].pstFirstNode->pstPreNode = NULL;
                        gpstDDP_Neighb_List[u4ListIdx].pstFirstNode = pstNeighb_CurNode->pstNextNode;
                        if(gpstDDP_Neighb_List[u4ListIdx].pstFirstNode){
                            gpstDDP_Neighb_List[u4ListIdx].pstFirstNode->pstPreNode = NULL;
                        }
                    }

                    /* Handle pstFirstNode if the second node is purged */
                    if(pstNeighb_CurNode->pstPreNode == gpstDDP_Neighb_List[u4ListIdx].pstFirstNode){
                        DDP_DEBUG_SWITCH("\t line<%d> Second node<0x%x> will be purged.\n",
                                         __LINE__, (unsigned int)pstNeighb_CurNode);

                        gpstDDP_Neighb_List[u4ListIdx].pstFirstNode->pstNextNode = pstNeighb_CurNode->pstNextNode;
                        if(pstNeighb_CurNode->pstNextNode != NULL){
                            (pstNeighb_CurNode->pstNextNode)->pstPreNode = gpstDDP_Neighb_List[u4ListIdx].pstFirstNode;
                        }

                    }
                }

                /* Handle pstTailNode if the tail node will be purged. */
                if(gpstDDP_Neighb_List[u4ListIdx].pstTailNode != NULL){
                    if(pstNeighb_CurNode == gpstDDP_Neighb_List[u4ListIdx].pstTailNode){
                        DDP_DEBUG_SWITCH("\t line<%d> Tail node<0x%x> will be purged.\n",
                                         __LINE__, (unsigned int)gpstDDP_Neighb_List[u4ListIdx].pstTailNode);

                        gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstNextNode = NULL;
                        gpstDDP_Neighb_List[u4ListIdx].pstTailNode = gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstPreNode;
                        if(gpstDDP_Neighb_List[u4ListIdx].pstTailNode != NULL){
                            gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstNextNode = NULL;
                        }
                    }

                    /* Handle pstTailNode if the node before tail node is purged */
                    if(pstNeighb_CurNode->pstNextNode == gpstDDP_Neighb_List[u4ListIdx].pstTailNode){
                        DDP_DEBUG_SWITCH("\t line<%d> node<0x%x> before tail node will be purged.\n",
                                         __LINE__, (unsigned int)pstNeighb_CurNode);

                        gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstPreNode = pstNeighb_CurNode->pstPreNode;
                        if(gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstPreNode != NULL){
                            (gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstPreNode)->pstNextNode = gpstDDP_Neighb_List[u4ListIdx].pstTailNode;
                        }
                    }
                }

                /* For general nodes */
                /* Attach next node to previous node because current node will be pruged. */

                /* Attach next node to previous node */
                if(pstNeighb_CurNode->pstNextNode != NULL){
                   (pstNeighb_CurNode->pstNextNode)->pstPreNode = pstNeighb_CurNode->pstPreNode;
                }

                /* Attach previous node to next node */
                if(pstNeighb_CurNode->pstPreNode != NULL){
                    (pstNeighb_CurNode->pstPreNode)->pstNextNode = pstNeighb_CurNode->pstNextNode;
                }

            PURGE:
                if(pstNeighb_CurNode != NULL){
                    memset(pstNeighb_CurNode, 0, sizeof(struct stDDP_Neighb_Node));

                    free(pstNeighb_CurNode);
                    pstNeighb_CurNode = NULL;

                    if(gpstDDP_Neighb_List[u4ListIdx].u4TotalNodes > 0){
                        (gpstDDP_Neighb_List[u4ListIdx].u4TotalNodes--);
                    }
                }
                else{
                    DDP_DEBUG_SWITCH("\t line<%d> Attempt to purge NULL node !!!!! \n",
                                     __LINE__);
                    return DDP_RET_SWITCH_ERROR_PURGE_NODE;
                }
            } /* if current MAC exists */

            pstNeighb_CurNode = pstNeighb_TmpNode;
        } /* while current node is not NULL */
    }

    return DDP_RET_SWITCH_OK;
}


/* i4fnDDP_proto_switch_NeighbDB_Insert
 *   function to insert one entry to the end of neighbor info list
 *   Currently, no sorting.
 *   And the db array list consists of array[1] ~ array[max port number]
 *   each array has entries that belongs to the same switch port number.
 *
 *   pstDDP_ifs: pointer to the switch interface where the node is received from.
 *   pstNewEntry : pointer to the new entry to insert.
 *
 *   return : DDP_RET_SWITCH_OK -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_NeighbDB_Insert
(
    struct ddp_interface* pstDDP_ifs,
    struct stDDP_neighbor_entry* pstNewEntry
)
{
    struct stDDP_Neighb_Node* pstNeighb_NewNode = NULL;
    struct stDDP_Neighb_Node* pstNeighb_TmpNode = NULL;
    struct stDDP_Neighb_Node* pstNeighb_CurNode = NULL;
    UINT4 u4Idx = 0;
    UINT4 u4ListIdx = 0;
    UINT4 u4Counter = 0;

    if(gpstDDP_Neighb_List == NULL){
        DDP_DEBUG_SWITCH("\n%s: line<%d> Please Init Switch DB first.\n", __FUNCTION__, __LINE__);
        return DDP_RET_SWITCH_ERROR_INIT_DB;
    }

    if((pstNewEntry == NULL) || (pstDDP_ifs == NULL))
    {
        DDP_DEBUG_SWITCH("\n%s: line<%d> NULL pointer.\n", __FUNCTION__, __LINE__);
        return DDP_RET_SWITCH_FAIL;
    }

    /* The switch port number where the neighbor report comes from */
    u4Idx = pstDDP_ifs->ifindex;

    if(u4Idx > kDDP_SWITCH_PORT_NUMBER){
        DDP_DEBUG_SWITCH("\n%s: line<%d> Invalid index<%d>\n",
                  __FUNCTION__, __LINE__, u4Idx);
        return DDP_RET_SWITCH_ERROR_INSERT_NODE;
    }

    /* Allocate memory for new node. */
    pstNeighb_NewNode = malloc(sizeof(struct stDDP_Neighb_Node));

    if(pstNeighb_NewNode == NULL){
        DDP_DEBUG_SWITCH("\n%s: line<%d> Failed to allocate memory\n", __FUNCTION__, __LINE__);
        return DDP_RET_SWITCH_ERROR_MEM;
    }

    /* Copy entry information to new node. */
    memcpy((INT1*)&(pstNeighb_NewNode->stDDP_Neighb_Entry), (INT1*)pstNewEntry, sizeof(struct stDDP_neighbor_entry));
    pstNeighb_NewNode->u2InIF = u4Idx;
    pstNeighb_NewNode->u2TTL= kDDP_SWITCH_UPDATE_TTL;

    /* Invalid count. Delete all nodes. */
    if( (gpstDDP_Neighb_List[u4Idx].u4TotalNodes == 0) && (gpstDDP_Neighb_List[u4Idx].pstFirstNode != NULL))
    {
        DDP_DEBUG_SWITCH("\n%s: line<%d> Invalid count.\n", __FUNCTION__, __LINE__);

        if(i4fnDDP_proto_switch_NeighbDB_delete_all(gpstDDP_Neighb_List[u4Idx].pstFirstNode) != DDP_RET_SWITCH_OK)
        {
            DDP_DEBUG_SWITCH("\n%s: line<%d> Failed to clear list.\n", __FUNCTION__, __LINE__);
        }

        gpstDDP_Neighb_List[u4Idx].pstFirstNode = NULL;
        gpstDDP_Neighb_List[u4Idx].pstTailNode = NULL;
        gpstDDP_Neighb_List[u4Idx].u4TotalNodes = 0;
    }

    /* Check all list to see whether the node already exists, delete it if any */
    for(u4ListIdx = 1; u4ListIdx <= kDDP_SWITCH_PORT_NUMBER; u4ListIdx++){
        pstNeighb_CurNode = gpstDDP_Neighb_List[u4ListIdx].pstFirstNode;
        u4Counter = 0;
        while(pstNeighb_CurNode != NULL){
            u4Counter++;
            pstNeighb_TmpNode = pstNeighb_CurNode->pstNextNode;
            if(memcmp(pstNeighb_CurNode->stDDP_Neighb_Entry.ai1MAC,
                      pstNeighb_NewNode->stDDP_Neighb_Entry.ai1MAC, MAC_ADDRLEN) == 0){

                /* Found existing entry, purge it. */
                DDP_DEBUG_SWITCH("\n == %s ==\n\t line<%d> list<%d> idx<%d>, Found existing entry , purge it.\n",
                                 __FUNCTION__, __LINE__, u4ListIdx, u4Counter);

                /* The only one node will be purged, and the list will be empty. */
                if( (pstNeighb_CurNode->pstPreNode == NULL)
                    && (pstNeighb_CurNode->pstNextNode == NULL) ){

                    DDP_DEBUG_SWITCH("\t line<%d> The only one node is purge, the list<%d> is empty.\n",
                                     __LINE__, u4ListIdx);

                    /* The only node will be purged, set pstFirstNode and pstTailNode. */
                    if(gpstDDP_Neighb_List[u4ListIdx].pstFirstNode != NULL){
                        gpstDDP_Neighb_List[u4ListIdx].pstFirstNode->pstNextNode = NULL;
                        gpstDDP_Neighb_List[u4ListIdx].pstFirstNode->pstPreNode = NULL;
                    }

                    if(gpstDDP_Neighb_List[u4ListIdx].pstTailNode != NULL){
                        gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstNextNode = NULL;
                        gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstPreNode = NULL;
                    }

                    gpstDDP_Neighb_List[u4ListIdx].pstFirstNode = NULL;
                    gpstDDP_Neighb_List[u4ListIdx].pstTailNode = NULL;
                    gpstDDP_Neighb_List[u4ListIdx].u4TotalNodes = 0;

                    goto PURGE;
                }

                if(gpstDDP_Neighb_List[u4ListIdx].pstFirstNode != NULL){

                    /* Handle pstFirstNode if the first node will be purged. */
                    if(pstNeighb_CurNode == gpstDDP_Neighb_List[u4ListIdx].pstFirstNode){
                        DDP_DEBUG_SWITCH("\t line<%d> First node<0x%x> will be purged.\n",
                                         __LINE__, (unsigned int)gpstDDP_Neighb_List[u4ListIdx].pstFirstNode);

                        gpstDDP_Neighb_List[u4ListIdx].pstFirstNode->pstPreNode = NULL;
                        gpstDDP_Neighb_List[u4ListIdx].pstFirstNode = pstNeighb_CurNode->pstNextNode;
                        if(gpstDDP_Neighb_List[u4ListIdx].pstFirstNode){
                            gpstDDP_Neighb_List[u4ListIdx].pstFirstNode->pstPreNode = NULL;
                        }
                    }

                    /* Handle pstFirstNode if the second node is purged */
                    if(pstNeighb_CurNode->pstPreNode == gpstDDP_Neighb_List[u4ListIdx].pstFirstNode){
                        DDP_DEBUG_SWITCH("\t line<%d> Second node<0x%x> will be purged.\n",
                                         __LINE__, (unsigned int)pstNeighb_CurNode);

                        gpstDDP_Neighb_List[u4ListIdx].pstFirstNode->pstNextNode = pstNeighb_CurNode->pstNextNode;
                        if(pstNeighb_CurNode->pstNextNode != NULL){
                            (pstNeighb_CurNode->pstNextNode)->pstPreNode = gpstDDP_Neighb_List[u4ListIdx].pstFirstNode;
                        }

                    }
                }

                /* Handle pstTailNode if the tail node will be purged. */
                if(gpstDDP_Neighb_List[u4ListIdx].pstTailNode != NULL){
                    if(pstNeighb_CurNode == gpstDDP_Neighb_List[u4ListIdx].pstTailNode){
                        DDP_DEBUG_SWITCH("\n%s: line<%d> Tail node<0x%x> will be purged.\n",
                                  __FUNCTION__, __LINE__, (unsigned int)gpstDDP_Neighb_List[u4ListIdx].pstTailNode);

                        gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstNextNode = NULL;
                        gpstDDP_Neighb_List[u4ListIdx].pstTailNode = gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstPreNode;
                        if(gpstDDP_Neighb_List[u4ListIdx].pstTailNode != NULL){
                            gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstNextNode = NULL;
                        }
                    }

                    /* Handle pstTailNode if the node before tail node is purged */
                    if(pstNeighb_CurNode->pstNextNode == gpstDDP_Neighb_List[u4ListIdx].pstTailNode){
                        DDP_DEBUG_SWITCH("\n%s: line<%d> node<0x%x> before tail node will be purged.\n",
                                  __FUNCTION__, __LINE__, (unsigned int)pstNeighb_CurNode);

                        gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstPreNode = pstNeighb_CurNode->pstPreNode;
                        if(gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstPreNode != NULL){
                            (gpstDDP_Neighb_List[u4ListIdx].pstTailNode->pstPreNode)->pstNextNode = gpstDDP_Neighb_List[u4ListIdx].pstTailNode;
                        }
                    }
                }

                /* For general nodes */
                /* Attach next node to previous node because current node will be pruged. */

                /* Attach next node to previous node */
                if(pstNeighb_CurNode->pstNextNode != NULL){
                   (pstNeighb_CurNode->pstNextNode)->pstPreNode = pstNeighb_CurNode->pstPreNode;
                }

                /* Attach previous node to next node */
                if(pstNeighb_CurNode->pstPreNode != NULL){
                    (pstNeighb_CurNode->pstPreNode)->pstNextNode = pstNeighb_CurNode->pstNextNode;
                }

            PURGE:
                if(pstNeighb_CurNode != NULL){
                    memset(pstNeighb_CurNode, 0, sizeof(struct stDDP_Neighb_Node));

                    free(pstNeighb_CurNode);
                    pstNeighb_CurNode = NULL;

                    if(gpstDDP_Neighb_List[u4ListIdx].u4TotalNodes > 0){
                        (gpstDDP_Neighb_List[u4ListIdx].u4TotalNodes--);
                    }

                    goto INSERT;
                }
                else{
                    DDP_DEBUG_SWITCH("\n%s: line<%d> Attempt to purge NULL node !!!!! \n",
                              __FUNCTION__, __LINE__);
                    return DDP_RET_SWITCH_ERROR_PURGE_NODE;
                }
            } /* if current MAC exists */

            pstNeighb_CurNode = pstNeighb_TmpNode;
        } /* while current node is not NULL */
    }

INSERT:
    if( (gpstDDP_Neighb_List[u4Idx].u4TotalNodes) > kDDP_SWITCH_MAX_ENTRY_PER_PACKET){
        DDP_DEBUG_SWITCH("\t line<%d> Each port supports max %d devices.\n",
                  __LINE__, kDDP_SWITCH_MAX_ENTRY_PER_PACKET);
        return DDP_RET_SWITCH_ERROR_MAX_NODE_PER_PKT;
    }

    if(pstNeighb_NewNode != NULL){
        pstNeighb_NewNode->u2InIF = u4Idx;
        pstNeighb_NewNode->u2TTL= kDDP_SWITCH_UPDATE_TTL;
        pstNeighb_NewNode->pstNextNode = NULL;
        /* Empty list, insert this node as the first one. */
        if(gpstDDP_Neighb_List[u4Idx].pstFirstNode == NULL)
        {
            if(gpstDDP_Neighb_List[u4Idx].u4TotalNodes != 0)
            {
                DDP_DEBUG_SWITCH("\t line<%d> Invalid total number.\n",
                                 __LINE__);
            }


            pstNeighb_NewNode->pstPreNode = NULL;

            gpstDDP_Neighb_List[u4Idx].pstFirstNode = pstNeighb_NewNode;
            gpstDDP_Neighb_List[u4Idx].pstTailNode = pstNeighb_NewNode;
            gpstDDP_Neighb_List[u4Idx].u4TotalNodes = 1;

            DDP_DEBUG_SWITCH("\t line<%d> Insert node to head of list<%d>\n",
                      __LINE__, u4Idx);

            goto OVER;
        }

        DDP_DEBUG_SWITCH("\t line<%d> Insert node<0x%x> to list<%d> \n",
                  __LINE__, (unsigned int)pstNeighb_NewNode, u4Idx);

        DDP_DEBUG_SWITCH("\t First<0x%x>, Tail<0x%x>, total<%d>\n",
                (unsigned int)gpstDDP_Neighb_List[u4Idx].pstFirstNode,
                (unsigned int)gpstDDP_Neighb_List[u4Idx].pstTailNode,
                (unsigned int)gpstDDP_Neighb_List[u4Idx].u4TotalNodes);

        /* Insert the new node as tail node. */
        if(gpstDDP_Neighb_List[u4Idx].pstTailNode->pstNextNode != NULL){
            /* Abnormal case! Log and check. */
            DDP_DEBUG_SWITCH("\t line<%d> Invalid tail node, please check!!!\n", __LINE__);
        }

        pstNeighb_NewNode->pstPreNode = gpstDDP_Neighb_List[u4Idx].pstTailNode;

        /* When inserting second node, set the pointer for pstFirstNode. */
        if(gpstDDP_Neighb_List[u4Idx].pstFirstNode->pstNextNode == NULL){
            DDP_DEBUG_SWITCH("\t line<%d> Insert second node\n",  __LINE__);
            gpstDDP_Neighb_List[u4Idx].pstFirstNode->pstNextNode = pstNeighb_NewNode;
            gpstDDP_Neighb_List[u4Idx].pstFirstNode->pstPreNode = NULL;
        }

        gpstDDP_Neighb_List[u4Idx].pstTailNode = pstNeighb_NewNode;

        if(gpstDDP_Neighb_List[u4Idx].pstTailNode->pstPreNode != NULL){
            (gpstDDP_Neighb_List[u4Idx].pstTailNode->pstPreNode)->pstNextNode = pstNeighb_NewNode;
        }

        (gpstDDP_Neighb_List[u4Idx].u4TotalNodes)++;
    }
    else{
        DDP_DEBUG_SWITCH("\t line<%d> Invalid node.\n", __LINE__);
        return DDP_RET_SWITCH_ERROR_MEM;
    }
OVER:

    /********************/
    /*  Debug message   */
    /********************/
    if (g_debugFlag & DDP_DEBUG_PRINT_SWITCH) {
    	DDP_DEBUG_SWITCH("\n\t ***************************************\n");
    	DDP_DEBUG_SWITCH("\t **** Updated db List[%d], total<%02d> ****\n",
               u4Idx, gpstDDP_Neighb_List[u4Idx].u4TotalNodes);
    	DDP_DEBUG_SWITCH("\t ***************************************\n");

        DDP_DEBUG_SWITCH("\t New<0x%x>, First<0x%x>, Tail<0x%x>\n",
                (unsigned int)pstNeighb_NewNode,
                  (unsigned int)gpstDDP_Neighb_List[u4Idx].pstFirstNode,
                  (unsigned int)gpstDDP_Neighb_List[u4Idx].pstTailNode);

        pstNeighb_CurNode = gpstDDP_Neighb_List[u4Idx].pstFirstNode;
        u4Counter = 0;
        while(pstNeighb_CurNode != NULL){

            DDP_DEBUG_SWITCH("\n\t <0x%x> -> <0x%x> -> <0x%x> >>>>  ",
                    (unsigned int)pstNeighb_CurNode->pstPreNode,
                    (unsigned int)pstNeighb_CurNode,
                    (unsigned int)pstNeighb_CurNode->pstNextNode);

            u4Counter++;
            DDP_DEBUG_SWITCH("\n\t Node %d: MAC [", u4Counter);
            for(u4ListIdx=0; u4ListIdx < MAC_ADDRLEN; u4ListIdx++)
            	DDP_DEBUG_SWITCH("0x%02x ", pstNeighb_CurNode->stDDP_Neighb_Entry.ai1MAC[u4ListIdx]);

            DDP_DEBUG_SWITCH("]\n\n");
            pstNeighb_CurNode = pstNeighb_CurNode->pstNextNode;
        }
    }

    return DDP_RET_SWITCH_OK;
}

/* i4fnDDP_proto_switch_NeighbDB_delete_all
 *   function to delete neighbor information list starting from the given node.
 *
 *   pstDDP_Neighb_Node : pointer to the starting node.
 *   This function will do the following thins:
 *   1. free the node on the list.
 *   2. set the pointer to null.
 *
 *   return : DDP_RET_SWITCH_OK -> success, others -> error
 */
INT4 i4fnDDP_proto_switch_NeighbDB_delete_all(struct stDDP_Neighb_Node* pstDDP_Neighb_Node)
{
    struct stDDP_Neighb_Node* pstDDP_Neighb_Current = NULL;
    struct stDDP_Neighb_Node* pstDDP_Neighb_Next = NULL;

    if(pstDDP_Neighb_Node == NULL)
    {
        DDP_DEBUG_SWITCH("\n%s: line<%d> NULL pointer.\n", __FUNCTION__, __LINE__);
        return DDP_INVALID_FIELD_QUERY_NEIGHBOR_IN_DATA;
    }

    pstDDP_Neighb_Current = pstDDP_Neighb_Node;

    while(pstDDP_Neighb_Current != NULL)
    {
        pstDDP_Neighb_Next = pstDDP_Neighb_Current->pstNextNode;

        memset(pstDDP_Neighb_Current, 0x0, sizeof(struct stDDP_Neighb_Node));
        if(pstDDP_Neighb_Current != NULL){

            free(pstDDP_Neighb_Current);
            pstDDP_Neighb_Current = NULL;
        }

        pstDDP_Neighb_Current = pstDDP_Neighb_Next;
    }

    DDP_DEBUG_SWITCH("%s: line<%d> Done\n", __FUNCTION__, __LINE__);
    return DDP_RET_SWITCH_OK;
}

/* i4fnDDP_proto_switch_vlan_cfg (OP: 0xD2 (210))
 *   function to process vlan cfg packet and send through the incoming interfaces
 *
 *   pstDDP_ifs : pointer to the ddp interface structure
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan cfg msg structure
 *
 *   return : DDP_ALL_FIELD_SUCCESS -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_vlan_cfg
(
    struct ddp_interface* pstDDP_ifs,
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    struct ddp_header stDDP_Hdr_out;
    struct ddp_message stDDP_Msg_pkt;
    struct ddp_exe_status stDDP_ErrStatus_out;

    INT4 i4ret = DDP_ALL_FIELD_SUCCESS;
    INT4 i4outMsgLen = 0;
    INT4 i4len = 0;
    INT4 i4pos = 0;
    UINT1* pu1inBody = NULL;
    UINT1* pu1outMsg = NULL;
    UINT1* pu1outBody = NULL;
    INT1 ai1str[kDDP_VLAN_CFG_STR_LEN];
    INT1 ai1plist[kDDP_VLAN_CFG_PLIST_LEN];
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];

    if (pstDDP_ifs == NULL || pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL)
    {
        return -1;
    }

    /* verify input message */
    if (i4fnDDP_proto_switch_vlan_cfg_check_header(pstDDP_Hdr_in, pstDDP_Msg_in) != DDP_RETCODE_OK)
    {
        return DDP_INVALID_FIELD_VLAN_SET;
    }

    memset(&stDDP_ErrStatus_out, 0, sizeof(struct ddp_exe_status));
    memset(&stDDP_Hdr_out, 0, sizeof(struct ddp_header));
    stDDP_Hdr_out.ipVer = pstDDP_Hdr_in->ipVer;
    stDDP_Hdr_out.seq = pstDDP_Hdr_in->seq;
    stDDP_Hdr_out.opcode = pstDDP_Hdr_in->opcode;
    memcpy(stDDP_Hdr_out.macAddr, pstDDP_ifs->macAddr, MAC_ADDRLEN);
    stDDP_Hdr_out.protoVer = DDP_PROTO_V2;
    stDDP_Hdr_out.bodyLen = DDP_REPLY_LEN_VLAN_CFG;
    /* Fill in ipv4 or ipv6 address */
    if (pstDDP_Hdr_in->ipVer == IPV4_FLAG) {
        stDDP_Hdr_out.identifier = IPV4_REPLY;
        i4len = IPV4_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IP_ADDR, stDDP_Hdr_out.ipAddr.ipv4Addr, &i4len);
        if (pstDDP_Hdr_in->bodyLen > 0) { pu1inBody = pstDDP_Msg_in->payload + HDR_END_V4_OFFSET; }
        i4outMsgLen = HDR_END_V4_OFFSET + stDDP_Hdr_out.bodyLen;
    } else {
        stDDP_Hdr_out.identifier = IPV6_REPLY;
        i4len = IPV6_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IPV6_ADDR, stDDP_Hdr_out.ipAddr.ipv6Addr, &i4len);
        if (pstDDP_Hdr_in->bodyLen > 0) { pu1inBody = pstDDP_Msg_in->payload + HDR_END_V6_OFFSET; }
        i4outMsgLen = HDR_END_V6_OFFSET + stDDP_Hdr_out.bodyLen;
    }

    /* allocate output message */
    pu1outMsg = (UINT1*)malloc(i4outMsgLen);

    if (pu1outMsg == NULL) {
        DDP_DEBUG_SWITCH("fail to allocate vlan cfg output request msg\n");
        return -3;
    }

    memset(pu1outMsg, 0, i4outMsgLen);
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        pu1outBody = pu1outMsg + HDR_END_V4_OFFSET;
    } else {
        pu1outBody = pu1outMsg + HDR_END_V6_OFFSET;
    }

    if (pstDDP_Hdr_in->bodyLen == 0 || pu1inBody == NULL) {
        stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
        i4ret = -1;
        goto process_vlan_cfg_over;
    }
    /***************************************/
    /* configure corresponding vlan fields */
    /***************************************/
    /* Initialize */
    i4len = 0;
    i4pos = 0;

    /* retrieve username and password from msg body and verify them */
    if (ddp_proto_check_op_authen(DDP_OP_VLAN_CFG) == DDP_OP_AUTHEN_ON) {
        if (i4ret == DDP_ALL_FIELD_SUCCESS && pu1inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, pu1inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, pu1inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            i4ret = ddp_proto_verify_authen(pstDDP_ifs, encUsername, encPassword);
        } else {
            i4ret = -2;
        }
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            stDDP_Hdr_out.retCode = DDP_RETCODE_AUTHEN_FAIL;
            goto process_vlan_cfg_over;
        }
        i4pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
    }

    /* Set VLAN name */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_VLAN_NAME;
        memset(ai1str, 0, kDDP_VLAN_CFG_STR_LEN);
        memcpy(ai1str, pu1inBody + i4pos, i4len);
        if (strlen(ai1str) > 0) {
            i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_VLAN_NAME, (UINT1*)ai1str, strlen(ai1str));
        }
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set vlan name fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_VLAN_SET_NAME_IN_DATA;
        }
    }

    /* Set VLAN ID */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_VLAN_ID;
        i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_VLAN_ID, pu1inBody + i4pos, i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set vlan id fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_VLAN_SET_ID_IN_DATA;
        }
    }

    /* Set VLAN tag port list type */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE;
        i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_VLAN_TAG_PLIST_TYPE, pu1inBody + i4pos, i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set vlan tag port list type fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_VLAN_SET_TLIST_TYPE_IN_DATA;

        }
    }

    /* Set VLAN tag port list */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_VLAN_TAG_PLIST;
        memset(ai1plist, 0, sizeof(ai1plist));
        memcpy(ai1plist, pu1inBody + i4pos, i4len);
        i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_VLAN_TAG_PLIST, (UINT1*)ai1plist, i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set vlan tag post list fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_VLAN_SET_TLIST_IN_DATA;
        }
    }

    /* Set VLAN untag port list type */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_VLAN_UNTAG_LIST_TYPE;
        i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_VLAN_UNTAG_PLIST_TYPE, pu1inBody + i4pos, i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set vlan untag port list type fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_VLAN_SET_ULIST_TYPE_IN_DATA;

        }
    }

    /* Set VLAN untag port list */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_VLAN_UNTAG_PLIST;
        memset(ai1plist, 0, sizeof(ai1plist));
        memcpy(ai1plist, pu1inBody + i4pos, i4len);
        i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_VLAN_UNTAG_PLIST, (UINT1*)ai1plist, i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set vlan untag post list fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_VLAN_SET_ULIST_IN_DATA;
        }
    }

    /* Set the vlan on machine, use vendor api */
    i4ret = ddp_platform_set_if_vlan(pstDDP_ifs);
    if (i4ret != DDP_RETCODE_OK) {
        DDP_DEBUG_SWITCH("\n%s (%d) : set vlan driver api fail (error %d)\n", __FILE__, __LINE__, i4ret);
        stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
        stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_VLAN_SET_DRIVER;
    }

process_vlan_cfg_over:
    memset(&stDDP_Msg_pkt, 0, sizeof(struct ddp_message));
    stDDP_Msg_pkt.ifindex = pstDDP_ifs->ifindex;

    if( i4fnDDP_proto_switch_pack_reply_msg(&stDDP_Hdr_out, pstDDP_Msg_in, &stDDP_ErrStatus_out,
        &stDDP_Msg_pkt, pu1outMsg, i4outMsgLen, pu1outBody) != DDP_RETCODE_OK)
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : pack reply msg fail.\n", __FILE__, __LINE__);
    }

    sendout_msg(&stDDP_Msg_pkt, pu1outMsg, i4outMsgLen);

    if (pu1outMsg) {
        free(pu1outMsg);
        pu1outMsg = NULL;
    }

    if (g_debugFlag & DDP_DEBUG_PRINT_RECV_MSG_HEX) {
        UINT1 au1Tmp[128]={0};
        int   ilen = 0;

        DDP_DEBUG_SWITCH("\n%s (%d) : \nRead-back test\n=======\n", __FILE__, __LINE__);

        memset(au1Tmp, 0, sizeof(au1Tmp));
        ilen = DDP_FIELD_LEN_VLAN_NAME;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_NAME, (UINT1*)au1Tmp, &ilen);


        memset(au1Tmp, 0, sizeof(au1Tmp));
        ilen = DDP_FIELD_LEN_VLAN_ID;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_ID, (UINT1*)au1Tmp, &ilen);

        memset(au1Tmp, 0, sizeof(au1Tmp));
        ilen = DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_TAG_PLIST_TYPE, (UINT1*)au1Tmp, &ilen);

        if(*au1Tmp == 0){ //string
            memset(au1Tmp, 0, sizeof(au1Tmp));
            ilen = DDP_FIELD_LEN_VLAN_TAG_PLIST;
            ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_TAG_PLIST, (UINT1*)au1Tmp, &ilen);
        }
        else{//bitmap
            memset(au1Tmp, 0, sizeof(au1Tmp));
            ilen = DDP_FIELD_LEN_VLAN_TAG_PLIST;
            ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_TAG_PLIST, (UINT1*)au1Tmp, &ilen);
        }

        memset(au1Tmp, 0, sizeof(au1Tmp));
        ilen = DDP_FIELD_LEN_VLAN_UNTAG_LIST_TYPE;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_UNTAG_PLIST_TYPE, (UINT1*)au1Tmp, &ilen);

        if(*au1Tmp == 0){ //string
            memset(au1Tmp, 0, sizeof(au1Tmp));
            ilen = DDP_FIELD_LEN_VLAN_UNTAG_PLIST;
            ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_UNTAG_PLIST, (UINT1*)au1Tmp, &ilen);
        }
        else{//bitmap
            memset(au1Tmp, 0, sizeof(au1Tmp));
            ilen = DDP_FIELD_LEN_VLAN_UNTAG_PLIST;
            ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_UNTAG_PLIST, (UINT1*)au1Tmp, &ilen);
        }

        DDP_DEBUG_SWITCH("\n=======\n");
    }

    return i4ret;
}

/* i4fnDDP_proto_switch_vlan_get (OP: 0xDC (220))
 *   function to process vlan get packet and send through the incoming interfaces
 *
 *   pstDDP_ifs : pointer to the ddp interface structure
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan get msg structure
 *
 *   return : DDP_ALL_FIELD_SUCCESS -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_vlan_get
(
    struct ddp_interface* pstDDP_ifs,
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    struct ddp_header stDDP_Hdr_out;
    struct ddp_message stDDP_Msg_pkt;

    INT4 i4ret = DDP_ALL_FIELD_SUCCESS;
    INT4 i4outMsgLen = 0;
    INT4 i4len = 0;
    INT4 i4pos = 0;
    UINT1* pu1inBody = NULL;
    UINT1* pu1outMsg = NULL;
    UINT1* pu1outBody = NULL;
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;
    UINT1 type = 0;
    UINT4 vlanId = 0;
    struct vlan_container tmp;

    if (pstDDP_ifs == NULL || pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL)
    {
        return -1;
    }

    memset(&stDDP_Hdr_out, 0, sizeof(struct ddp_header));
    stDDP_Hdr_out.ipVer = pstDDP_Hdr_in->ipVer;
    stDDP_Hdr_out.seq = pstDDP_Hdr_in->seq;
    stDDP_Hdr_out.opcode = pstDDP_Hdr_in->opcode;
    memcpy(stDDP_Hdr_out.macAddr, pstDDP_ifs->macAddr, MAC_ADDRLEN);
    stDDP_Hdr_out.protoVer = DDP_PROTO_V2;
    stDDP_Hdr_out.bodyLen = DDP_REPLY_LEN_VLAN_GET;
    /* Fill in ipv4 or ipv6 address */
    if (pstDDP_Hdr_in->ipVer == IPV4_FLAG) {
        stDDP_Hdr_out.identifier = IPV4_REPLY;
        i4len = IPV4_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IP_ADDR, stDDP_Hdr_out.ipAddr.ipv4Addr, &i4len);
        if (pstDDP_Hdr_in->bodyLen > 0) { pu1inBody = pstDDP_Msg_in->payload + HDR_END_V4_OFFSET; }
        i4outMsgLen = HDR_END_V4_OFFSET + stDDP_Hdr_out.bodyLen;
    } else {
        stDDP_Hdr_out.identifier = IPV6_REPLY;
        i4len = IPV6_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IPV6_ADDR, stDDP_Hdr_out.ipAddr.ipv6Addr, &i4len);
        if (pstDDP_Hdr_in->bodyLen > 0) { pu1inBody = pstDDP_Msg_in->payload + HDR_END_V6_OFFSET; }
        i4outMsgLen = HDR_END_V6_OFFSET + stDDP_Hdr_out.bodyLen;
    }

    /* allocate output message */
    pu1outMsg = (UINT1*)malloc(i4outMsgLen);
    if (pu1outMsg == NULL) {
        DDP_DEBUG_SWITCH("fail to allocate vlan get output request msg\n");
        return -3;
    }
    memset(pu1outMsg, 0, i4outMsgLen);
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        pu1outBody = pu1outMsg + HDR_END_V4_OFFSET;
    } else {
        pu1outBody = pu1outMsg + HDR_END_V6_OFFSET;
    }

    if (pu1inBody == NULL) {
        DDP_DEBUG_SWITCH("can not get body content in in msg\n");
        stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
        i4ret = -4;
        goto process_vlan_get_over;
    }
    /* process input body content */
    type = *(pu1inBody);
    if (type == 1) {
        memcpy(&vlanId, pu1inBody + DDP_FIELD_LEN_VLAN_TYPE, DDP_FIELD_LEN_VLAN_ID);
        vlanId = DDP_NTOHL(vlanId);
        stDDP_Hdr_out.bodyLen = 168;
        if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
            i4outMsgLen = HDR_END_V4_OFFSET + stDDP_Hdr_out.bodyLen;
        } else {
            i4outMsgLen = HDR_END_V6_OFFSET + stDDP_Hdr_out.bodyLen;
        }
    } else if (type == 0) {
        stDDP_Hdr_out.bodyLen = 513;
    } else {
        stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
        goto process_vlan_get_over;
    }
    /***************************************/
    /* get corresponding vlan fields */
    /***************************************/
    /* Initialize */
    i4len = 0;
    i4pos = 0;

    *pu1outBody = type;
    i4pos += DDP_FIELD_LEN_VLAN_TYPE;
    /* request vlan list */
    if (type == 0) {
        i4len = DDP_FIELD_LEN_VLAN_LIST;
        i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_LIST, pu1outBody + i4pos, &i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : get vlan list fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
        }
    }
    /* request info of the specified vlan */
    else {
        /* VLAN state */
        memset((void*)&tmp, 0, sizeof(tmp));
        tmp.vlanId = vlanId;
        i4len = DDP_FIELD_LEN_VLAN_STATE;
        i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_STATE, (UINT1*)&tmp, &i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : get vlan name fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
        } else {
            memcpy((void*)(pu1outBody + i4pos), (void*)tmp.buf, DDP_FIELD_LEN_VLAN_STATE);
        }
        
        /* VLAN name */
        if (i4ret == DDP_ALL_FIELD_SUCCESS) {
            memset((void*)&tmp, 0, sizeof(tmp));
            tmp.vlanId = vlanId;
            i4pos += DDP_FIELD_LEN_VLAN_STATE;
            i4len = DDP_FIELD_LEN_VLAN_NAME;
            i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_NAME, (UINT1*)&tmp, &i4len);
            if (i4ret != DDP_ALL_FIELD_SUCCESS) {
                DDP_DEBUG_SWITCH("\n%s (%d) : get vlan name fail (error %d)\n", __FILE__, __LINE__, i4ret);
                stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            } else {
                strcpy((INT1*)(pu1outBody + i4pos), (INT1*)tmp.buf);
            }
        }

        /* VLAN ID */
        if (i4ret == DDP_ALL_FIELD_SUCCESS) {
            i4pos += DDP_FIELD_LEN_VLAN_NAME;
            i4len = DDP_FIELD_LEN_VLAN_ID;
            memcpy(pu1outBody + i4pos, pu1inBody + DDP_FIELD_LEN_VLAN_TYPE, i4len);
        }

        /* VLAN tag port list type */
        if (i4ret == DDP_ALL_FIELD_SUCCESS) {
            memset(&tmp, 0, sizeof(tmp));
            tmp.vlanId = vlanId;
            i4pos += DDP_FIELD_LEN_VLAN_ID;
            i4len = DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE;
            i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_TAG_PLIST_TYPE, (UINT1*)&tmp, &i4len);
            if (i4ret != DDP_ALL_FIELD_SUCCESS) {
                DDP_DEBUG_SWITCH("\n%s (%d) : get vlan tag port list type fail (error %d)\n", __FILE__, __LINE__, i4ret);
                stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            } else {
                memcpy(pu1outBody + i4pos, tmp.buf, i4len);
            }
        }

        /* VLAN tag port list */
        if (i4ret == DDP_ALL_FIELD_SUCCESS) {
            memset(&tmp, 0, sizeof(tmp));
            tmp.vlanId = vlanId;
            i4pos += DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE;
            i4len = DDP_FIELD_LEN_VLAN_TAG_PLIST;
            i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_TAG_PLIST, (UINT1*)&tmp, &i4len);
            if (i4ret != DDP_ALL_FIELD_SUCCESS) {
                DDP_DEBUG_SWITCH("\n%s (%d) : get vlan tag post list fail (error %d)\n", __FILE__, __LINE__, i4ret);
                stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            } else {
                memcpy(pu1outBody + i4pos, tmp.buf, i4len);
            }
        }

        /* VLAN untag port list type */
        if (i4ret == DDP_ALL_FIELD_SUCCESS) {
            memset(&tmp, 0, sizeof(tmp));
            tmp.vlanId = vlanId;
            i4pos += DDP_FIELD_LEN_VLAN_TAG_PLIST;
            i4len = DDP_FIELD_LEN_VLAN_UNTAG_LIST_TYPE;
            i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_UNTAG_PLIST_TYPE, (UINT1*)&tmp, &i4len);
            if (i4ret != DDP_ALL_FIELD_SUCCESS) {
                DDP_DEBUG_SWITCH("\n%s (%d) : get vlan untag port list type fail (error %d)\n", __FILE__, __LINE__, i4ret);
                stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            } else {
                memcpy(pu1outBody + i4pos, tmp.buf, i4len);
            }
        }

        /* VLAN tag port list */
        if (i4ret == DDP_ALL_FIELD_SUCCESS) {
            memset(&tmp, 0, sizeof(tmp));
            tmp.vlanId = vlanId;
            i4pos += DDP_FIELD_LEN_VLAN_UNTAG_LIST_TYPE;
            i4len = DDP_FIELD_LEN_VLAN_UNTAG_PLIST;
            i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_VLAN_UNTAG_PLIST, (UINT1*)&tmp, &i4len);
            if (i4ret != DDP_ALL_FIELD_SUCCESS) {
                DDP_DEBUG_SWITCH("\n%s (%d) : get vlan untag post list fail (error %d)\n", __FILE__, __LINE__, i4ret);
                stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            } else {
                memcpy(pu1outBody + i4pos, tmp.buf, i4len);
            }
        }
    }

process_vlan_get_over:
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&stDDP_Hdr_out);
    }
    pack_header(pu1outMsg, &stDDP_Hdr_out);

    memset(&stDDP_Msg_pkt, 0, sizeof(stDDP_Msg_pkt));
    stDDP_Msg_pkt.ifindex = pstDDP_ifs->ifindex;
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        inAddr = (struct sockaddr_in*)&pstDDP_Msg_in->sender;
        outAddr = (struct sockaddr_in*)&stDDP_Msg_pkt.sender;
        outAddr->sin_family = inAddr->sin_family;
        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        inAddr6 = (struct sockaddr_in6*)&pstDDP_Msg_in->sender;
        outAddr6 = (struct sockaddr_in6*)&stDDP_Msg_pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(pu1outMsg, i4outMsgLen);
    }
    sendout_msg(&stDDP_Msg_pkt, pu1outMsg, i4outMsgLen);

    if (pu1outMsg) { free(pu1outMsg); pu1outMsg = NULL; }
    return i4ret;

    return i4ret;
}

/* i4fnDDP_proto_switch_asv_cfg (OP: 0xD3 (211))
 *   function to process asv cfg packet and send through the incoming interfaces
 *
 *   pstDDP_ifs : pointer to the ddp interface structure
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan cfg msg structure
 *
 *   return : DDP_ALL_FIELD_SUCCESS -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_asv_cfg
(
    struct ddp_interface* pstDDP_ifs,
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    struct ddp_header stDDP_Hdr_out;
    struct ddp_message stDDP_Msg_pkt;
    struct ddp_exe_status stDDP_ErrStatus_out;

    INT4 i4ret = DDP_ALL_FIELD_SUCCESS;
    INT4 i4outMsgLen = 0;
    INT4 i4len = 0;
    INT4 i4pos = 0;
    UINT1* pu1inBody = NULL;
    UINT1* pu1outMsg = NULL;
    UINT1* pu1outBody = NULL;
    INT1 ai1plist[kDDP_ASV_CFG_PLIST_LEN];
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];

    if (pstDDP_ifs == NULL || pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL)
    {
        return -1;
    }

    /* verify input message */
    if (i4fnDDP_proto_switch_asv_cfg_check_header(pstDDP_Hdr_in, pstDDP_Msg_in) != DDP_RETCODE_OK)
    {
        return DDP_INVALID_FIELD_ASV_SET;
    }

    memset(&stDDP_ErrStatus_out, 0, sizeof(struct ddp_exe_status));
    memset(&stDDP_Hdr_out, 0, sizeof(struct ddp_header));
    stDDP_Hdr_out.ipVer = pstDDP_Hdr_in->ipVer;
    stDDP_Hdr_out.seq = pstDDP_Hdr_in->seq;
    stDDP_Hdr_out.opcode = pstDDP_Hdr_in->opcode;
    memcpy(stDDP_Hdr_out.macAddr, pstDDP_ifs->macAddr, MAC_ADDRLEN);
    stDDP_Hdr_out.protoVer = DDP_PROTO_V2;
    stDDP_Hdr_out.bodyLen = DDP_REPLY_LEN_ASV_CFG;
    /* Fill in ipv4 or ipv6 address */
    if (pstDDP_Hdr_in->ipVer == IPV4_FLAG) {
        stDDP_Hdr_out.identifier = IPV4_REPLY;
        i4len = IPV4_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IP_ADDR, stDDP_Hdr_out.ipAddr.ipv4Addr, &i4len);
        if (pstDDP_Hdr_in->bodyLen > 0) { pu1inBody = pstDDP_Msg_in->payload + HDR_END_V4_OFFSET; }
        i4outMsgLen = HDR_END_V4_OFFSET + stDDP_Hdr_out.bodyLen;
    } else {
        stDDP_Hdr_out.identifier = IPV6_REPLY;
        i4len = IPV6_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IPV6_ADDR, stDDP_Hdr_out.ipAddr.ipv6Addr, &i4len);
        if (pstDDP_Hdr_in->bodyLen > 0) { pu1inBody = pstDDP_Msg_in->payload + HDR_END_V6_OFFSET; }
        i4outMsgLen = HDR_END_V6_OFFSET + stDDP_Hdr_out.bodyLen;
    }

    /* Set the asv user mac on machine, use vendor api */
    i4ret = ddp_platform_set_if_asv_usr_def_mac_hw(pstDDP_ifs);
    if (i4ret != DDP_RETCODE_OK) {
        DDP_DEBUG_SWITCH("\n%s (%d) : set ASV user mac driver api fail (error %d)\n", __FILE__, __LINE__, i4ret);
        stDDP_Hdr_out.retCode = DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET_DRIVER;
        stDDP_ErrStatus_out.errNo = i4ret;
    }

    /* allocate output message */
    pu1outMsg = (UINT1*)malloc(i4outMsgLen);

    if (pu1outMsg == NULL) {
        DDP_DEBUG_SWITCH("fail to allocate asv cfg output request msg\n");
        return -3;
    }

    memset(pu1outMsg, 0, i4outMsgLen);
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        pu1outBody = pu1outMsg + HDR_END_V4_OFFSET;
    } else {
        pu1outBody = pu1outMsg + HDR_END_V6_OFFSET;
    }

    /***************************************/
    /* configure corresponding asv fields */
    /***************************************/
    /* Initialize */
    i4len = 0;
    i4pos = 0;

    /* retrieve username and password from msg body and verify them */
    if (ddp_proto_check_op_authen(DDP_OP_ASV_CFG) == DDP_OP_AUTHEN_ON) {
        if (i4ret == DDP_ALL_FIELD_SUCCESS && pu1inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, pu1inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, pu1inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            i4ret = ddp_proto_verify_authen(pstDDP_ifs, encUsername, encPassword);
        } else {
            i4ret = -2;
        }
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            stDDP_Hdr_out.retCode = DDP_RETCODE_AUTHEN_FAIL;
            goto process_asv_cfg_over;
        }
        i4pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
    }

    /* Set ASV ID */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_ASV_ID;
        i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_ASV_ID, pu1inBody + i4pos, i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set asv id fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_ASV_SET_ID_IN_DATA;
        }
    }

    /* Set ASV Priority */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_ASV_PRI;
        i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_ASV_PRI, pu1inBody + i4pos, i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set asv priority fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_ASV_SET_PRIORITY_IN_DATA;
        }
    }

    /* Set ASV State */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_ASV_STATE;
        i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_ASV_STATE, pu1inBody + i4pos, i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set asv state fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_ASV_SET_STATE_IN_DATA;
        }
    }

    /* Set ASV tag port list type */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_ASV_TAG_LIST_TYPE;
        i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_ASV_TAG_PLIST_TYPE, pu1inBody + i4pos, i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set ASV tag port list type fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_ASV_SET_TLIST_TYPE_IN_DATA;

        }
    }

    /* Set ASV tag port list */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_ASV_TAG_PLIST;
        memset(ai1plist, 0, sizeof(ai1plist));
        memcpy(ai1plist, pu1inBody + i4pos, i4len);
        i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_ASV_TAG_PLIST, (UINT1*)ai1plist, i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : set ASV tag post list fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
            stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_ASV_SET_TLIST_IN_DATA;
        }
    }

    /* Set the asv on machine, use vendor api */
    i4ret = ddp_platform_set_if_asv_hw(pstDDP_ifs);
    if (i4ret != DDP_RETCODE_OK) {
        DDP_DEBUG_SWITCH("\n%s (%d) : set ASV driver api fail (error %d)\n", __FILE__, __LINE__, i4ret);
        stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
        stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_ASV_SET_DRIVER;
    }

process_asv_cfg_over:
    memset(&stDDP_Msg_pkt, 0, sizeof(struct ddp_message));
    stDDP_Msg_pkt.ifindex = pstDDP_ifs->ifindex;

    if( i4fnDDP_proto_switch_pack_reply_msg(&stDDP_Hdr_out, pstDDP_Msg_in, &stDDP_ErrStatus_out,
        &stDDP_Msg_pkt, pu1outMsg, i4outMsgLen, pu1outBody) != DDP_RETCODE_OK)
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : pack reply msg fail.\n", __FILE__, __LINE__);
    }

    sendout_msg(&stDDP_Msg_pkt, pu1outMsg, i4outMsgLen);

    if (pu1outMsg) {
        free(pu1outMsg);
        pu1outMsg = NULL;
    }

    if (g_debugFlag & DDP_DEBUG_PRINT_RECV_MSG_HEX) {
        UINT1 au1Tmp[128]={0};
        int   ilen = 0;

        DDP_DEBUG_SWITCH("\n%s (%d) : \nRead-back test\n=======\n", __FILE__, __LINE__);

        memset(au1Tmp, 0, sizeof(au1Tmp));
        ilen = DDP_FIELD_LEN_ASV_ID;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_ID, (UINT1*)au1Tmp, &ilen);

        memset(au1Tmp, 0, sizeof(au1Tmp));
        ilen = DDP_FIELD_LEN_ASV_PRI;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_PRI, (UINT1*)au1Tmp, &ilen);

        memset(au1Tmp, 0, sizeof(au1Tmp));
        ilen = DDP_FIELD_LEN_ASV_STATE;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_STATE, (UINT1*)au1Tmp, &ilen);

        memset(au1Tmp, 0, sizeof(au1Tmp));
        ilen = DDP_FIELD_LEN_ASV_TAG_LIST_TYPE;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_TAG_PLIST_TYPE, (UINT1*)au1Tmp, &ilen);

        if(*au1Tmp == 0){ //string
            memset(au1Tmp, 0, sizeof(au1Tmp));
            ilen = DDP_FIELD_LEN_ASV_TAG_PLIST;
            ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_TAG_PLIST, (UINT1*)au1Tmp, &ilen);
        }
        else{//bitmap
            memset(au1Tmp, 0, sizeof(au1Tmp));
            ilen = DDP_FIELD_LEN_VLAN_TAG_PLIST;
            ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_TAG_PLIST, (UINT1*)au1Tmp, &ilen);
        }

        DDP_DEBUG_SWITCH("\n=======\n");
    }

    return i4ret;
}

/* i4fnDDP_proto_switch_asv_get (OP: 0xDB (219))
 *   function to process asv get packet and send through the incoming interfaces
 *
 *   pstDDP_ifs : pointer to the ddp interface structure
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input asvlan get msg structure
 *
 *   return : DDP_ALL_FIELD_SUCCESS -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_asv_get
(
    struct ddp_interface* pstDDP_ifs,
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    struct ddp_header stDDP_Hdr_out;
    struct ddp_message stDDP_Msg_pkt;

    INT4 i4ret = DDP_ALL_FIELD_SUCCESS;
    INT4 i4outMsgLen = 0;
    INT4 i4len = 0;
    INT4 i4pos = 0;
    UINT1* pu1inBody = NULL;
    UINT1* pu1outMsg = NULL;
    UINT1* pu1outBody = NULL;
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in* outAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    struct sockaddr_in6* outAddr6 = NULL;

    if (pstDDP_ifs == NULL || pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL)
    {
        return -1;
    }

    memset(&stDDP_Hdr_out, 0, sizeof(struct ddp_header));
    stDDP_Hdr_out.ipVer = pstDDP_Hdr_in->ipVer;
    stDDP_Hdr_out.seq = pstDDP_Hdr_in->seq;
    stDDP_Hdr_out.opcode = pstDDP_Hdr_in->opcode;
    memcpy(stDDP_Hdr_out.macAddr, pstDDP_ifs->macAddr, MAC_ADDRLEN);
    stDDP_Hdr_out.protoVer = DDP_PROTO_V2;
    stDDP_Hdr_out.bodyLen = DDP_REPLY_LEN_ASV_GET;
    /* Fill in ipv4 or ipv6 address */
    if (pstDDP_Hdr_in->ipVer == IPV4_FLAG) {
        stDDP_Hdr_out.identifier = IPV4_REPLY;
        i4len = IPV4_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IP_ADDR, stDDP_Hdr_out.ipAddr.ipv4Addr, &i4len);
        if (pstDDP_Hdr_in->bodyLen > 0) { pu1inBody = pstDDP_Msg_in->payload + HDR_END_V4_OFFSET; }
        i4outMsgLen = HDR_END_V4_OFFSET + stDDP_Hdr_out.bodyLen;
    } else {
        stDDP_Hdr_out.identifier = IPV6_REPLY;
        i4len = IPV6_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IPV6_ADDR, stDDP_Hdr_out.ipAddr.ipv6Addr, &i4len);
        if (pstDDP_Hdr_in->bodyLen > 0) { pu1inBody = pstDDP_Msg_in->payload + HDR_END_V6_OFFSET; }
        i4outMsgLen = HDR_END_V6_OFFSET + stDDP_Hdr_out.bodyLen;
    }

    /* allocate output message */
    pu1outMsg = (UINT1*)malloc(i4outMsgLen);
    if (pu1outMsg == NULL) {
        DDP_DEBUG_SWITCH("fail to allocate asv get output request msg\n");
        return -3;
    }
    memset(pu1outMsg, 0, i4outMsgLen);
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        pu1outBody = pu1outMsg + HDR_END_V4_OFFSET;
    } else {
        pu1outBody = pu1outMsg + HDR_END_V6_OFFSET;
    }

    /***************************************/
    /* get corresponding asv fields */
    /***************************************/
    /* Initialize */
    i4len = 0;
    i4pos = 0;

    /* ASV ID */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos = DDP_FIELD_LEN_ASV_DEFINED;
        i4len = DDP_FIELD_LEN_ASV_ID;
        i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_ID, pu1outBody + i4pos, &i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : get asv id fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_INVALID_FIELD_ASV_GET_ID_IN_DATA;
        }
    }

    /* ASV Priority */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_ASV_PRI;
        i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_PRI, pu1outBody + i4pos, &i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : get asv priority fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_INVALID_FIELD_ASV_GET_PRIORITY_IN_DATA;
        }
    }

    /* ASV State */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_ASV_STATE;
        i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_STATE, pu1outBody + i4pos, &i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : get asv state fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_INVALID_FIELD_ASV_GET_STATE_IN_DATA;
        }
    }
    /* ASV defined */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        if (*(pu1outBody + i4pos) != 0 && *(pu1outBody + i4pos) != 1) {
            *(pu1outBody) = 0;
            stDDP_Hdr_out.bodyLen = 1;
            goto process_asv_get_over;
        } else {
            *(pu1outBody) = 1;
        }
    }

    /* ASV tag port list type */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_ASV_TAG_LIST_TYPE;
        i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_TAG_PLIST_TYPE, pu1outBody + i4pos, &i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : get ASV tag port list type fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_INVALID_FIELD_ASV_GET_TLIST_TYPE_IN_DATA;
        }
    }

    /* ASV tag port list */
    if (i4ret == DDP_ALL_FIELD_SUCCESS) {
        i4pos += i4len;
        i4len = DDP_FIELD_LEN_ASV_TAG_PLIST;
        i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_TAG_PLIST, pu1outBody + i4pos, &i4len);
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            DDP_DEBUG_SWITCH("\n%s (%d) : get ASV tag post list fail (error %d)\n", __FILE__, __LINE__, i4ret);
            stDDP_Hdr_out.retCode = DDP_INVALID_FIELD_ASV_GET_TLIST_IN_DATA;
        }
    }

process_asv_get_over:
    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("Header of output packet\n");
        print_unpack_header(&stDDP_Hdr_out);
    }
    pack_header(pu1outMsg, &stDDP_Hdr_out);

    memset(&stDDP_Msg_pkt, 0, sizeof(stDDP_Msg_pkt));
    stDDP_Msg_pkt.ifindex = pstDDP_ifs->ifindex;
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        inAddr = (struct sockaddr_in*)&pstDDP_Msg_in->sender;
        outAddr = (struct sockaddr_in*)&stDDP_Msg_pkt.sender;
        outAddr->sin_family = inAddr->sin_family;
        memcpy(&outAddr->sin_addr, &inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        inAddr6 = (struct sockaddr_in6*)&pstDDP_Msg_in->sender;
        outAddr6 = (struct sockaddr_in6*)&stDDP_Msg_pkt.sender;
        outAddr6->sin6_family = inAddr6->sin6_family;
        memcpy(&outAddr6->sin6_addr, &inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(pu1outMsg, i4outMsgLen);
    }
    sendout_msg(&stDDP_Msg_pkt, pu1outMsg, i4outMsgLen);

    if (pu1outMsg) { free(pu1outMsg); pu1outMsg = NULL; }
    return i4ret;
}

/* i4fnDDP_proto_switch_asv_usr_mac_cfg (OP: 0xD5 (213))
 *   function to process asv user defined mac cfg packet and
 *               send through the incoming interfaces
 *
 *   pstDDP_ifs : pointer to the ddp interface structure
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan cfg msg structure
 *
 *   return : 0 -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_asv_usr_mac_cfg
(
    struct ddp_interface* pstDDP_ifs,
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    struct ddp_header stDDP_Hdr_out;
    struct ddp_message stDDP_Msg_pkt;
    struct ddp_exe_status stDDP_ErrStatus_out;

    INT4 i4ret = DDP_ALL_FIELD_SUCCESS;
    INT4 i4outMsgLen = 0;
    INT4 i4len = 0;
    INT4 i4pos = 0;
    INT1 encUsername[DDP_FIELD_LEN_USERNAME];
    INT1 encPassword[DDP_FIELD_LEN_PASSWORD];
    UINT1* pu1inBody = NULL;
    UINT1* pu1outMsg = NULL;
    UINT1* pu1outBody = NULL;

    if (pstDDP_ifs == NULL || pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL)
    {
        return -1;
    }

    /* verify input message */
    if (i4fnDDP_proto_switch_asv_usr_mac_cfg_check_header(pstDDP_Hdr_in, pstDDP_Msg_in) != DDP_RETCODE_OK)
    {
        return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET;
    }

    memset(&stDDP_ErrStatus_out, 0, sizeof(struct ddp_exe_status));
    memset(&stDDP_Hdr_out, 0, sizeof(struct ddp_header));
    stDDP_Hdr_out.ipVer = pstDDP_Hdr_in->ipVer;
    stDDP_Hdr_out.seq = pstDDP_Hdr_in->seq;
    stDDP_Hdr_out.opcode = pstDDP_Hdr_in->opcode;
    memcpy(stDDP_Hdr_out.macAddr, pstDDP_ifs->macAddr, MAC_ADDRLEN);
    stDDP_Hdr_out.protoVer = DDP_PROTO_V2;
    stDDP_Hdr_out.bodyLen = DDP_REPLY_LEN_ASV_USR_DEF_MAC_CFG;
    /* Fill in ipv4 or ipv6 address */
    if (pstDDP_Hdr_in->ipVer == IPV4_FLAG) {
        stDDP_Hdr_out.identifier = IPV4_REPLY;
        i4len = IPV4_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IP_ADDR, stDDP_Hdr_out.ipAddr.ipv4Addr, &i4len);
        if (pstDDP_Hdr_in->bodyLen > 0) { pu1inBody = pstDDP_Msg_in->payload + HDR_END_V4_OFFSET; }
        i4outMsgLen = HDR_END_V4_OFFSET + stDDP_Hdr_out.bodyLen;
    } else {
        stDDP_Hdr_out.identifier = IPV6_REPLY;
        i4len = IPV6_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IPV6_ADDR, stDDP_Hdr_out.ipAddr.ipv6Addr, &i4len);
        if (pstDDP_Hdr_in->bodyLen > 0) { pu1inBody = pstDDP_Msg_in->payload + HDR_END_V6_OFFSET; }
        i4outMsgLen = HDR_END_V6_OFFSET + stDDP_Hdr_out.bodyLen;
    }

    /* allocate output message */
    pu1outMsg = (UINT1*)malloc(i4outMsgLen);

    if (pu1outMsg == NULL) {
        DDP_DEBUG_SWITCH("fail to allocate asv cfg output request msg\n");
        return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET;
    }

    memset(pu1outMsg, 0, i4outMsgLen);
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        pu1outBody = pu1outMsg + HDR_END_V4_OFFSET;
    } else {
        pu1outBody = pu1outMsg + HDR_END_V6_OFFSET;
    }

    /*******************************************************/
    /* configure corresponding asv user defined mac*/
    /*******************************************************/
    if (pu1inBody == NULL) {
        DDP_DEBUG_SWITCH("Invalid packet body.\n");
        if(pu1outMsg != NULL)
        {
            free(pu1outMsg);
            pu1outMsg = NULL;
        }
        return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET;
    }

    i4len = 0; i4pos = 0;
    /* retrieve username and password from msg body and verify them */
    if (ddp_proto_check_op_authen(DDP_OP_ASV_USR_MAC_CFG) == DDP_OP_AUTHEN_ON) {
        if (i4ret == DDP_ALL_FIELD_SUCCESS && pu1inBody != NULL) {
            memset(encUsername, 0, sizeof(encUsername));
            memset(encPassword, 0, sizeof(encPassword));
            memcpy(encUsername, pu1inBody, DDP_FIELD_LEN_USERNAME);
            memcpy(encPassword, pu1inBody + DDP_FIELD_LEN_USERNAME, DDP_FIELD_LEN_PASSWORD);
            i4ret = ddp_proto_verify_authen(pstDDP_ifs, encUsername, encPassword);
        } else {
            i4ret = -2;
        }
        if (i4ret != DDP_ALL_FIELD_SUCCESS) {
            stDDP_Hdr_out.retCode = DDP_RETCODE_AUTHEN_FAIL;
            goto process_asv_usr_mac_cfg_over;
        }
        i4pos = DDP_FIELD_LEN_USERNAME + DDP_FIELD_LEN_PASSWORD;
    }
    /*
     * DDP_REQ_LEN_ASV_USR_DEF_MAC_CFG: 29 = sizeof(struct stDDP_PF_ASV_USR_MAC)
     *
     */
    i4ret = ddp_platform_set_field(pstDDP_ifs, DDP_FIELD_ASV_USR_DEF_MAC, pu1inBody + i4pos, sizeof(struct stDDP_PF_ASV_USR_MAC));

    if (i4ret != DDP_ALL_FIELD_SUCCESS) {
        DDP_DEBUG_SWITCH("\n%s (%d) : set asv mac address fail (error %d)\n", __FILE__, __LINE__, i4ret);
        stDDP_Hdr_out.retCode = DDP_RETCODE_FAIL;
        stDDP_ErrStatus_out.errNo = DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET;

    }

process_asv_usr_mac_cfg_over:
    memset(&stDDP_Msg_pkt, 0, sizeof(struct ddp_message));
    stDDP_Msg_pkt.ifindex = pstDDP_ifs->ifindex;

    if( i4fnDDP_proto_switch_pack_reply_msg(&stDDP_Hdr_out, pstDDP_Msg_in, &stDDP_ErrStatus_out,
        &stDDP_Msg_pkt, pu1outMsg, i4outMsgLen, pu1outBody) != DDP_RETCODE_OK)
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : pack reply msg fail.\n", __FILE__, __LINE__);
    }

    sendout_msg(&stDDP_Msg_pkt, pu1outMsg, i4outMsgLen);

    if (pu1outMsg) {
        free(pu1outMsg);
        pu1outMsg = NULL;
    }

    return i4ret;
}

/* i4fnDDP_proto_switch_asv_usr_mac_get (OP: 0xD6 (214))
 *   function to process asv user defined mac get packet and
 *               send through the incoming interfaces
 *
 *   pstDDP_ifs : pointer to the ddp interface structure
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input asv mac get msg structure
 *
 *   return : 0 -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_asv_usr_mac_get
(
    struct ddp_interface* pstDDP_ifs,
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    struct ddp_header stDDP_Hdr_out;
    struct ddp_message stDDP_Msg_pkt;
    struct ddp_exe_status stDDP_ErrStatus_out;

    INT4 i4ret = DDP_ALL_FIELD_SUCCESS;
    INT4 i4outMsgLen = 0;
    INT4 i4len = 0;
    UINT1* pu1outMsg = NULL;
    UINT1* pu1outBody = NULL;

    if (pstDDP_ifs == NULL || pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL)
    {
        return -1;
    }

    /* verify input message */
    if (i4fnDDP_proto_switch_asv_usr_mac_get_check_header(pstDDP_Hdr_in, pstDDP_Msg_in) != DDP_RETCODE_OK)
    {
        return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_GET;
    }

    memset(&stDDP_ErrStatus_out, 0, sizeof(struct ddp_exe_status));
    memset(&stDDP_Hdr_out, 0, sizeof(struct ddp_header));
    stDDP_Hdr_out.ipVer = pstDDP_Hdr_in->ipVer;
    stDDP_Hdr_out.seq = pstDDP_Hdr_in->seq;
    stDDP_Hdr_out.opcode = pstDDP_Hdr_in->opcode;
    memcpy(stDDP_Hdr_out.macAddr, pstDDP_ifs->macAddr, MAC_ADDRLEN);
    stDDP_Hdr_out.protoVer = DDP_PROTO_V2;
    stDDP_Hdr_out.bodyLen = DDP_REPLY_LEN_ASV_USR_DEF_MAC_GET+1;

    /* Fill in ipv4 or ipv6 address */
    if (pstDDP_Hdr_in->ipVer == IPV4_FLAG) {
        stDDP_Hdr_out.identifier = IPV4_REPLY;
        i4len = IPV4_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IP_ADDR, stDDP_Hdr_out.ipAddr.ipv4Addr, &i4len);
        i4outMsgLen = HDR_END_V4_OFFSET + stDDP_Hdr_out.bodyLen;
    } else {
        stDDP_Hdr_out.identifier = IPV6_REPLY;
        i4len = IPV6_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IPV6_ADDR, stDDP_Hdr_out.ipAddr.ipv6Addr, &i4len);
        i4outMsgLen = HDR_END_V6_OFFSET + stDDP_Hdr_out.bodyLen;
    }

    /* allocate output message */
    pu1outMsg = (UINT1*)malloc(i4outMsgLen);

    if (pu1outMsg == NULL) {
        DDP_DEBUG_SWITCH("fail to allocate asv mac get output  msg\n");
        return -3;
    }

    memset(pu1outMsg, 0, i4outMsgLen);
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        pu1outBody = pu1outMsg + HDR_END_V4_OFFSET;
    } else {
        pu1outBody = pu1outMsg + HDR_END_V6_OFFSET;
    }

    /*******************************************************/
    /* get asv user defined mac fields */
    /*******************************************************/
    /* Initialize */
    i4len = DDP_REPLY_LEN_ASV_USR_DEF_MAC_GET+1;

    /*
     * DDP_REQ_LEN_ASV_USR_DEF_MAC_CFG: 29 = sizeof(struct stDDP_PF_ASV_USR_MAC)
     *
     */
    i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_ASV_USR_DEF_MAC, pu1outBody, &i4len);

    if (i4ret != DDP_ALL_FIELD_SUCCESS) {
        DDP_DEBUG_SWITCH("\n%s (%d) : set asv mac address fail (error %d)\n", __FILE__, __LINE__, i4ret);
        stDDP_Hdr_out.retCode = DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET;
        stDDP_ErrStatus_out.errNo = i4ret;

    }

    memset(&stDDP_Msg_pkt, 0, sizeof(struct ddp_message));
    stDDP_Msg_pkt.ifindex = pstDDP_ifs->ifindex;

    /* Update Message length */
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        i4outMsgLen = HDR_END_V4_OFFSET + i4len;
    } else {
        i4outMsgLen = HDR_END_V6_OFFSET + i4len;
    }

    if( i4fnDDP_proto_switch_pack_reply_msg(&stDDP_Hdr_out, pstDDP_Msg_in, &stDDP_ErrStatus_out,
        &stDDP_Msg_pkt, pu1outMsg, i4outMsgLen, pu1outBody) != DDP_RETCODE_OK)
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : pack reply msg fail.\n", __FILE__, __LINE__);
    }

    sendout_msg(&stDDP_Msg_pkt, pu1outMsg, i4outMsgLen);

    if (pu1outMsg) {
        free(pu1outMsg);
        pu1outMsg = NULL;
    }

    return i4ret;
}

/* i4fnDDP_proto_switch_get_port_number (OP: 0xD7 (215))
 *   function to process get device port number packet and
 *               send through the incoming interfaces
 *
 *   pstDDP_ifs : pointer to the ddp interface structure
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input get msg structure
 *
 *   return : 0 -> success, others -> error
 */
INT4
i4fnDDP_proto_switch_get_port_number
(
    struct ddp_interface* pstDDP_ifs,
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    struct ddp_header stDDP_Hdr_out;
    struct ddp_message stDDP_Msg_pkt;
    struct ddp_exe_status stDDP_ErrStatus_out;

    INT4 i4ret = DDP_ALL_FIELD_SUCCESS;
    INT4 i4outMsgLen = 0;
    INT4 i4len = 0;
    UINT1* pu1outMsg = NULL;
    UINT1* pu1outBody = NULL;

    DDP_DEBUG_SWITCH("\n====Enter %s\n", __FUNCTION__);

    if (pstDDP_ifs == NULL || pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL)
    {
        DDP_DEBUG_SWITCH("%s: line<%d> Invalid input\n", __FUNCTION__, __LINE__);
        return DDP_RET_SWITCH_ERROR_PARAMETER;
    }

    /* verify input message */
    if (i4fnDDP_proto_switch_get_port_number_check_header(pstDDP_Hdr_in, pstDDP_Msg_in) != DDP_RETCODE_OK)
    {
        DDP_DEBUG_SWITCH("\t line<%d> Invalid header\n", __LINE__);
        return DDP_INVALID_FIELD_PORT_NUM_GET;
    }

    memset(&stDDP_ErrStatus_out, 0, sizeof(struct ddp_exe_status));
    memset(&stDDP_Hdr_out, 0, sizeof(struct ddp_header));
    stDDP_Hdr_out.ipVer = pstDDP_Hdr_in->ipVer;
    stDDP_Hdr_out.seq = pstDDP_Hdr_in->seq;
    stDDP_Hdr_out.opcode = pstDDP_Hdr_in->opcode;
    memcpy(stDDP_Hdr_out.macAddr, pstDDP_ifs->macAddr, MAC_ADDRLEN);
    stDDP_Hdr_out.protoVer = DDP_PROTO_V2;
    stDDP_Hdr_out.bodyLen = DDP_REPLY_LEN_PORT_NUMBER_GET+1;

    /* Fill in ipv4 or ipv6 address */
    if (pstDDP_Hdr_in->ipVer == IPV4_FLAG) {
        stDDP_Hdr_out.identifier = IPV4_REPLY;
        i4len = IPV4_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IP_ADDR, stDDP_Hdr_out.ipAddr.ipv4Addr, &i4len);
        i4outMsgLen = HDR_END_V4_OFFSET + stDDP_Hdr_out.bodyLen;
    } else {
        stDDP_Hdr_out.identifier = IPV6_REPLY;
        i4len = IPV6_ADDRLEN;
        ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_DEVICE_IPV6_ADDR, stDDP_Hdr_out.ipAddr.ipv6Addr, &i4len);
        i4outMsgLen = HDR_END_V6_OFFSET + stDDP_Hdr_out.bodyLen;
    }

    /* allocate output message */
    pu1outMsg = (UINT1*)malloc(i4outMsgLen);

    if (pu1outMsg == NULL) {
        DDP_DEBUG_SWITCH("fail to allocate asv mac get output  msg\n");
        return -3;
    }

    memset(pu1outMsg, 0, i4outMsgLen);
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        pu1outBody = pu1outMsg + HDR_END_V4_OFFSET;
    } else {
        pu1outBody = pu1outMsg + HDR_END_V6_OFFSET;
    }

    /*******************************************************/
    /* get device port number fields */
    /*******************************************************/
    /* Initialize */
    i4len = DDP_REPLY_LEN_PORT_NUMBER_GET+1;

    i4ret = ddp_platform_get_field(pstDDP_ifs, DDP_FIELD_PORT_NUM_GET, pu1outBody, &i4len);

    if (i4ret != DDP_ALL_FIELD_SUCCESS) {
        DDP_DEBUG_SWITCH("\n%s (%d) : get device port number fail (error %d)\n", __FILE__, __LINE__, i4ret);
        stDDP_Hdr_out.retCode = DDP_INVALID_FIELD_PORT_NUM_GET;
        stDDP_ErrStatus_out.errNo = i4ret;

    }

    memset(&stDDP_Msg_pkt, 0, sizeof(struct ddp_message));
    stDDP_Msg_pkt.ifindex = pstDDP_ifs->ifindex;

    /* Update Message length */
    if (stDDP_Hdr_out.ipVer == IPV4_FLAG) {
        i4outMsgLen = HDR_END_V4_OFFSET + i4len;
    } else {
        i4outMsgLen = HDR_END_V6_OFFSET + i4len;
    }

    if( i4fnDDP_proto_switch_pack_reply_msg(&stDDP_Hdr_out, pstDDP_Msg_in, &stDDP_ErrStatus_out,
        &stDDP_Msg_pkt, pu1outMsg, i4outMsgLen, pu1outBody) != DDP_RETCODE_OK)
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : pack reply msg fail.\n", __FILE__, __LINE__);
    }

    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("\n======== %s: line<%d> ==============\n", __FUNCTION__, __LINE__);
        printf("After packing, output MSG in hex, len<%d>\n", i4outMsgLen);
        print_message_hex(pu1outMsg, i4outMsgLen);
    }

    sendout_msg(&stDDP_Msg_pkt, pu1outMsg, i4outMsgLen);

    if (pu1outMsg) {
        free(pu1outMsg);
        pu1outMsg = NULL;
    }

    return i4ret;
}

/* i4fnDDP_proto_switch_vlan_cfg_check_header
 *   function to check the content of vlan cfg header
 *
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan cfg msg structure
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_vlan_cfg_check_header(
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    if ( pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL )
    {
        return DDP_RETCODE_FAIL;
    }

    /* verify input message */
    if (pstDDP_Hdr_in->bodyLen != DDP_REQ_LEN_VLAN_CFG || pstDDP_Hdr_in->opcode != DDP_OP_VLAN_CFG
        || pstDDP_Msg_in->payload == NULL
        || (pstDDP_Hdr_in->ipVer != IPV4_FLAG && pstDDP_Hdr_in->ipVer != IPV6_FLAG))
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : Invalid vlan request header (error %d)\n", __FILE__, __LINE__, DDP_RETCODE_FAIL);
        return DDP_RETCODE_FAIL;
    }

    return DDP_RETCODE_OK;
}

/* i4fnDDP_proto_switch_asv_cfg_check_header
 *   function to check the content of asv cfg header
 *
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan cfg msg structure
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_asv_cfg_check_header(
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    if ( pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL )
    {
        return DDP_RETCODE_FAIL;
    }

    /* verify input message */
    if (pstDDP_Hdr_in->bodyLen != DDP_REQ_LEN_ASV_CFG || pstDDP_Hdr_in->opcode != DDP_OP_ASV_CFG
        || pstDDP_Msg_in->payload == NULL
        || (pstDDP_Hdr_in->ipVer != IPV4_FLAG && pstDDP_Hdr_in->ipVer != IPV6_FLAG))
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : Invalid asv request header (error %d)\n", __FILE__, __LINE__, DDP_RETCODE_FAIL);
        return DDP_RETCODE_FAIL;
    }

    return DDP_RETCODE_OK;
}

/* i4fnDDP_proto_switch_get_port_number_check_header
 *   function to check the content of get port number header
 *
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input get port number msg structure
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_get_port_number_check_header(
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    if ( pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL )
    {
        return DDP_RETCODE_FAIL;
    }

    /* verify input message */
    if (pstDDP_Hdr_in->bodyLen != DDP_REQ_LEN_GET_SYS_PORT_NUM || pstDDP_Hdr_in->opcode != DDP_OP_GET_SYS_PORT_NUM
        || pstDDP_Msg_in->payload == NULL
        || (pstDDP_Hdr_in->ipVer != IPV4_FLAG && pstDDP_Hdr_in->ipVer != IPV6_FLAG))
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : Invalid get port number request header (error %d)\n", __FILE__, __LINE__, DDP_RETCODE_FAIL);
        return DDP_RETCODE_FAIL;
    }

    return DDP_RETCODE_OK;
}

/* i4fnDDP_proto_switch_asv_usr_mac_cfg_check_header
 *   function to check the content of asv user defined mac cfg header
 *
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input vlan cfg msg structure
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_asv_usr_mac_cfg_check_header(
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    if ( pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL )
    {
        return DDP_RETCODE_FAIL;
    }

    /* verify input message */
    if (pstDDP_Hdr_in->bodyLen != DDP_REQ_LEN_ASV_USR_DEF_MAC_CFG || pstDDP_Hdr_in->opcode != DDP_OP_ASV_USR_MAC_CFG
        || pstDDP_Msg_in->payload == NULL
        || (pstDDP_Hdr_in->ipVer != IPV4_FLAG && pstDDP_Hdr_in->ipVer != IPV6_FLAG))
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : Invalid asv user defined mac request header (error %d)\n", __FILE__, __LINE__, DDP_RETCODE_FAIL);
        return DDP_RETCODE_FAIL;
    }

    return DDP_RETCODE_OK;
}

/* i4fnDDP_proto_switch_asv_usr_mac_get_check_header
 *   function to check the content of asv user defined mac get header
 *
 *   pstDDP_Hdr_in : pointer to the ddp input header structure
 *   pstDDP_Msg_in : pointer to the ddp input usr asv mac get msg structure
 *
 *   return : 0 -> DDP_RETCODE_OK, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_asv_usr_mac_get_check_header(
    struct ddp_header* pstDDP_Hdr_in,
    struct ddp_message* pstDDP_Msg_in
)
{
    if ( pstDDP_Hdr_in == NULL || pstDDP_Msg_in == NULL )
    {
        return DDP_RETCODE_FAIL;
    }

    /* verify input message */
    if (pstDDP_Hdr_in->bodyLen != DDP_REQ_LEN_ASV_USR_DEF_MAC_GET || pstDDP_Hdr_in->opcode != DDP_OP_ASV_USR_MAC_GET
        || pstDDP_Msg_in->payload == NULL
        || (pstDDP_Hdr_in->ipVer != IPV4_FLAG && pstDDP_Hdr_in->ipVer != IPV6_FLAG))
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : Invalid asv user defined mac get header (error %d)\n", __FILE__, __LINE__, DDP_RETCODE_FAIL);
        return DDP_RETCODE_FAIL;
    }

    return DDP_RETCODE_OK;
}

/* i4fnDDP_proto_switch_pack_reply_msg
 *   function to pack up the reply msg
 *
 *   return : DDP_RETCODE_OK -> success, DDP_RETCODE_FAIL -> error
 */
INT4
i4fnDDP_proto_switch_pack_reply_msg(
    struct ddp_header* pstDDP_Hdr_out,
    struct ddp_message* pstDDP_Msg_in,
    struct ddp_exe_status* pstDDP_ErrStatus_out,
    struct ddp_message* pstDDP_Msg_pkt,
    UINT1* pu1outMsg,
    INT4 i4outMsgLen,
    UINT1* pu1outBody
)
{
    struct sockaddr_in* pstAddr_inAddr = NULL;
    struct sockaddr_in* pstAddr_outAddr = NULL;
    struct sockaddr_in6* pstAddr6_inAddr6 = NULL;
    struct sockaddr_in6* pstAddr6_outAddr6 = NULL;

    if(pstDDP_Hdr_out == NULL || pstDDP_Msg_in == NULL || pstDDP_ErrStatus_out == NULL
       || pu1outMsg == NULL || pu1outBody == NULL)
    {
        return DDP_RETCODE_FAIL;
    }

    /* packing */
    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HDR) {
        printf("\nHeader of output packet\n");
        print_unpack_header(pstDDP_Hdr_out);
    }

    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_EXEST) {
        print_exe_status(pstDDP_ErrStatus_out);
    }

    pack_header(pu1outMsg, pstDDP_Hdr_out);

    if (pstDDP_Hdr_out->ipVer == IPV4_FLAG) {
        pstAddr_inAddr = (struct sockaddr_in*)&(pstDDP_Msg_in->sender);
        pstAddr_outAddr = (struct sockaddr_in*)&(pstDDP_Msg_pkt->sender);
        pstAddr_outAddr->sin_family = pstAddr_inAddr->sin_family;
        memcpy(&pstAddr_outAddr->sin_addr, &pstAddr_inAddr->sin_addr, IPV4_ADDRLEN);
        //memcpy(&pstAddr_outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        pstAddr_outAddr->sin_port = htons(UDP_PORT_SERVER);
    } else {
        pstAddr6_inAddr6 = (struct sockaddr_in6*)&(pstDDP_Msg_in->sender);
        pstAddr6_outAddr6 = (struct sockaddr_in6*)&(pstDDP_Msg_pkt->sender);
        pstAddr6_outAddr6->sin6_family = pstAddr6_inAddr6->sin6_family;
        memcpy(&pstAddr6_outAddr6->sin6_addr, &pstAddr6_inAddr6->sin6_addr, IPV6_ADDRLEN);
        //memcpy(&outAddr->sin_addr, IPV4_BRCAST, IPV4_ADDRLEN);
        pstAddr6_outAddr6->sin6_port = htons(UDP_PORT_SERVER);
    }

    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        print_message_hex(pu1outMsg, i4outMsgLen);
    }

    return DDP_RETCODE_OK;
}

