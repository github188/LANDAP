
#ifndef _DDP_PLATFORM_H_
#define _DDP_PLATFORM_H_

#include "ddp_general.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Enable this define to set product category as "Switch".
 */
//#define SWITCH_MODE

/* Enable this define to set product category as "NAS" with the capability of handling general report.
 * The current SMB-Utility has copyright protection against non-Cameo switch.
 * Developer should not set product category as "Switch", in order to observe the behavior of switch.
 */
//#define FAKE_SWITCH

/* Enable this define to set product category as "Access point".
 */
//#define AP_MODE

/* string of product category */
extern INT1 prodcat_str_switch[];
extern INT1 prodcat_str_ap[];
extern INT1 prodcat_str_netcam[];
extern INT1 prodcat_str_videoencoder[];
extern INT1 prodcat_str_nvr[];
extern INT1 prodcat_str_nas[];
extern INT1 prodcat_str_servroute[];
extern INT1 prodcat_str_wlc[];
extern INT1 prodcat_str_wls[];
extern INT1 prodcat_str_wlr[];
extern INT1 prodcat_str_epos[];
extern INT1 prodcat_str_aaa[];

/* Switch Info */
#define kDDP_SWITCH_PORT_NUMBER 12
#define kDDP_SWITCH_UPDATE_TTL 2
#define kDDP_SWITCH_MAX_ENTRY_PER_PACKET 5
#define kDDP_SWITCH_MAX_UNIT_PER_PACKET 32

/* 0x00D2 VLAN */
#define kDDP_PF_DEFAULT_VLAN_ID 1
#define ksDDP_PF_DEFAULT_VLAN_NAME 	"Default VLAN"
#define kDDP_PF_DEFAULT_VLAN_TAG_LIST_TYPE 1
#define kDDP_PF_DEFAULT_VLAN_UNTAG_LIST_TYPE 1

/* 0x00D3 ASV */
#define kDDP_PF_DEFAULT_ASV_ID 1
#define kDDP_PF_DEFAULT_ASV_PRIORITY 1
#define kDDP_PF_DEFAULT_ASV_STATE 0
#define kDDP_PF_DEFAULT_ASV_TAG_LIST_TYPE 1

/* 0x00D5 user defined MAC for ASV */
#define kDDP_PF_DEFAULT_ASV_USR_MAC_SRV_TYPE 1
#define kDDP_PF_DEFAULT_ASV_USR_MAC_ADDR_TYPE 1
#define kDDP_PF_DEFAULT_ASV_USR_MAC_ADDR_CATG 1
#define kDDP_PF_DEFAULT_ASV_USR_MAC_ADDR {0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#define ksDDP_PF_DEFAULT_ASV_USR_MAC_ADDR_DESC "Default MAC"

/* First 8 ssid configuration are accessed */
#define SSID_NUMBER 8
/* data structure of ssid configuration
 */
struct ssid_entry {
    INT1 name[32];
    INT4 vid;
    INT4 security;
    INT1 password[16];
};
/* data structure of DST (daylight saving time) between proto engine and platform
 */
struct dst_setting {
    UINT1 enable;
    UINT2 start_year;
    UINT1 start_month;
    UINT1 start_day;
    UINT1 start_hour;
    UINT1 start_minute;
    UINT2 end_year;
    UINT1 end_month;
    UINT1 end_day;
    UINT1 end_hour;
    UINT1 end_minute;
    UINT1 offset;
};
/* data structure of get vlan configuration between proto engine and platform
 */
struct vlan_container {
    UINT4 vlanId;
    UINT1 buf[DDP_FIELD_LEN_VLAN_TAG_PLIST];
};
/* the size of snmp name container */
#define DDP_PLATFORM_SNMP_NAMES_MAX 10
/* data structure of get snmp configuration bwtween proto engine and platform
 */
struct snmp_container {
    UINT4 number;
    UINT1 *list;
};
/* data structure of ntp server address between proto engine and platform
 */
struct ntp_addr {
    UINT1 format;
    UINT1 buf[DDP_FIELD_LEN_NTP_SERVER];
};


/* ddp_platform_search_if_by_mac
 *   subroutine to find network interface by mac address
 *
 *   list : interface list to be searched
 *   macAddr : mac address
 *
 *   return : the corresponding interface or null
 */
struct ddp_interface* ddp_platform_search_if_by_mac(struct ddp_interface* list, UINT1* macAddr);
/* ddp_platform_search_if_by_index
 *   subroutine to find network interface by index
 *
 *   list : interface list to be searched
 *   ifindex : interface index
 *
 *   return : the corresponding interface or null
 */
struct ddp_interface* ddp_platform_search_if_by_index(struct ddp_interface* list, UINT4 ifindex);

/* ddp_platform_get_if_list
 *   subroutine to generate interface list
 *
 *   list : interface list pointer to keep list
 *   len  : number of interfaces in list
 *
 *   return : 0 -> success, others -> error
 */
INT4 ddp_platform_get_if_list(struct ddp_interface** list, INT4* len);
/* ddp_platform_free_if_list
 *   subroutine to release interface list that is created at process startup
 *
 *   list : interface list to be released
 *
 *   return : 0 -> success, others -> error
 */
INT4 ddp_platform_free_if_list(struct ddp_interface* list);

/* ddp_platform_get_field
 *   subroutine to get value of the specified field from platform
 *
 *   ifs : interface
 *   field : field id
 *   buf : container for the value
 *   bufLen : buffer size.
 *            If it is smaller than the size defined in spec, it is given the correct size.
 *
 *   return : 0 -> success, others -> error
 */
INT4 ddp_platform_get_field(struct ddp_interface* ifs, INT4 field, UINT1* buf, INT4* bufLen);
/* ddp_platform_set_field
 *   function to set new value of the specified field to platform
 *
 *   ifs : interface
 *   field : field id
 *   data : new value buffer
 *   dataLen : the buffer size of data
 *
 *   return : 0 -> success, others -> error
 */
INT4 ddp_platform_set_field(struct ddp_interface* ifs, INT4 field, UINT1* buf, INT4 bufLen);

/* ddp_platform_get_user
 *   function to get user information of the specified user name from platform
 *
 *   ifs : interface
 *   user : struct to hold data written by platform
 *
 *   return : 0 -> success, others -> error
 */
INT4 ddp_platform_get_user(struct ddp_interface* ifs, struct ddp_user* user);
/* ddp_platform_set_user
 *   function to
 *   1. add user account if the specified user name does not exist
 *   2. modify user account if the specified user name exists
 *
 *   ifs : interface
 *   user : new user account information
 *
 *   return : 0 -> success, others -> error
 */
INT4 ddp_platform_set_user(struct ddp_interface* ifs, struct ddp_user* user);

/* ddp_platform_capable_process_report
 *   function to verify whether the device can handle general report or not
 *
 *   This function should be located inside protocol layer.
 *   However, platform vendor can change the returned value according to product spec
 *   and does not need to have another sdk release.
 *
 *   ifs: interface
 *
 *   return: DDP_PROCESS_REPORT_NO -> no, DDP_PROCESS_REPORT_YES -> yes
 */
INT4 ddp_platform_capable_process_report(struct ddp_interface* ifs);

/* ddp_platform_op_support
 *   function to check whether the specified opcode is supported by the device model or not
 *   Platform developer could set the different combination of opcodes for different model.
 *
 *   ifs: interface
 *   op: opcode
 *
 *   return: 0 -> not supported, 1 -> supported
 */
INT4 ddp_platform_op_support(struct ddp_interface* ifs, UINT2 op);

/* ddp_platform_init
 *   function to do platform init.
 *
 *   ddp_entrance will call this function.
 *   please keep this function here.
 *   developer can decide what it do
 */
INT4 ddp_platform_init(void);
/* ddp_platform_free
 *   function to do platform cleanup
 *
 *   it is called when program is going to be terminated.
 *   please keep this function here.
 *   developer can decide what it do.
 */
INT4 ddp_platform_free(void);


/* Set/Get Swith info */
/* ddp_platform_get_if_vlan_name
 *   function to
 *   1. Get the VLAN name
 *
 *   ifs : interface
 *   pu1data : VLAN name (BF_TEXT)
 *   pi4dataLen : length of VLAN name (DDP_FIELD_LEN_VLAN_NAME = 32 bytes)
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_vlan_name(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_vlan_id
 *   function to
 *   1. Get the VLAN id
 *
 *   ifs : interface
 *   pu1data : VLAN id (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_VLAN_ID = 4 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_vlan_id(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_vlan_tag_list_type
 *   function to
 *   1. Get the format of VLAN tag port list
 *
 *   ifs : interface
 *   pu1data : format of tag port list, 0 for string, 1 for port bitmap (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_vlan_tag_list_type(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_vlan_tag_list
 *   function to
 *   1. Get the VLAN tag port list.
 *
 *   ifs : interface
 *   pu1data : VLAN tag port list (BF_TEXT)
 *   pi4dataLen : length of VLAN tag port list (DDP_FIELD_LEN_VLAN_TAG_PLIST = 64 bytes)
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_vlan_tag_list(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_vlan_untag_list_type
 *   function to
 *   1. Get the format of VLAN un-tag port list
 *
 *   ifs : interface
 *   pu1data : format of un-tag port list, 0 for string, 1 for port bitmap (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_VLAN_UNTAG_LIST_TYPE = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_vlan_untag_list_type(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_vlan_untag_list
 *   function to
 *   1. Get the VLAN un-tag port list.
 *
 *   ifs : interface
 *   pu1data : VLAN un-tag port list (BF_TEXT)
 *   pi4dataLen : length of VLAN un-tag port list (DDP_FIELD_LEN_VLAN_TAG_PLIST = 64 bytes)
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_vlan_untag_list(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_vlan_list
 *   function to
 *   1. Get the VLAN list.
 *
 *   ifs : interface
 *   pu1data : VLAN list (BF_TEXT)
 *   pi4dataLen : length of VLAN list (DDP_FIELD_LEN_VLAN_LIST = 512 bytes)
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_vlan_list(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_vlan_state
 *   function to
 *   1. Get the VLAN state.
 *
 *   ifs : interface
 *   pu1data : VLAN list (BF_TEXT)
 *   pi4dataLen : length of VLAN list (DDP_FIELD_LEN_VLAN_LIST = 1 bytes)
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_vlan_state(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);


/* ddp_platform_get_if_asv_id
 *   function to
 *   1. Get the auto surveillance VLAN (ASV) id
 *
 *   ifs : interface
 *   pu1data : Surveillance VLAN id (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_ID = 4 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_asv_id(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_asv_priority
 *   function to
 *   1. Get the auto surveillance VLAN (ASV) priority
 *
 *   ifs : interface
 *   pu1data : Surveillance VLAN priority (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_PRI = 4 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_asv_priority(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_asv_state
 *   function to
 *   1. Get the auto surveillance VLAN (ASV) state
 *
 *   ifs : interface
 *   pu1data : Surveillance VLAN state, 0 for disable, 1 for enable (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_STATE = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_asv_state(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_asv_tag_list_type
 *   function to
 *   1. Get the format of auto surveillance VLAN (ASV) tag port list
 *
 *   ifs : interface
 *   pu1data : format of ASV tag port list, 0 for string, 1 for bitmap (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_TAG_LIST_TYPE = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_asv_tag_list_type(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_asv_tag_list
 *   function to
 *   1. Get auto surveillance VLAN (ASV) tag port list
 *
 *   ifs : interface
 *   pu1data : ASV tag port list, (BF_TEXT)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_TAG_PLIST = 64 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_asv_tag_list(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_asv_usr_def_mac_srv_type
 *   function to
 *   1. Get the service type of auto surveillance VLAN (ASV) user defined MAC
 *
 *   ifs : interface
 *   pu1data : service type of ASV user defined MAC, 0 for Add, 1 for delete (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_USR_DEF_MAC_SRV_TYPE = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_asv_usr_def_mac_srv_type(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_asv_usr_def_mac_addr_type
 *   function to
 *   1. Get the type of auto surveillance VLAN (ASV) user defined MAC
 *
 *   ifs : interface
 *   pu1data : type of ASV user defined MAC, 0 for MAC address, 1 for OUI address (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_TYPE = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_asv_usr_def_mac_addr_type(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_asv_usr_def_mac_addr_category
 *   function to
 *   1. Get the category of auto surveillance VLAN (ASV) user defined MAC
 *
 *   ifs : interface
 *   pu1data : category of ASV user defined MAC, (BF_BINARY)
 *           0 for Video Management Server (VMS),
 *           1 for VMS client
 *           2 for Video encoder
 *           3 for Network storage
 *           4 for Other IP Surveillance device
 *   pi4dataLen : DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_CATG = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_asv_usr_def_mac_addr_category(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_asv_usr_def_mac_addr
 *   function to
 *   1. Get the address of auto surveillance VLAN (ASV) user defined MAC
 *
 *   ifs : interface
 *   pu1data : address of ASV user defined MAC, (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR = 6 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_asv_usr_def_mac_addr(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_get_if_asv_usr_def_mac_desc
 *   function to
 *   1. Get the description of auto surveillance VLAN (ASV) user defined MAC
 *
 *   ifs : interface
 *   pu1data : description of ASV user defined MAC, (BF_TEXT)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC = 20 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_get_if_asv_usr_def_mac_desc(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* ddp_platform_set_if_vlan
 *   function to set VLAN information on machine.
 *
 *   It needs driver api to set the VLAN information on machine.
 *   Reserved for vendor.
 *
 *   This function directly access global pf gstDDP_PF_VLAN
 *
 *   ifs : interface
 *
 *   return : 0 -> success, others -> error
 */
INT4 ddp_platform_set_if_vlan(struct ddp_interface* ifs);

/* ddp_platform_set_if_vlan_id
 *   function to
 *   1. Set the VLAN id
 *
 *   ifs : interface
 *   pu1data : VLAN id (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_VLAN_ID = 4 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_vlan_id(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* ddp_platform_set_if_vlan_tag_list_type
 *   function to
 *   1. Set the format of VLAN tag port list
 *
 *   ifs : interface
 *   pu1data : format of tag port list, 0 for string, 1 for port bitmap (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_vlan_tag_list_type(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* ddp_platform_set_if_vlan_tag_list
 *   function to
 *   1. Set the VLAN tag port list.
 *
 *   ifs : interface
 *   pu1data : VLAN tag port list (BF_TEXT)
 *   pi4dataLen : length of VLAN tag port list (DDP_FIELD_LEN_VLAN_TAG_PLIST = 64 bytes)
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_vlan_tag_list(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* ddp_platform_set_if_vlan_untag_list_type
 *   function to
 *   1. Set the format of VLAN un-tag port list
 *
 *   ifs : interface
 *   pu1data : format of un-tag port list, 0 for string, 1 for port bitmap (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_VLAN_UNTAG_LIST_TYPE = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_vlan_untag_list_type(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* ddp_platform_set_if_vlan_untag_list
 *   function to
 *   1. Set the VLAN un-tag port list.
 *
 *   ifs : interface
 *   pu1data : VLAN un-tag port list (BF_TEXT)
 *   pi4dataLen : length of VLAN un-tag port list (DDP_FIELD_LEN_VLAN_TAG_PLIST = 64 bytes)
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_vlan_untag_list(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* ddp_platform_set_if_asv_hw
 *   function to set ASV information on machine.
 *
 *   It needs driver api to set the ASV information on machine.
 *   Reserved for vendor.
 *
 *   This function directly access global PF gstDDP_PF_ASV
 *
 *   ifs : interface
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_asv_hw(struct ddp_interface* ifs);

/* ddp_platform_set_if_asv_id
 *   function to
 *   1. Set the auto surveillance VLAN (ASV) id
 *
 *   ifs : interface
 *   pu1data : Surveillance VLAN id (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_ID = 4 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_asv_id(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* ddp_platform_set_if_asv_priority
 *   function to
 *   1. Set the auto surveillance VLAN (ASV) priority
 *
 *   ifs : interface
 *   pu1data : Surveillance VLAN priority (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_PRI = 4 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_asv_priority(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* ddp_platform_set_if_asv_state
 *   function to
 *   1. Set the auto surveillance VLAN (ASV) state
 *
 *   ifs : interface
 *   pu1data : Surveillance VLAN state, 0 for disable, 1 for enable (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_STATE = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_asv_state(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* ddp_platform_set_if_asv_tag_list_type
 *   function to
 *   1. Set the format of auto surveillance VLAN (ASV) tag port list
 *
 *   ifs : interface
 *   pu1data : format of ASV tag port list, 0 for string, 1 for bitmap (BF_BINARY)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_TAG_LIST_TYPE = 1 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_asv_tag_list_type(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* ddp_platform_set_if_asv_tag_list
 *   function to
 *   1. Set auto surveillance VLAN (ASV) tag port list
 *
 *   ifs : interface
 *   pu1data : ASV tag port list, (BF_TEXT)
 *   pi4dataLen : DDP_FIELD_LEN_ASV_TAG_PLIST = 64 bytes
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_asv_tag_list(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* ddp_platform_set_if_asv_usr_def_mac_hw
 *   function to set user-defined ASV MAC information on machine.
 *
 *   It needs driver api to set the user-defined ASV MAC information on machine.
 *   Reserved for vendor.
 *
 *   This function directly accesses PF gstDDP_PF_ASV_USR_MAC.
 *
 *   pstDDP_IF_ifs : interface
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_asv_usr_def_mac_hw(struct ddp_interface* pstDDP_IF_ifs);

/* ddp_platform_set_if_asv_usr_def_mac
 *   function to set user-defined ASV MAC information.
 *
 *   This function directly accesses gastDDP_ASV_MAC_List.
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data:        input data, struct stDDP_PF_ASV
 *   i4dataLen:      data length, sizeof(struct stDDP_PF_ASV)
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4 ddp_platform_set_if_asv_usr_def_mac(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/*****************************************
 * OP:   0x00D6
 * DESC: Get user defined ASV MAC address
 *
 * Related field:
 *  DDP_FIELD_ASV_USR_DEF_MAC,
 *
 *****************************************/
/* ddp_platform_get_if_asv_usr_def_mac
 *   function to get new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_asv_usr_def_mac(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/*****************************************
 * OP:   0x00D7
 * DESC: Get device port number
 *
 * Related field:
 *  DDP_FIELD_PORT_NUM_GET,
 *
 *****************************************/
/* ddp_platform_get_if_port_number
 *   function to get device port number
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 *
 *   NOTE: This function will call a driver api to get device information.
 *         the driver api is reserved for vendor.
 *
 */
INT4
ddp_platform_get_if_port_number(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4*  pi4dataLen);

/* i4fnddp_platform_get_if_port_number_hw
 *   driver function to get device port number
 *   Reserved for vendor
 *
 * pstPortList - device port list, including stacking units.
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 *
 *   NOTE: the driver api is reserved for vendor.
 *
 */
INT4
i4fnddp_platform_get_if_port_number_hw(stDDP_PORT_LIST_t *pstPortList);

/* ddp_platform_set_multi_ssid
 *   to set ssid into device configuration (the first 8 ssid groups).
 *
 *   band : frequency id
 *   table : new configuration
 *
 *   return : 0 -> success, others ->error
 */
INT4 ddp_platform_set_multi_ssid(INT1 band, struct ssid_entry* table);

/* ddp_platform_get_multi_ssid
 *   to get ssid into device configuration (the first 8 ssid groups).
 *
 *   band : frequency id
 *   table : container to store configuraion
 *
 *   return : 0 -> success, others ->error
 */
INT4 ddp_platform_get_multi_ssid(INT1 band, struct ssid_entry* table);

#ifdef __cplusplus
}
#endif

#endif /* _DDP_PLATFORM_H_ */
