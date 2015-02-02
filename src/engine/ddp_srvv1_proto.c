
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ddp.h"
#include "ddp_platform.h"

INT4 ddp_srvv1_proto_discovery(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);

/* function api used */
/* ddp_proto_basic.c */
//INT4 ddp_proto_general_report(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_discovery(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_set_basic_info(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_user_verify(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_change_id_psw(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_query_support_opt(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_device_alert_info(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_snmp_cfg(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_snmp_get(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_reboot(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_ddp_info(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
///* ddp_proto_opt.c */
//INT4 ddp_proto_reset(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_factory_reset(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_query_neighbor_info(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_get_sys_date_time(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_set_sys_date_time(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_set_ipv4_addr(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_set_ipv6_addr(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
//INT4 ddp_proto_set_device_info(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);

/* ddp_proto_filter
 *   function to verify the incoming message is sent for this interface of the machine
 *
 *   hdr : header
 *   ifs : interface
 *
 *   return : 0 -> no, 1 -> yes
 */
//INT4
//ddp_proto_filter
//(
//    struct ddp_header* hdr,
//    struct ddp_interface* ifs
//)
//{
//    INT4 ret = 0;
//
//    if (hdr == NULL) { return -1; }
//    /* TRADEOFF : utility may send packet in proto v1 */
//    //if (hdr->protoVer != DDP_PROTO_V2) { return -2; }
//    if (hdr->identifier == IPV4_REQ || hdr->identifier == IPV6_REQ) {
//        if (hdr->retCode == REQ_MODE_UNICAST) {
//            if (ifs) {
//                if (memcmp(hdr->macAddr, ifs->macAddr, MAC_ADDRLEN) == 0) { ret = 1; }
//            }
//        }
//        else if (hdr->retCode == 0x0000) {
//            if (memcmp(hdr->macAddr, (void*)&MAC_ALL, MAC_ADDRLEN) == 0) {
//                ret = 1;
//            } else {
//                if (ifs) {
//                    if (memcmp(hdr->macAddr, ifs->macAddr, MAC_ADDRLEN) == 0) { ret = 1; }
//                }
//            }
//        }
//    }
//
//    return ret;
//}

/* ddp_proto_query_op_info
 *   funciton to query opcode information
 *
 *   opcode : opcode
 *
 *   return : op_info struct or NULL (no match opcode)
 */
//struct op_info*
//ddp_proto_query_op_info
//(
//    UINT2 opcode
//)
//{
//    INT4 idx = 0;
//    struct op_info* tmp = NULL;
//
//    while (g_opList[idx].opcode != 0) {
//        if (opcode == g_opList[idx].opcode && g_opList[idx].opcode != 0) {
//            tmp = &g_opList[idx];
//            break;
//        }
//        idx++;
//    }
//
//    return tmp;
//}

/* ddp_proto_check_op_authen
 *   function to query whether input opcode requires authentication
 *
 *   opcode : opcode
 *
 *   return : DDP_OP_AUTHEN_ON or DDP_OP_AUTHEN_OFF
 */
//UINT1
//ddp_proto_check_op_authen
//(
//    UINT2 opcode
//)
//{
//    struct op_info* op = NULL;
//
//    op = ddp_proto_query_op_info(opcode);
//    if (op == NULL) {
//        return 0;
//    }
//
//    return op->authen;
//}

/* ddp_proto_check_op_enable
 *   function to query whether this opcode is enabled on not
 *
 *   opcode : opcode
 *
 *   return : DDP_OP_ENABLE or DDP_OP_DISABEL
 */
//UINT1
//ddp_proto_check_op_enable
//(
//    UINT2 opcode
//)
//{
//    struct op_info* op = NULL;
//
//    op = ddp_proto_query_op_info(opcode);
//    if (op == NULL) {
//        return 0;
//    }
//
//    return op->enable;
//}

/* ddp_proto_query_user_info
 *   get user information of the specified user name from platform
 *
 *   ifs : interface
 *   user : struct to hold user information
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_query_user_info
//(
//    struct ddp_interface* ifs,
//    struct ddp_user* user
//)
//{
//    INT4 ret = 0;
//    if (ifs == NULL || user == NULL) { return -1; }
//    ret = ddp_platform_get_user(ifs, user);
//    return ret;
//}

/* ddp_proto_query_user_type
 *   function to get the user type of the specified user name
 *
 *   ifs : interface
 *   encUsername : encoded user name
 *   encPassword : encoded password
 *
 *   return : DDP_USER_TYPE_ADMIN, DDP_USER_TYPE_USER, or others (no match)
 */
//INT4
//ddp_proto_query_user_type
//(
//    struct ddp_interface* ifs,
//    INT1* encUsername,
//    INT1* encPassword
//)
//{
//    INT4 ret = 0;
//    INT4 encMethod = DDP_ENC_BASE64;
//    struct ddp_user inUser;
//    struct ddp_user origUser;
//
//    if (encUsername == NULL || encPassword == NULL) { return -2; }
//
//    memset(&inUser, 0, sizeof(inUser));
//    memset(&origUser, 0, sizeof(origUser));
//    if (string_decode(encUsername, inUser.name, sizeof(inUser.name), encMethod) <= 0) {
//        return -3;
//    }
//    if (string_decode(encPassword, inUser.pass, sizeof(inUser.pass), encMethod) <= 0) {
//        return -4;
//    }
//    DDP_DEBUG("in name %s,  pawd %s\n", inUser.name, inUser.pass);
//    strcpy(origUser.name, inUser.name);
//    ret = ddp_proto_query_user_info(ifs, &origUser);
//    if (ret != 0) { return -5; }
//
//    if (strcmp(origUser.pass, inUser.pass) != 0) {
//        ret = -1;
//    } else {
//        ret = (INT4)origUser.type;
//    }
//    return ret;
//}

/* ddp_proto_verify_authen
 *   function to do authentication of input user name and password
 *
 *   ifs : interface
 *   encUsername : encoded user name
 *   encPassword : encoded password
 *
 *   return : 0 -> pass, others -> fail
 */
//INT4
//ddp_proto_verify_authen
//(
//    struct ddp_interface* ifs,
//    INT1* encUsername,
//    INT1* encPassword
//)
//{
//    INT4 ret = 0;
//    INT4 encMethod = DDP_ENC_BASE64;
//    struct ddp_user inUser;
//    struct ddp_user origUser;
//
//    if (encUsername == NULL || encPassword == NULL) { return -2; }
//
//    memset(&inUser, 0, sizeof(inUser));
//    memset(&origUser, 0, sizeof(origUser));
//    if (string_decode(encUsername, inUser.name, sizeof(inUser.name), encMethod) <= 0) {
//        return -3;
//    }
//    if (string_decode(encPassword, inUser.pass, sizeof(inUser.pass), encMethod) <= 0) {
//        return -4;
//    }
//    DDP_DEBUG("req : name %s ,  pawd %s\n", inUser.name, inUser.pass);
//    strcpy(origUser.name, inUser.name);
//    ret = ddp_proto_query_user_info(ifs, &origUser);
//    if (ret != 0) { return -5; }
//
//    if (strcmp(origUser.pass, inUser.pass) != 0) {
//        DDP_DEBUG("%s (%d) : authen fail", __FILE__, __LINE__);
//        ret = -1;
//    } else {
//        ret = DDP_ALL_FIELD_SUCCESS;
//    }
//    return ret;
//}

/* ddp_proto_process_message
 *   dispatch function to process message
 *
 *   inMsg : input raw message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_srvv1_proto_process_message
(
    struct ddp_message* inMsg
)
{
    INT4 retVal = 0;
    struct ddp_interface* ifs = NULL;
    struct ddp_header inHdr;

    /* extract header */
    extract_header(inMsg->payload, inMsg->size, &inHdr);

    /* print out header after parsing */
    if (g_debugFlag & DDP_DEBUG_PRINT_IN_MSG_HDR) {
        printf("Header of input packet\n");
        print_unpack_header(&inHdr);
    }

    if (inMsg->ifindex == 0xFFFFFFFF) {
        /* send through all interfaces */
    } else {
        ifs = ddp_platform_search_if_by_index(g_ifList, inMsg->ifindex);
        if (ifs == NULL) {
            DDP_DEBUG("No interface is matched (if index %d) --> Drop it\n", inMsg->ifindex);
            retVal = -1;
            goto process_msg_over;
        }
    }

    /*if (ddp_proto_filter(&inHdr, ifs) != 1) {
        DDP_DEBUG("Req is not for this host/interface --> Drop it\n")
        goto process_msg_over;
    }*/
    /* op code */    
    switch (inHdr.opcode) {
//        case DDP_OP_GENERAL_REPORT:
//            if (ddp_proto_check_op_enable(DDP_OP_GENERAL_REPORT) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_general_report(ifs, &inHdr, inMsg);
//            }
//            break;
        case DDP_OP_DISCOVERY:
            //if (ddp_proto_check_op_enable(DDP_OP_DISCOVERY) == DDP_OP_ENABLE) {
                retVal = ddp_srvv1_proto_discovery(ifs, &inHdr, inMsg);
            //}
            break;
//        case DDP_OP_SET_BASIC_INFO:
//            if (ddp_proto_check_op_enable(DDP_OP_SET_BASIC_INFO) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_set_basic_info(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_USER_VERIFY:
//            if (ddp_proto_check_op_enable(DDP_OP_USER_VERIFY) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_user_verify(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_CHANGE_ID_PSW:
//            if (ddp_proto_check_op_enable(DDP_OP_CHANGE_ID_PSW) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_change_id_psw(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_QUERY_SUPPORT_OPT:
//            if (ddp_proto_check_op_enable(DDP_OP_QUERY_SUPPORT_OPT) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_query_support_opt(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_DEVICE_ALERT_INFO:
//            if (ddp_proto_check_op_enable(DDP_OP_DEVICE_ALERT_INFO) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_device_alert_info(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_REBOOT:
//            if (ddp_proto_check_op_enable(DDP_OP_REBOOT) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_reboot(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_RESET:
//            if (ddp_proto_check_op_enable(DDP_OP_RESET) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_reset(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_FACTORY_RESET:
//            if (ddp_proto_check_op_enable(DDP_OP_FACTORY_RESET) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_factory_reset(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_QUERY_NEIGHBOR_INFO:
//            if (ddp_proto_check_op_enable(DDP_OP_QUERY_NEIGHBOR_INFO) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_query_neighbor_info(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_VLAN_CFG:
//            if (ddp_proto_check_op_enable(inHdr.opcode) == DDP_OP_ENABLE) {
//                retVal = i4fnDDP_proto_switch_vlan_cfg(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_VLAN_GET:
//            if (ddp_proto_check_op_enable(inHdr.opcode) == DDP_OP_ENABLE) {
//                retVal = i4fnDDP_proto_switch_vlan_get(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_ASV_CFG:
//            if (ddp_proto_check_op_enable(inHdr.opcode) == DDP_OP_ENABLE) {
//                retVal = i4fnDDP_proto_switch_asv_cfg(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_ASV_GET:
//            if (ddp_proto_check_op_enable(inHdr.opcode) == DDP_OP_ENABLE) {
//                retVal = i4fnDDP_proto_switch_asv_get(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_SNMP_CFG:
//            if (ddp_proto_check_op_enable(DDP_OP_SNMP_CFG) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_snmp_cfg(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_SNMP_GET:
//            if (ddp_proto_check_op_enable(DDP_OP_SNMP_GET) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_snmp_get(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_ASV_USR_MAC_CFG:
//            if (ddp_proto_check_op_enable(inHdr.opcode) == DDP_OP_ENABLE) {
//                retVal = i4fnDDP_proto_switch_asv_usr_mac_cfg(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_ASV_USR_MAC_GET:
//            if (ddp_proto_check_op_enable(inHdr.opcode) == DDP_OP_ENABLE) {
//                retVal = i4fnDDP_proto_switch_asv_usr_mac_get(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_GET_SYS_PORT_NUM:
//            if (ddp_proto_check_op_enable(inHdr.opcode) == DDP_OP_ENABLE) {
//                retVal = i4fnDDP_proto_switch_get_port_number(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_FW_UPGRADE:
//            if (ddp_proto_check_op_enable(DDP_OP_FW_UPGRADE) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_upgrade_fw(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_CFG_BACKUP:
//            if (ddp_proto_check_op_enable(DDP_OP_CFG_BACKUP) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_upgrade_cfg_backup(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_CFG_RESTORE:
//            if (ddp_proto_check_op_enable(DDP_OP_CFG_RESTORE) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_upgrade_cfg_restore(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_GET_SYS_DATE_TIME:
//            if (ddp_proto_check_op_enable(DDP_OP_GET_SYS_DATE_TIME) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_get_sys_date_time(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_SET_SYS_DATE_TIME:
//            if (ddp_proto_check_op_enable(DDP_OP_SET_SYS_DATE_TIME) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_set_sys_date_time(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_SET_MULTI_SSID:
//            if (ddp_proto_check_op_enable(DDP_OP_SET_MULTI_SSID) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_set_multi_ssid(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_GET_MULTI_SSID:
//            if (ddp_proto_check_op_enable(DDP_OP_GET_MULTI_SSID) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_get_multi_ssid(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_DDP_INFO:
//            if (ddp_proto_check_op_enable(DDP_OP_DDP_INFO) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_ddp_info(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_SET_IPV4_ADDR:
//            if (ddp_proto_check_op_enable(DDP_OP_SET_IPV4_ADDR) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_set_ipv4_addr(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_SET_IPV6_ADDR:
//            if (ddp_proto_check_op_enable(DDP_OP_SET_IPV6_ADDR) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_set_ipv6_addr(ifs, &inHdr, inMsg);
//            }
//            break;
//        case DDP_OP_SET_DEVICE_INFO:
//            if (ddp_proto_check_op_enable(DDP_OP_SET_DEVICE_INFO) == DDP_OP_ENABLE) {
//                retVal = ddp_proto_set_device_info(ifs, &inHdr, inMsg);
//            }
//            break;
        default:
            DDP_DEBUG("%s (%d) : opcode (%X) is not supported\n", __FILE__, __LINE__, inHdr.opcode);
            break;
    }

process_msg_over:
    DDP_DEBUG("----- REPLY over -----\n");
    return retVal;
}
