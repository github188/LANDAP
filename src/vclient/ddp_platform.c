
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

//#include <pwd.h>
//#include <shadow.h>

#include "ddp.h"
#include "ddp_platform.h"

/* Strings of product category
 * Please do not modify these strings by yourself.
 * They must be synchronized with the definition in spec.
 */
INT1 prodcat_str_switch[]       = DDP_PRODCAT_SWITCH;
INT1 prodcat_str_ap[]           = DDP_PRODCAT_AP;
INT1 prodcat_str_netcam[]       = DDP_PRODCAT_NETCAM;
INT1 prodcat_str_videoencoder[] = DDP_PRODCAT_VIDEOENCORDER;
INT1 prodcat_str_nvr[]          = DDP_PRODCAT_NVR;
INT1 prodcat_str_nas[]          = DDP_PRODCAT_NAS;
INT1 prodcat_str_servroute[]    = DDP_PRODCAT_SERVROUTE;
INT1 prodcat_str_wlc[]          = DDP_PRODCAT_WLC;
INT1 prodcat_str_wls[]          = DDP_PRODCAT_WLS;
INT1 prodcat_str_wlr[]          = DDP_PRODCAT_WLR;
INT1 prodcat_str_epos[]         = DDP_PRODCAT_EPOS;
INT1 prodcat_str_aaa[]          = DDP_PRODCAT_AAA;


/* ddp_platform_search_if_by_mac
 *   subroutine to find network interface by mac address
 *
 *   list : interface list to be searched
 *   macAddr : mac address
 *
 *   return : the corresponding interface or null
 */
struct ddp_interface*
ddp_platform_search_if_by_mac
(
    struct ddp_interface* list,
    UINT1* macAddr
)
{
    struct ddp_interface* tmp = NULL;

    if (list == NULL || macAddr == NULL) { return tmp; }
    tmp = list->next;
    while (tmp != NULL) {
        if (memcmp(tmp->macAddr, macAddr, MAC_ADDRLEN) == 0) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return tmp;
}

/* ddp_platform_search_if_by_index
 *   subroutine to find network interface by index
 *
 *   list : interface list to be searched
 *   ifindex : interface index
 *
 *   return : the corresponding interface or null
 */
struct ddp_interface*
ddp_platform_search_if_by_index
(
    struct ddp_interface* list,
    UINT4 ifindex
)
{
    struct ddp_interface* tmp = NULL;

    if (list == NULL) { return tmp; }
    tmp = list->next;
    while (tmp) {
        if (tmp->ifindex == ifindex) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return tmp;
}

/* ddp_platform_free_if_list
 *   subroutine to release interface list that is created at process startup
 *
 *   list : interface list to be released
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_platform_free_if_list
(
    struct ddp_interface* list
)
{
    INT4 ret = 0;
    struct ddp_interface* ifn = NULL;

    if (list == NULL) {
        return ret;
    }

    ifn = list->next;
    while (ifn) {
        list->next = ifn->next;
        if (ifn->name) {
            free(ifn->name); ifn->name = NULL;
        }
        free(ifn);
        ifn = list->next;
    }
    free(list); list = NULL;

    return ret;
}

/* ddp_platform_get_if_list
 *   subroutine to generate interface list
 *
 *   list : interface list pointer to keep list
 *   len  : number of interfaces in list
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_platform_get_if_list
(
    struct ddp_interface** list,
    INT4* len
)
{
    INT4 ret = 0;
    struct ddp_interface *if_list = NULL;
    struct ifreq ifr;
    struct ifreq* it = NULL;
    struct ifconf ifc;
    char buf[2048];
    INT4 count = 0;
    UINT4 num = 0;
    INT4 if_idx = 0;
    struct ddp_interface *ifn = NULL;
    INT4 idx = 0;
    UINT1 uc;
    UINT4 zero = 0;
    INT4 sockfd = 0;

    if_list = (struct ddp_interface*)malloc(sizeof(struct ddp_interface));
    if (if_list == NULL) {
        ret = -1;
        goto get_if_over;
    } else {
        if_list->next = NULL;
    }

    ifc.ifc_len = sizeof(buf); 
    ifc.ifc_buf = buf; 
    memset(buf, 0, ifc.ifc_len);
    /* query */
    if (g_iSockfd > 0) {
        sockfd = g_iSockfd;
    } else {
        sockfd = g_srvSockfd;
    }
    ret = ioctl(sockfd, SIOCGIFCONF, &ifc);

    if (ret != 0) {
        if (ifc.ifc_len > 2048) {
            ifc.ifc_buf = (char*)malloc(ifc.ifc_len);
            if (ifc.ifc_buf == NULL) {
                ret = -2;
                goto get_if_over;
            }
            memset(ifc.ifc_buf, 0, ifc.ifc_len);
            /* get info */
            if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
                ret = -3;
                goto get_if_over;
            }
        } else {
            ret = -4;
            goto get_if_over;
        }
    }
    
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get ifc\n");
    it = ifc.ifc_req; 
    for (num = 0; num < ifc.ifc_len / sizeof(struct ifreq); num++, it++) {
        if (it->ifr_name) {
            strcpy(ifr.ifr_name, it->ifr_name);
        }

        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == 0) {
            if ( !(ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "\n  IF name: %s\n",ifr.ifr_name);

                if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0) {
                    /* skip ip (0.0.0.0) */
                    if (memcmp(ifr.ifr_addr.sa_data + 2, &zero, 4) == 0) { continue; }
                    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "  IP     :");
                    for (idx = 0; idx < 4; idx++) {
                         uc = *(ifr.ifr_addr.sa_data + 2 + idx);
                         DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, " %d ",uc);
                    }
                    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "\n");
                }
                if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == 0) {
                    if_idx = ifr.ifr_ifindex;
                    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "  INDEX  : %d\n", ifr.ifr_ifindex);
                }
                if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == 0) {
                    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "  MAC    :");
                    for (idx = 0; idx < 6; idx++) {
                          uc = *(ifr.ifr_hwaddr.sa_data + idx);
                          DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, " %02X ",uc);
                    }
                    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "\n");
                }
                /* insert into interface list */
                ifn = (struct ddp_interface*)malloc(sizeof(struct ddp_interface));
                if (ifn == NULL) {
                    DDP_DEBUG("%s (%d) : fail to allocate interface\n", __FUNCTION__, __LINE__);
                } else {
                    ifn->name = (INT1*)malloc(sizeof(INT1) * strlen(ifr.ifr_name));
                    if (ifn->name == NULL) {
                        free(ifn); ifn = NULL;
                        continue;
                    }
                    memset(ifn->name, 0, sizeof(sizeof(INT1) * strlen(ifn->name)));
                    strcpy(ifn->name, ifr.ifr_name);
                    memcpy(ifn->macAddr, ifr.ifr_hwaddr.sa_data, MAC_ADDRLEN);
                    ifn->ifindex = if_idx;
                    if (if_list->next == NULL) {
                        if_list->next = ifn;
                        ifn->next = NULL;
                    } else {
                        ifn->next = if_list->next;
                        if_list->next = ifn;
                    }
                    count++;
                }
            }
        } 
        else { /* handle error */ } 
    } // for loop

get_if_over:
    if (ifc.ifc_buf && ifc.ifc_buf != buf) { free(ifc.ifc_buf); ifc.ifc_buf = NULL; }
    *list = if_list;
    *len = count;
    
    return ret;
}

/* ddp_platform_switch.c */
INT4 ddp_platform_set_if_vlan_name(struct ddp_interface* pstDDP_IF_ifs, UINT1* pu1data, INT4 i4dataLen);

/* flag to indicate whether it can receive and process general report
 * -1: init state, 0: no, 1: yes
 */
INT4 pf_capableProcessReport = -1;

/* The following variables are designed for virtual client ONLY.
 *   When program startup, all default values are copied to corresponding buffer.
 *   Utility may change field values in buffer.
 */
/* default field values for virtual client */
UINT1 vc_brandName[] = "VCLIENT";
UINT1 vc_prodName[] = "DEVICE CLIENT";
UINT1 vc_modelName[] = "MODEL TEST";
UINT1 vc_version[] = "3.7";
//UINT1 vc_prodType[] = { 1, 2 };
UINT2 vc_prodType = 0x0103;
//UINT1 vc_prodHwSpec[] = { 11, 12, 13, 14 };
UINT4 vc_prodHwSpec = 0x11121314;
UINT1 vc_sysName[] = "NAS12";
//UINT1 vc_webPort[] = { 80, 0 };
UINT2 vc_webPort = 80;
UINT1 vc_customizedDns = 0;
UINT1 vc_subnetMask[] = { 255, 255, 255, 0 };
UINT1 vc_defaultGateway[] = { 172, 17, 2, 254 };
UINT1 vc_primaryDns[] = { 172, 17, 5, 1 };
UINT1 vc_secondDns[] = { 172, 17, 5, 10 };
UINT1 vc_ip[] = {172, 17, 2, 138 };
UINT1 vc_dhcp = 1;
UINT1 vc_ntpEnable = 1;
UINT1 vc_ntpServer[] = "ntpserver.com";
UINT1 vc_ntpFormat = 0;
UINT1 vc_ntpSupport = 0;
/* DST */
UINT1 vc_dst_enable = 0;
UINT2 vc_dst_start_year = 2013;
UINT1 vc_dst_start_month = 3;
UINT1 vc_dst_start_day = 1;
UINT1 vc_dst_start_hour = 1;
UINT1 vc_dst_start_minute = 0;
UINT2 vc_dst_end_year = 2015;
UINT1 vc_dst_end_month = 10;
UINT1 vc_dst_end_day = 31;
UINT1 vc_dst_end_hour = 2;
UINT1 vc_dst_end_minute = 0;
UINT1 vc_dst_offset = 60;

/* Only switch will handle general report information. */
/* ddp_platform_capable_process_report */
#ifdef SWITCH_MODE
INT1* vc_prodCat = prodcat_str_switch;
#elif defined AP_MODE
INT1* vc_prodCat = prodcat_str_ap;
#else
INT1* vc_prodCat = prodcat_str_nas;
#endif

UINT1 vc_hwVersion[] = "X3.1";
UINT1 vc_serialNumber[] = "12345";
UINT1 vc_interfaceNumber[] = "LAN1";
struct ddp_user vc_users[] = {
    { "admin", "admin", DDP_USER_TYPE_ADMIN },
    { "user",  "user",  DDP_USER_TYPE_USER }
};
UINT4 vc_rebootWaitTime = 10000; /* milli-second */
UINT1 vc_snmpGlobalState = 1;
UINT1 vc_snmpCommunityNameRO[] = "COMMUNITY RO";
UINT1 vc_snmpCommunityNameRW[] = "COMMUNITY RW";
UINT1 vc_snmpViewNameRO[] = "VIEW RO";
UINT1 vc_snmpViewNameRW[] = "VIEW RW";
UINT1 vc_fwLocalPath[] = "test.png";
UINT1 vc_cfgBackupLocalPath[] = "cfg_bk.img";
UINT1 vc_cfgRestoreLocalPath[] = "cfg_rest.img";

/* Fields for switch */
/* 0x00D2 VLAN */
UINT1 vc_vlan_name[DDP_FIELD_LEN_VLAN_NAME] = ksDDP_PF_DEFAULT_VLAN_NAME;
UINT4 vc_vlan_id = kDDP_PF_DEFAULT_VLAN_ID;
UINT1 vc_vlan_tag_list_type = kDDP_PF_DEFAULT_VLAN_TAG_LIST_TYPE;
UINT1 vc_vlan_tag_plist[DDP_FIELD_LEN_VLAN_TAG_PLIST]="";
UINT1 vc_vlan_untag_list_type = kDDP_PF_DEFAULT_VLAN_UNTAG_LIST_TYPE;
UINT1 vc_vlan_untag_plist[DDP_FIELD_LEN_VLAN_UNTAG_PLIST]="";

/* 0x00D3 ASV */
UINT4 vc_asv_id = kDDP_PF_DEFAULT_ASV_ID;
UINT4 vc_asv_priority = kDDP_PF_DEFAULT_ASV_ID;
UINT1 vc_asv_state = kDDP_PF_DEFAULT_ASV_STATE;
UINT1 vc_asv_tag_list_type = kDDP_PF_DEFAULT_ASV_TAG_LIST_TYPE;
UINT1 vc_asv_tag_plist[DDP_FIELD_LEN_ASV_TAG_PLIST]="";

/* 0x00D5 User defined MAC for ASV */
UINT1 vc_asv_user_mac_srv_type = kDDP_PF_DEFAULT_ASV_USR_MAC_SRV_TYPE;
UINT1 vc_asv_user_mac_addr_type = kDDP_PF_DEFAULT_ASV_USR_MAC_ADDR_TYPE;
UINT1 vc_asv_user_mac_addr_category = kDDP_PF_DEFAULT_ASV_USR_MAC_ADDR_CATG;
UINT1 vc_asv_user_mac_addr[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR]=kDDP_PF_DEFAULT_ASV_USR_MAC_ADDR;
UINT1 vc_asv_user_mac_addr_desc[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC]=ksDDP_PF_DEFAULT_ASV_USR_MAC_ADDR_DESC;
/* DDP information */
UINT1 vc_specVer[] = "0.25";

/* buffer to keep field values */
UINT1 pf_brandName[DDP_FIELD_LEN_BRAND_NAME];
UINT1 pf_prodName[DDP_FIELD_LEN_PRODUCT_NAME];
UINT1 pf_modelName[DDP_FIELD_LEN_MODEL_NAME];
UINT1 pf_version[DDP_FIELD_LEN_VERSION];
//UINT1 pf_prodType[DDP_FIELD_LEN_PRODUCT_TYPE];
UINT2 pf_prodType = 0;
//UINT1 pf_prodHwSpec[DDP_FIELD_LEN_PRODUCT_HW_SPEC];
UINT4 pf_prodHwSpec = 0;
UINT1 pf_macAddr[MAC_ADDRLEN];
UINT1 pf_sysName[DDP_FIELD_LEN_SYSTEM_NAME];
//UINT1 pf_webPort[DDP_FIELD_LEN_WEB_SERVICE_PORT];
UINT2 pf_webPort = 0;
UINT1 pf_customizedDns;
UINT1 pf_subnetMask[DDP_FIELD_LEN_SUBNET_MASK];
UINT1 pf_defaultGateway[DDP_FIELD_LEN_DEFAULT_GATEWAY];
UINT1 pf_primaryDns[DDP_FIELD_LEN_PRIMARY_DNS];
UINT1 pf_secondDns[DDP_FIELD_LEN_SECONDARY_DNS];
UINT1 pf_ip[DDP_FIELD_LEN_DEVICE_IP_ADDR];
UINT1 pf_dhcp;
UINT1 pf_prodCat[DDP_FIELD_LEN_PRODUCT_CATEGORY];
UINT1 pf_hwVersion[DDP_FIELD_LEN_HARDWARE_VERSION];
UINT1 pf_serialNumber[DDP_FIELD_LEN_SERIAL_NUMBER];
UINT1 pf_interfaceNumber[DDP_FIELD_LEN_INTERFACE_NUMBER];
UINT4 pf_rebootWaitTime;
UINT1 pf_snmpGlobalState;
UINT1 pf_snmpCommunityNameRO[DDP_PLATFORM_SNMP_NAMES_MAX][DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO];
UINT1 pf_snmpCommunityNameRW[DDP_PLATFORM_SNMP_NAMES_MAX][DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW];
UINT1 pf_snmpViewNameRO[DDP_PLATFORM_SNMP_NAMES_MAX][DDP_FIELD_LEN_SNMP_VIEW_NAME_RO];
UINT1 pf_snmpViewNameRW[DDP_PLATFORM_SNMP_NAMES_MAX][DDP_FIELD_LEN_SNMP_VIEW_NAME_RW];
UINT1 pf_fwLocalPath[DDP_FIELD_LEN_FW_LOCAL_PATH];
UINT1 pf_cfgBackupLocalPath[DDP_FIELD_LEN_CFG_BACKUP_LOCAL_PATH];
UINT1 pf_cfgRestoreLocalPath[DDP_FIELD_LEN_CFG_RESTORE_LOCAL_PATH];
UINT1 pf_ntpEnable;
UINT1 pf_ntpServer[DDP_FIELD_LEN_NTP_SERVER];
UINT1 pf_ntpFormat;
/* DST */
UINT1 pf_dst_enable;
UINT2 pf_dst_start_year;
UINT1 pf_dst_start_month;
UINT1 pf_dst_start_day;
UINT1 pf_dst_start_hour;
UINT1 pf_dst_start_minute;
UINT2 pf_dst_end_year;
UINT1 pf_dst_end_month;
UINT1 pf_dst_end_day;
UINT1 pf_dst_end_hour;
UINT1 pf_dst_end_minute;
UINT1 pf_dst_offset;
/* ssid */
struct ssid_entry ssid_table_24g[SSID_NUMBER];
struct ssid_entry ssid_table_5g[SSID_NUMBER];
/* DDP information */
UINT1 pf_specVer[DDP_FIELD_LEN_SPEC_VERSION];

/* Fields for switch */
/* 0x00D2 VLAN */
struct stDDP_PF_VLAN gstDDP_PF_VLAN;
struct stDDP_PF_VLAN pf_vlan_arr[DDP_VLANID_MAX];

/* 0x00D3 ASV */
struct stDDP_PF_ASV gstDDP_PF_ASV;

/* 0x00D5 User defined MAC for ASV */
INT1 gaiDDP_PF_DEFAULT_ASV_USR_MAC_ADDR[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR]={0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
/* Array of ASV user defined MAC List: 1-based array  */
stDDP_ASV_USR_MAC_NODE_t gastDDP_ASV_MAC_List[kDDP_SWITCH_MAX_ENTRY_PER_PACKET+1];
struct ddp_user pf_users[4];

INT4
ddp_platform_get_if_brand_name
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (strlen((INT1*)pf_brandName) >= (*bufLen) && strlen((INT1*)pf_brandName) < DDP_FIELD_LEN_BRAND_NAME) {
        *bufLen = strlen((INT1*)pf_brandName);
        return -2;
    }
    /* copy brand name string to buf
     * do not write beyond bufLen
     */
    strncpy((INT1*)buf, (INT1*)pf_brandName, strlen((INT1*)pf_brandName));
    return 0;
}

INT4
ddp_platform_get_if_product_name
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (strlen((INT1*)pf_prodName) >= (*bufLen) && strlen((INT1*)pf_prodName) < DDP_FIELD_LEN_PRODUCT_NAME) {
        *bufLen = strlen((INT1*)pf_prodName);
        return -2;
    }
    /* copy product name string to buf
     * do not write beyond bufLen
     */
    strncpy((INT1*)buf, (INT1*)pf_prodName, strlen((INT1*)pf_prodName));
    return 0;
}

INT4
ddp_platform_get_if_model_name
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    FILE *pFile = NULL;
    char line[255];
    INT1 fileBuf[255];
    memset(fileBuf, 0, 255);
    pFile = fopen("/etc/model", "rb");
    if(pFile) {
    	while(fgets(line, 255, pFile)) {
    		strcpy((char *)fileBuf, line);
    	}
    	fclose(pFile);
    }
    else {
    	strncpy((INT1*)buf, (INT1*)pf_modelName, strlen((INT1*)pf_modelName));
    	return -2;
    }
    if (strlen((INT1*)fileBuf) == 0 || strlen((INT1*)fileBuf) > (*bufLen)) {
    	strncpy((INT1*)buf, (INT1*)pf_modelName, strlen((INT1*)pf_modelName));
    	return -2;
    }

    memset(pf_modelName, 0, DDP_FIELD_LEN_MODEL_NAME);
    strncpy((INT1*)pf_modelName, (INT1*)fileBuf, strlen((INT1*)fileBuf));
    strncpy((INT1*)buf, (INT1*)pf_modelName, strlen((INT1*)pf_modelName));
    return 0;
}

INT4
ddp_platform_get_if_version
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    FILE *pFile = NULL;
    char line[255];
    UINT1 fileBuf[DDP_FIELD_LEN_VERSION];
    memset(fileBuf, 0, DDP_FIELD_LEN_VERSION);
    pFile = fopen("ddp_config", "rb");
    if(pFile) {
    	while(fgets(line, 255, pFile)) {
    		char *key = "version";
    		int len = strlen(key);
    		if(strncmp(line, key, len) == 0) {
    			if(line[len] == '=') {
    				strcpy((char *)fileBuf, line + len + 1);
    				if(fileBuf[strlen((char *)fileBuf)-1] == '\n')
    					fileBuf[strlen((char *)fileBuf)-1] = '\0';
    			}
    		}
    	}
    	fclose(pFile);
    }
    else {
    	strncpy((INT1*)buf, (INT1*)pf_version, strlen((INT1*)pf_version));
    	return -2;
    }

    if (strlen((INT1*)fileBuf) == 0 || strlen((INT1*)fileBuf) > (*bufLen)) {
    	strncpy((INT1*)buf, (INT1*)pf_version, strlen((INT1*)pf_version));
    	return -2;
    }
    memset(pf_version, 0, DDP_FIELD_LEN_VERSION);
    strncpy((INT1*)pf_version, (INT1*)fileBuf, strlen((INT1*)fileBuf));

    strncpy((INT1*)buf, (INT1*)pf_version, strlen((INT1*)pf_version));

    return 0;
}

INT4
ddp_platform_get_if_prod_type
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_PRODUCT_TYPE > (*bufLen)) {
        *bufLen = DDP_FIELD_LEN_PRODUCT_TYPE;
        return -2;
    }
    /* copy product type value of the specified interface to buf
     * do not write beyond bufLen
     */
    //memcpy((INT1*)buf, (INT1*)pf_prodType, *bufLen);
    UINT2* pBuf = (UINT2*)buf;
    *pBuf = pf_prodType;
    return 0;
}

INT4
ddp_platform_get_if_prod_hw_spec
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_PRODUCT_HW_SPEC > (*bufLen)) {
        *bufLen = DDP_FIELD_LEN_PRODUCT_HW_SPEC;
        return -2;
    }
    /* copy product hw spec value to buf
     * do not write beyond bufLen
     */
    //memcpy((INT1*)buf, (INT1*)pf_prodHwSpec, *bufLen);
    UINT4* pBuf = (UINT4*)buf;
    *pBuf = pf_prodHwSpec;
    return 0;
}

INT4
ddp_platform_get_if_system_name
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    FILE *pFile = NULL;
    char line[255];
    UINT1 fileBuf[DDP_FIELD_LEN_SYSTEM_NAME];
	memset(fileBuf, 0, DDP_FIELD_LEN_SYSTEM_NAME);
    pFile = popen("hostname", "r");

    if(pFile) {
    	while(fgets(line, 255, pFile)) {
    		if(strlen(line) > 0) {
    			strcpy((char *)fileBuf, line);
    			break;
    		}
    	}
    	pclose(pFile);
    }
    else {
    	strncpy((INT1*)buf, (INT1*)pf_sysName, strlen((INT1*)pf_sysName));
    	return -2;
    }

    if (strlen((INT1*)fileBuf) == 0 || strlen((INT1*)fileBuf) > (*bufLen)) {
    	strncpy((INT1*)buf, (INT1*)pf_sysName, strlen((INT1*)pf_sysName));
    	return -2;
    }

    memset(pf_sysName, 0, DDP_FIELD_LEN_SYSTEM_NAME);
    strncpy((INT1*)pf_sysName, (INT1*)fileBuf, strlen((INT1*)fileBuf));

    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "System name: %s\n", pf_sysName);
    strncpy((INT1*)buf, (INT1*)pf_sysName, strlen((INT1*)pf_sysName));
    return 0;
}

INT4
ddp_platform_get_if_mac
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_MAC_ADDR > *bufLen) {
        *bufLen = DDP_FIELD_LEN_MAC_ADDR;
        return -2;
    }
    /* copy mac address of the specified interface to buf
     * do not write beyond bufLen
     */
    memcpy((INT1*)buf, (INT1*)ifs->macAddr, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_ipv4
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    struct ifreq ifr;
    struct sockaddr_in* sa = NULL;
    
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifs->name);
    ifr.ifr_ifindex = ifs->ifindex;
    if (ioctl(g_iSockfd, SIOCGIFADDR, &ifr) != 0) {
        DDP_DEBUG("%s (%d) : ioctl get ip fail\n", __FUNCTION__, __LINE__);
        return -2;
    }
    sa = (struct sockaddr_in*)&ifr.ifr_addr;

    /* copy the ipv4 address of the specified interface to buf
     * do not write beyond bufLen
     */
    memcpy(buf, &sa->sin_addr, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_web_service_port
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    UINT2* pBuf = (UINT2*)buf;
    FILE *pFile = NULL;
    char line[255];
    INT1 fileBuf[255];
    memset(fileBuf, 0, 255);
    pFile = fopen("ddp_config", "rb");
    if(pFile) {
    	while(fgets(line, 255, pFile)) {
    		char *key = "webport";
    		int len = strlen(key);
    		if(strncmp(line, key, len) == 0) {
    			if(line[len] == '=') {
    				strcpy((char *)fileBuf, line + len + 1);
    				if(fileBuf[strlen((char *)fileBuf)-1] == '\n')
    					fileBuf[strlen((char *)fileBuf)-1] = '\0';
    			}
    		}
    	}
    	fclose(pFile);
    }
    else {
    	*pBuf = pf_webPort;
    	return -2;
    }

    if (strlen((INT1*)fileBuf) == 0) {
    	*pBuf = pf_webPort;
    	return -2;
    }

    pf_webPort = atoi(fileBuf);
    *pBuf = pf_webPort;
    return 0;
}

INT4
ddp_platform_get_if_customized_dns
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    FILE *pFile = NULL;
    char line[255];
    INT1 fileBuf[255];
    memset(fileBuf, 0, 255);
    pFile = fopen("ddp_config", "rb");
    if(pFile) {
    	while(fgets(line, 255, pFile)) {
    		char *key = "customized_dns";
    		int len = strlen(key);
    		if(strncmp(line, key, len) == 0) {
    			if(line[len] == '=') {
    				strcpy((char *)fileBuf, line + len + 1);
    				if(fileBuf[strlen((char *)fileBuf)-1] == '\n')
    					fileBuf[strlen((char *)fileBuf)-1] = '\0';
    			}
    		}
    	}
    	fclose(pFile);
    }
    else {
    	*buf = pf_customizedDns;
    	return -2;
    }

    if (strlen((INT1*)fileBuf) == 0) {
    	*buf = pf_customizedDns;
    	return -2;
    }

    pf_customizedDns = atoi(fileBuf);
	*buf = pf_customizedDns;
    return 0;
}

INT4
ddp_platform_get_if_subnet_mask
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    struct ifreq ifr;
    struct sockaddr_in* sa = NULL;

    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifs->name);
    ifr.ifr_ifindex = ifs->ifindex;
    if (ioctl(g_iSockfd, SIOCGIFNETMASK, &ifr) != 0) {
        DDP_DEBUG("%s (%d) : ioctl get ip fail\n", __FUNCTION__, __LINE__);
        return -2;
    }
    sa = (struct sockaddr_in*)&ifr.ifr_netmask;
	
	//memcpy((INT1*)buf, (INT1*)pf_subnetMask, *bufLen);
    memcpy(buf, &sa->sin_addr, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_default_gateway
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    FILE *pFile = NULL;
	char line[255], *pSave;
	char *pIf = NULL, *pDest = NULL, *pGW = NULL;
	char sAddr[256];
	memset(sAddr, 0, sizeof(sAddr));
	INT2 bFound = 0;
	struct in_addr addr;
	pFile = fopen("/proc/net/route" , "rb");

	if(pFile) {
		while(fgets(line, 255, pFile))
		{
			pIf = strtok_r(line , " \t", &pSave);
			pDest = strtok_r(NULL , " \t", &pSave);
			pGW = strtok_r(NULL , " \t", &pSave);

			if(pIf && pDest)
			{
				if(strcmp(pDest, "00000000") == 0)
				{
					if (pGW)
					{
						char *pEnd;
						int ng = strtol(pGW, &pEnd, 16);
						addr.s_addr = ng;

						if (inet_ntop(AF_INET, (void*)&addr, (INT1*)sAddr, sizeof(sAddr))) {
							bFound = 1;
						}
					}
					break;
				}
			}
		}
		fclose(pFile);
	}

	if(!bFound) {
		memcpy((INT1*)buf, (INT1*)pf_defaultGateway, *bufLen);
		return -2;
	};

	DDP_DEBUG("Found Gateway %s \n", sAddr);
    //memcpy((INT1*)buf, (INT1*)pf_defaultGateway, *bufLen);
    memcpy(buf, &addr, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_primary_dns
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    FILE *pFile = NULL;
    INT1 line[255], *pSave;
    memset(line, 0, 255);
    char *pField1 = NULL, *pField2 = NULL;
    char sDnsAddr[255];
    memset(sDnsAddr, 0, 255);
    struct in_addr dnsAddr;

    pFile = fopen("/etc/resolv.conf", "rb");

    if(pFile) {
    	while(fgets((char *)line, 255, pFile)) {
    		pField1 = strtok_r(line , " ", &pSave);
    		pField2 = strtok_r(NULL , " ", &pSave);

			if(pField1 && strcmp(pField1, "nameserver") == 0) {
				int len = strlen(pField2);
				if(pField2[len - 1] == '\n')
					len--;
				strncpy(sDnsAddr, pField2, len);
				break;
			}
    	}
    	fclose(pFile);
    }
    else {
    	memcpy((INT1*)buf, (INT1*)pf_primaryDns, *bufLen);
    	return -2;
    }

	if(strlen(sDnsAddr) == 0) {
		memcpy((INT1*)buf, (INT1*)pf_primaryDns, *bufLen);
		return -2;
	}

	if (inet_pton(AF_INET, sDnsAddr, &dnsAddr) == 0) {
		DDP_DEBUG("inet_pton\n");
    	memcpy((INT1*)buf, (INT1*)pf_primaryDns, *bufLen);
    	return -2;
	}

	DDP_DEBUG("Found Primary Dns %s \n", sDnsAddr);
    memcpy(buf, &dnsAddr, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_second_dns
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    FILE *pFile = NULL;
    INT1 line[255], *pSave;
    memset(line, 0, 255);
    char *pField1 = NULL, *pField2 = NULL;
    char sDnsAddr[255];
    memset(sDnsAddr, 0, 255);
    struct in_addr dnsAddr;

    pFile = fopen("/etc/resolv.conf", "rb");

    if(pFile) {
    	int count = 0;
    	while(fgets((char *)line, 255, pFile)) {
    		pField1 = strtok_r(line , " ", &pSave);
    		pField2 = strtok_r(NULL , " ", &pSave);

			if(pField1 && strcmp(pField1, "nameserver") == 0) {
				count++;
				if(count == 1)
					continue;
				int len = strlen(pField2);
				if(pField2[len -1] == '\n')
					len--;
				strncpy(sDnsAddr, pField2, len);
				break;
			}
    	}
    	fclose(pFile);
    }
    else {
    	memcpy((INT1*)buf, (INT1*)pf_secondDns, *bufLen);
    	return -2;
    }

	if(strlen(sDnsAddr) == 0) {
		memcpy((INT1*)buf, (INT1*)pf_secondDns, *bufLen);
		return -2;
	}

	if (inet_pton(AF_INET, sDnsAddr, &dnsAddr) == 0) {
		DDP_DEBUG("inet_ntop\n");
    	memcpy((INT1*)buf, (INT1*)pf_secondDns, *bufLen);
    	return -2;
	}

	DDP_DEBUG("Found Sencond Dns %s \n", sDnsAddr);
    memcpy(buf, &dnsAddr, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_dhcp
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    FILE *pFile = NULL;
    char line[255];
    INT1 fileBuf[255];
    memset(fileBuf, 0, 255);
    pFile = fopen("/etc/sysconfig/config/sys_config", "rb");
    if(pFile) {
    	while(fgets(line, 255, pFile)) {
    		char *key = "dhcp_enable";
    		int len = strlen(key);
    		if(strncmp(line, key, len) == 0) {
    			if(line[len] == '=') {
    				strcpy((char *)fileBuf, line + len + 1);
    				if(fileBuf[strlen((char *)fileBuf)-1] == '\n')
    					fileBuf[strlen((char *)fileBuf)-1] = '\0';
    			}
    		}
    	}
    	fclose(pFile);
    }
    else {
    	*buf = pf_dhcp;
    	return -2;
    }

    if (strlen((INT1*)fileBuf) == 0) {
    	*buf = pf_dhcp;
    	return -2;
    }

    pf_dhcp = atoi(fileBuf);
    memcpy((INT1*)buf, (INT1*)&pf_dhcp, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_date_time
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    UINT1 str[*bufLen];

    if (DDP_FIELD_LEN_DATE_TIME > *bufLen) {
        *bufLen = DDP_FIELD_LEN_DATE_TIME;
        return -2;
    }
    /* generate date time string of the specified interface according to spec,
     * then copy it to buf
     * do not write beyond bufLen
     */
    time_t now = time(NULL);
    struct tm* t; t = localtime(&now);
    memset(str, 0, *bufLen);
    sprintf((INT1*)str, "%04d%02d%02d%02d%02d%02d", (t->tm_year + 1900), (t->tm_mon + 1), t->tm_mday,
                                                     t->tm_hour, t->tm_min, t->tm_sec);
    strncpy((INT1*)buf, (INT1*)str, strlen((INT1*)str));

    return 0;
}

INT4
ddp_platform_get_if_time_zone
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    if (DDP_FIELD_LEN_TIMEZONE > *bufLen) {
        *bufLen = DDP_FIELD_LEN_TIMEZONE;
        return -2;
    }
    /*
     * generate time zone value of the specified interface according to spec,
     * then copy it to buf
     * do not write beyond bufLen
     *
     * 2 styles to represent time zone.
     * please refer to spec
     */
    UINT1 dhex[2] = {0,0};
    time_t tm1 = time(NULL);
    struct tm* t2 = gmtime(&tm1);
    time_t tm2 = mktime(t2);
    long dtime = (long)(tm1 - tm2);
	// GMT format
    if (dtime >= 0) {
        dhex[0] = abs(dtime) / 3600;
    }
    else {
        dhex[0] = 0xFF - ((abs(dtime) / 3600) - 1);
    }
    //dhex = (dhex << 8);
    memcpy((INT1*)buf, (INT1*)&dhex, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_daylight_saving
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }

    if (DDP_FIELD_LEN_DAYLIGHT_SAVING > *bufLen) {
        *bufLen = DDP_FIELD_LEN_DAYLIGHT_SAVING;
        return -2;
    }
    /* get the daylight saving status of the specified interface and copy it to buf
     * do not write beyond bufLen
     */
    time_t t = time(NULL);
    struct tm* tm1 = localtime(&t);
    UINT1 ds = 0;
    if (tm1->tm_isdst > 0) { ds = 1; }
    else if (tm1->tm_isdst == 0) { ds = 0; }
    else { ds = 0xFF; }
    memcpy((INT1*)buf, (INT1*)&ds, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_prod_category
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (strlen((INT1*)pf_prodCat) >= (*bufLen) && strlen((INT1*)pf_prodCat) < DDP_FIELD_LEN_PRODUCT_CATEGORY) {
        *bufLen = strlen((INT1*)pf_prodCat);
        return -2;
    }
    /* copy product category string to buf
     * do not write beyond bufLen
     */
    strncpy((INT1*)buf, (INT1*)pf_prodCat, strlen((INT1*)pf_prodCat));
    return 0;
}

INT4
ddp_platform_get_if_hw_version
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (strlen((INT1*)pf_hwVersion) >= (*bufLen) && strlen((INT1*)pf_hwVersion) < DDP_FIELD_LEN_HARDWARE_VERSION) {
        *bufLen = strlen((INT1*)pf_hwVersion);
        return -2;
    }
    /* copy hw version string to buf
     * do not write beyond buflen
     */
    strncpy((INT1*)buf, (INT1*)pf_hwVersion, strlen((INT1*)pf_hwVersion));
    return 0;
}

INT4
ddp_platform_get_if_serial_number
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (strlen((INT1*)pf_serialNumber) >= (*bufLen) && strlen((INT1*)pf_serialNumber) < DDP_FIELD_LEN_SERIAL_NUMBER) {
        *bufLen = strlen((INT1*)pf_serialNumber);
        return -2;
    }
    /* copy serial number string to buf
     * do not write beyond bufLen
     */
    strncpy((INT1*)buf, (INT1*)pf_serialNumber, strlen((INT1*)pf_serialNumber));
    return 0;
}

INT4
ddp_platform_get_if_interface_number
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_INTERFACE_NUMBER > *bufLen) {
        *bufLen = DDP_FIELD_LEN_INTERFACE_NUMBER;
        return -2;
    }
    /* copy the interface number string of the specified interface to buf
     * do not write beyond bufLen
     */
     /* this interface number (port) may be the specific to the ifs.
      * Diferent ifs could have different number
      */
    memcpy((INT1*)buf, (INT1*)pf_interfaceNumber, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_ipv6
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_DEVICE_IPV6_ADDR > *bufLen) {
        *bufLen = DDP_FIELD_LEN_DEVICE_IPV6_ADDR;
        return -2;
    }
    /* copy ipv6 adddress of the specified interface to buf
     * do not write beyond bufLen
     */
    //memcpy((INT1*)buf, (INT1*)pf_ipv6, bufLen);
    INT1 sbuf[100];
    INT1 token[60];
    UINT1 addr[IPV6_ADDRLEN];
    INT4 index = 0;
    INT4 prefix = 0;
    INT4 flag = 0;
    INT4 i = 0, j = 0;
    INT4 ret = 0;
    FILE* fd = fopen("/proc/net/if_inet6", "r");
    if (fd == NULL){ printf("open error %d\n", errno); }
    while (1) {
        memset(token, 0, sizeof(token));
        memset(sbuf, 0, sizeof(sbuf));
        ret = fscanf(fd, "%s", sbuf);
        //printf("ipv6 : %s\n", sbuf);
        j = 0;
        for (i = 0; i < strlen(sbuf); i++, j++) {
            token[j] = sbuf[i];
            if ((i + 1) % 4 == 0 && i < strlen(sbuf) - 1) {
                token[j+1]=':'; j++;
            }
        }
        //printf("TO %s\n", token);

        ret = fscanf(fd, "%X", &index);
        //printf("idx: %d\n", index);
        if(ret<=0)
        {
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "%s: line<%d> Failed to get index.\n",
                            __FUNCTION__, __LINE__);
            break;
        }

        ret = fscanf(fd, "%X", &prefix);
        //printf("prefix: %d\n", tmp);
        if(ret<=0)
        {
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "%s: line<%d> Failed to get pre-index.\n",
                            __FUNCTION__, __LINE__);
            break;
        }

        ret = fscanf(fd, "%s", sbuf);
        //printf("scope: %s\n", sbuf);
        if(ret<=0)
        {
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "%s: line<%d> Failed to get scope.\n",
                            __FUNCTION__, __LINE__);
            break;
        }

        ret = fscanf(fd, "%X", &flag);
        //printf("flag ; %02X", flag);
        if(ret<=0)
        {
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "%s: line<%d> Failed to get flag.\n",
                            __FUNCTION__, __LINE__);
            break;
        }

        ret = fscanf(fd, "%s", sbuf);
        //printf("name: %s\n", sbuf);
        if(ret<=0)
        {
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "%s: line<%d> Failed to get name.\n",
                            __FUNCTION__, __LINE__);
            break;
        }

        if (index == ifs->ifindex) {
            //DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "ipv6 %s, ifindex %d\n", token, index);
            if (inet_pton(AF_INET6, token, addr) != 1) {
                DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "pton fail (error %d)", errno);
            } else {
                memcpy(buf, addr, *bufLen);
            }
            break;
        }
    }
    if (fd) { fclose(fd); fd = NULL; }

    return 0;
}

INT4
ddp_platform_get_if_ipv6_prefix
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_DEVICE_IPV6_PREFIX > *bufLen) {
        *bufLen = DDP_FIELD_LEN_DEVICE_IPV6_PREFIX;
        return -2;
    }
    /* copy ipv prefix of the specified interface to buf
     * do not write beydon bufLen
     */
    //memcpy((INT1*)buf, (INT1*)pf_ipv6Prefix, bufLen);
    *buf = 64;
    return 0;
}

INT4
ddp_platform_get_if_def_gateway_ipv6
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_DEFAULT_GATEWAY_IPV6 > *bufLen) {
        *bufLen = DDP_FIELD_LEN_DEFAULT_GATEWAY_IPV6;
        return -2;
    }
    /* copy the default gateway ipv6 address of the specified interface to buf
     * do not write beyond bufLen
     */
    //memcpy((INT1*)buf, (INT1*)pf_ipv6DefGateway, bufLen);
    return 0;
}

INT4
ddp_platform_get_if_primary_dns_ipv6
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_PRIMARY_DNS_IPV6 > *bufLen) {
        *bufLen = DDP_FIELD_LEN_PRIMARY_DNS_IPV6;
        return -2;
    }
    /* copy the primary DNS ipv6 address of the specified interface to buf
     * do not write beyond bufLen
     */
    //memcpy((INT1*)buf, (INT1*)pf_ipv6PrimaryDns, bufLen);
    return 0;
}

INT4
ddp_platform_get_if_second_dns_ipv6
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_SECONDARY_DNS_IPV6 > *bufLen) {
        *bufLen = DDP_FIELD_LEN_SECONDARY_DNS_IPV6;
        return -2;
    }
    /* copy the secondary DNS ipv6 address of the specified interface to buf
     * do not write beyond bufLen
     */
    //memcpy((INT1*)buf, (INT1*)pf_ipv6SecondDns, bufLen);
    return 0;
}

INT4
ddp_platform_get_if_dhcp_ipv6
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_DHCP_IPV6 > *bufLen) {
        *bufLen = DDP_FIELD_LEN_DHCP_IPV6;
        return -2;
    }
    /* copy the ipv6 DHCP status of the specified interface to buf
     * do not write beyond bufLen
     */
    //memcpy((INT1*)buf, (INT1*)pf_ipv6Dhcp, bufLen);
    return 0;
}

INT4
ddp_platform_get_if_alert_report
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_ALERT_REPORT > *bufLen) {
        *bufLen = DDP_FIELD_LEN_ALERT_REPORT;
        return -2;
    }
    /* copy the device alert status to buf
     * do not write beydon bueLen
     */
    //memcpy((INT1*)buf, (INT1*)pf_alertReport, bufLen);
    return 0;
}

INT4
ddp_platform_get_if_alert_report_info
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_ALERT_REPORT_INFO > *bufLen) {
        *bufLen = DDP_FIELD_LEN_ALERT_REPORT_INFO;
        return -2;
    }
    /* copy latest alert info string to buf
     * do not write beyond bufLen
     */
    strcpy((INT1*)buf, "Test device got a critical event");
    return 0;
}

INT4
ddp_platform_get_if_reboot_wait_time
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_REBOOT_WAIT_TIME > *bufLen) {
        *bufLen = DDP_FIELD_LEN_REBOOT_WAIT_TIME;
        return -2;
    }
    /* copy reboot wait time to buf
     * do not write beyond bufLen
     */
    memcpy((INT1*)buf, (INT1*)&pf_rebootWaitTime, sizeof(pf_rebootWaitTime));
    return 0;
}

INT4
ddp_platform_get_if_fw_local_path
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_FW_LOCAL_PATH > *bufLen) {
        *bufLen = DDP_FIELD_LEN_FW_LOCAL_PATH;
        return -2;
    }
    /* copy fw upgrade local file path to buf
     * do not write beyond bufLen
     */
    strncpy((INT1*)buf, (INT1*)pf_fwLocalPath, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_cfg_backup_local_path
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_CFG_BACKUP_LOCAL_PATH > *bufLen) {
        *bufLen = DDP_FIELD_LEN_CFG_BACKUP_LOCAL_PATH;
        return -2;
    }
    /* copy cfg backup local file path to buf
     * do not write beyond bufLen
     */
    strncpy((INT1*)buf, (INT1*)pf_cfgBackupLocalPath, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_cfg_restore_local_path
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_CFG_RESTORE_LOCAL_PATH > *bufLen) {
        *bufLen = DDP_FIELD_LEN_CFG_RESTORE_LOCAL_PATH;
        return -2;
    }
    /* copy cfg restore local file path to buf
     * do not write beyond bufLen
     */
    strncpy((INT1*)buf, (INT1*)pf_cfgRestoreLocalPath, *bufLen);
    return 0;
}

INT4
ddp_platform_get_ntp_enable
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_NTP_ENABLE > *bufLen) {
        *bufLen = DDP_FIELD_LEN_NTP_ENABLE;
        return -2;
    }
    /* copy ntp enable to buf
     * do not write beyond bufLen
     */
    memcpy((INT1*)buf, &pf_ntpEnable, sizeof(pf_ntpEnable));
    return 0;
}

INT4
ddp_platform_get_ntp_server
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if ((DDP_FIELD_LEN_NTP_SERVER + DDP_FIELD_LEN_NTP_FORMAT)> *bufLen) {
        *bufLen = (DDP_FIELD_LEN_NTP_SERVER + DDP_FIELD_LEN_NTP_FORMAT);
        return -2;
    }
    /* copy ntp server to buf
     * do not write beyond bufLen
     */
    struct ntp_addr *addr = (struct ntp_addr*)buf;
    addr->format = pf_ntpFormat;
    memcpy((void*)addr->buf, (void*)pf_ntpServer, DDP_FIELD_LEN_NTP_SERVER);
    //strncpy((INT1*)buf, (INT1*)pf_ntpServer, *bufLen);
    return 0;
}

INT4
ddp_platform_get_ntp_support
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_NTP_SUPPORT > *bufLen) {
        *bufLen = DDP_FIELD_LEN_NTP_SUPPORT;
        return -2;
    }
    /* copy ntp server supprot format to buf
     * do not write beyond bufLen
     */
    *buf = vc_ntpSupport;
    return 0;
}

INT4
ddp_platform_get_dst
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (*bufLen < sizeof(struct dst_setting)) {
        *bufLen = sizeof(struct dst_setting);
        return -2;
    }
    struct dst_setting *dst = (struct dst_setting*)buf;
    dst->enable = pf_dst_enable; dst->offset = pf_dst_offset;
    dst->start_year = pf_dst_start_year; dst->start_month = pf_dst_start_month;
    dst->start_day = pf_dst_start_day; dst->start_hour = pf_dst_start_hour;
    dst->start_minute = pf_dst_start_minute; dst->end_year = pf_dst_end_year;
    dst->end_month = pf_dst_end_month; dst->end_day = pf_dst_end_day;
    dst->end_hour = pf_dst_end_hour; dst->end_minute = pf_dst_end_minute;
    return 0;
}

INT4
ddp_platform_get_spec_ver
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || bufLen == NULL) { return -1; }
    if (DDP_FIELD_LEN_SPEC_VERSION > *bufLen) {
        *bufLen = DDP_FIELD_LEN_SPEC_VERSION;
        return -2;
    }
    /* copy ntp server to buf
     * do not write beyond bufLen
     */
    strncpy((INT1*)buf, (INT1*)pf_specVer, *bufLen);
    return 0;
}

INT4
ddp_platform_get_if_snmp_global_state
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || *bufLen == 0) { return -1; }
    /* get SNMP global state of the specified interface from platform
     */
    *buf = pf_snmpGlobalState;
    return 0;
}

INT4
ddp_platform_get_if_snmp_community_name_ro
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || *bufLen == 0) { return -1; }
    /* get SNMP community name (read-only) of the specified interface from platform
     * count the number and allocate memory with size (number * DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO))
     */
    INT4 i = 0;
    INT4 count = 0;
    UINT1 *arr = NULL;
    struct snmp_container *sc = (struct snmp_container*)buf;
    /* get number */
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpCommunityNameRO[i]) > 0) {
            count++;
        }
    }
    arr = (UINT1*)malloc(count * DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO);
    if (arr == NULL) {
        return -2;
    }
    memset(arr, 0, (count * DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO));
    sc->number = count;
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpCommunityNameRO[i]) > 0) {
            strcpy((INT1*)(arr + i * DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO), (INT1*)pf_snmpCommunityNameRO[i]);
        }
    }
    sc->list = arr;
    return 0;
}

INT4
ddp_platform_get_if_snmp_community_name_rw
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || *bufLen == 0) { return -1; }
    /* get SNMP community name (read-write) of the specified interface from platform
     * count the number and allocate memory with size (number * DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW))
     */
    INT4 i = 0;
    INT4 count = 0;
    UINT1 *arr = NULL;
    struct snmp_container *sc = (struct snmp_container*)buf;
    /* get number */
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpCommunityNameRW[i]) > 0) {
            count++;
        }
    }
    arr = (UINT1*)malloc(count * DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW);
    if (arr == NULL) {
        return -2;
    }
    memset(arr, 0, (count * DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW));
    sc->number = count;
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpCommunityNameRW[i]) > 0) {
            strcpy((INT1*)(arr + i * DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW), (INT1*)pf_snmpCommunityNameRW[i]);
        }
    }
    sc->list = arr;
    return 0;
}

INT4
ddp_platform_get_if_snmp_view_name_ro
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || *bufLen == 0) { return -1; }
    /* get SNMP view name (read-only) of the specified interface from platform
     * count the number and allocate memory with size (number * DDP_FIELD_LEN_SNMP_VIEW_NAME_RO))
     */
    INT4 i = 0;
    INT4 count = 0;
    UINT1 *arr = NULL;
    struct snmp_container *sc = (struct snmp_container*)buf;
    /* get number */
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpViewNameRO[i]) > 0) {
            count++;
        }
    }
    arr = (UINT1*)malloc(count * DDP_FIELD_LEN_SNMP_VIEW_NAME_RO);
    if (arr == NULL) {
        return -2;
    }
    memset(arr, 0, (count * DDP_FIELD_LEN_SNMP_VIEW_NAME_RO));
    sc->number = count;
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpViewNameRO[i]) > 0) {
            strcpy((INT1*)(arr + i * DDP_FIELD_LEN_SNMP_VIEW_NAME_RO), (INT1*)pf_snmpViewNameRO[i]);
        }
    }
    sc->list = arr;
    return 0;
}

INT4
ddp_platform_get_if_snmp_view_name_rw
(
    struct ddp_interface* ifs,
    UINT1* buf,
    INT4* bufLen
)
{
    if (buf == NULL || *bufLen == 0) { return -1; }
    /* get SNMP view name (read-write) of the specified interface from platform
     * count the number and allocate memory with size (number * DDP_FIELD_LEN_SNMP_VIEW_NAME_RW))
     */
    INT4 i = 0;
    INT4 count = 0;
    UINT1 *arr = NULL;
    struct snmp_container *sc = (struct snmp_container*)buf;
    /* get number */
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpViewNameRW[i]) > 0) {
            count++;
        }
    }
    arr = (UINT1*)malloc(count * DDP_FIELD_LEN_SNMP_VIEW_NAME_RW);
    if (arr == NULL) {
        return -2;
    }
    memset(arr, 0, (count * DDP_FIELD_LEN_SNMP_VIEW_NAME_RW));
    sc->number = count;
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpViewNameRW[i]) > 0) {
            strcpy((INT1*)(arr + i * DDP_FIELD_LEN_SNMP_VIEW_NAME_RW), (INT1*)pf_snmpViewNameRW[i]);
        }
    }
    sc->list = arr;
    return 0;
}

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
INT4
ddp_platform_get_field
(
    struct ddp_interface* ifs,
    INT4 field,
    UINT1* buf,
    INT4* bufLen
)
{
    INT4 retVal = 0;
    switch (field) {
        case DDP_FIELD_BRAND_NAME:
            retVal = ddp_platform_get_if_brand_name(ifs, buf, bufLen);
            break;
        case DDP_FIELD_PRODUCT_NAME:
            retVal = ddp_platform_get_if_product_name(ifs, buf, bufLen);
            break;
        case DDP_FIELD_MODEL_NAME:
            retVal = ddp_platform_get_if_model_name(ifs, buf, bufLen);
            break;
        case DDP_FIELD_VERSION:
            retVal = ddp_platform_get_if_version(ifs, buf, bufLen);
            break;
        case DDP_FIELD_PRODUCT_TYPE:
            retVal = ddp_platform_get_if_prod_type(ifs, buf, bufLen);
            break;
        case DDP_FIELD_PRODUCT_HW_SPEC:
            retVal = ddp_platform_get_if_prod_hw_spec(ifs, buf, bufLen);
            break;
        case DDP_FIELD_MAC_ADDR:
            retVal = ddp_platform_get_if_mac(ifs, buf, bufLen);
            break;
        case DDP_FIELD_SYSTEM_NAME:
            retVal = ddp_platform_get_if_system_name(ifs, buf, bufLen);
            break;
        case DDP_FIELD_DEVICE_IP_ADDR:
            retVal = ddp_platform_get_if_ipv4(ifs, buf, bufLen);
            break;
        case DDP_FIELD_WEB_SERVICE_PORT:
            retVal = ddp_platform_get_if_web_service_port(ifs, buf, bufLen);
            break;
        case DDP_FIELD_CUSTOMIZED_DNS:
            retVal = ddp_platform_get_if_customized_dns(ifs, buf, bufLen);
            break;
        case DDP_FIELD_SUBNET_MASK:
            retVal = ddp_platform_get_if_subnet_mask(ifs, buf, bufLen);
            break;
        case DDP_FIELD_DEFAULT_GATEWAY:
            retVal = ddp_platform_get_if_default_gateway(ifs, buf, bufLen);
            break;
        case DDP_FIELD_PRIMARY_DNS:
            retVal = ddp_platform_get_if_primary_dns(ifs, buf, bufLen);
            break;
        case DDP_FIELD_SECONDARY_DNS:
            retVal = ddp_platform_get_if_second_dns(ifs, buf, bufLen);
            break;
        case DDP_FIELD_DHCP:
            retVal = ddp_platform_get_if_dhcp(ifs, buf, bufLen);
            break;
        case DDP_FIELD_DATE_TIME:
            retVal = ddp_platform_get_if_date_time(ifs, buf, bufLen);
            break;
        case DDP_FIELD_TIMEZONE:
            retVal = ddp_platform_get_if_time_zone(ifs, buf, bufLen);
            break;
        case DDP_FIELD_DAYLIGHT_SAVING:
            retVal = ddp_platform_get_if_daylight_saving(ifs, buf, bufLen);
            break;
        case DDP_FIELD_PRODUCT_CATEGORY:
            retVal = ddp_platform_get_if_prod_category(ifs, buf, bufLen);
            break;
        case DDP_FIELD_HARDWARE_VERSION:
            retVal = ddp_platform_get_if_hw_version(ifs, buf, bufLen);
            break;
        case DDP_FIELD_SERIAL_NUMBER:
            retVal = ddp_platform_get_if_serial_number(ifs, buf, bufLen);
            break;
        case DDP_FIELD_INTERFACE_NUMBER:
            retVal = ddp_platform_get_if_interface_number(ifs, buf, bufLen);
            break;
        case DDP_FIELD_DEVICE_IPV6_ADDR:
            retVal = ddp_platform_get_if_ipv6(ifs, buf, bufLen);
            break;
        case DDP_FIELD_DEVICE_IPV6_PREFIX:
            retVal = ddp_platform_get_if_ipv6_prefix(ifs, buf, bufLen);
            break;
        case DDP_FIELD_DEFAULT_GATEWAY_IPV6:
            retVal = ddp_platform_get_if_def_gateway_ipv6(ifs, buf, bufLen);
            break;
        case DDP_FIELD_PRIMARY_DNS_IPV6:
            retVal = ddp_platform_get_if_primary_dns_ipv6(ifs, buf, bufLen);
            break;
        case DDP_FIELD_SECONDARY_DNS_IPV6:
            retVal = ddp_platform_get_if_second_dns_ipv6(ifs, buf, bufLen);
            break;
        case DDP_FIELD_DHCP_IPV6:
            retVal = ddp_platform_get_if_dhcp_ipv6(ifs, buf, bufLen);
            break;
        case DDP_FIELD_ALERT_REPORT:
            retVal = ddp_platform_get_if_alert_report(ifs, buf, bufLen);
            break;

        case DDP_FIELD_ALERT_REPORT_INFO:
            retVal = ddp_platform_get_if_alert_report_info(ifs, buf, bufLen);
            break;
        case DDP_FIELD_REBOOT_WAIT_TIME:
            retVal = ddp_platform_get_if_reboot_wait_time(ifs, buf, bufLen);
            break;
        case DDP_FIELD_FW_LOCAL_PATH:
            retVal = ddp_platform_get_if_fw_local_path(ifs, buf, bufLen);
            break;
        case DDP_FIELD_CFG_BACKUP_LOCAL_PATH:
            retVal = ddp_platform_get_if_cfg_backup_local_path(ifs, buf, bufLen);
            break;
        case DDP_FIELD_CFG_RESTORE_LOCAL_PATH:
            retVal = ddp_platform_get_if_cfg_restore_local_path(ifs, buf, bufLen);
            break;

        /* 0x00D2: get VLAN */
        case DDP_FIELD_VLAN_NAME:
            retVal = ddp_platform_get_if_vlan_name(ifs, buf, bufLen);
            break;
        case DDP_FIELD_VLAN_ID:
            retVal = ddp_platform_get_if_vlan_id(ifs, buf, bufLen);
            break;
        case DDP_FIELD_VLAN_TAG_PLIST_TYPE:
            retVal = ddp_platform_get_if_vlan_tag_list_type(ifs, buf, bufLen);
            break;
        case DDP_FIELD_VLAN_TAG_PLIST:
            retVal = ddp_platform_get_if_vlan_tag_list(ifs, buf, bufLen);
            break;
        case DDP_FIELD_VLAN_UNTAG_PLIST_TYPE:
            retVal = ddp_platform_get_if_vlan_untag_list_type(ifs, buf, bufLen);
            break;
        case DDP_FIELD_VLAN_UNTAG_PLIST:
            retVal = ddp_platform_get_if_vlan_untag_list(ifs, buf, bufLen);
            break;
        case DDP_FIELD_VLAN_LIST:
            retVal = ddp_platform_get_if_vlan_list(ifs, buf, bufLen);
            break;
        case DDP_FIELD_VLAN_STATE:
            retVal = ddp_platform_get_if_vlan_state(ifs, buf, bufLen);
            break;

        /* 0x00D3: get ASV */
        case DDP_FIELD_ASV_ID:
            retVal = ddp_platform_get_if_asv_id(ifs, buf, bufLen);
            break;
        case DDP_FIELD_ASV_PRI:
            retVal = ddp_platform_get_if_asv_priority(ifs, buf, bufLen);
            break;
        case DDP_FIELD_ASV_STATE:
            retVal = ddp_platform_get_if_asv_state(ifs, buf, bufLen);
            break;
        case DDP_FIELD_ASV_TAG_PLIST_TYPE:
            retVal = ddp_platform_get_if_asv_tag_list_type(ifs, buf, bufLen);
            break;
        case DDP_FIELD_ASV_TAG_PLIST:
            retVal = ddp_platform_get_if_asv_tag_list(ifs, buf, bufLen);
            break;

        /* 0x00D5, 0x00D6: get user defined MAC information for ASV */
        case DDP_FIELD_ASV_USR_DEF_MAC:
            retVal = ddp_platform_get_if_asv_usr_def_mac(ifs, buf, bufLen);
            break;

        /* 0x00D7: get device port number */
        case DDP_FIELD_PORT_NUM_GET:
            retVal = ddp_platform_get_if_port_number(ifs, buf, bufLen);
            break;

        case DDP_FIELD_NTP_ENABLE:
            retVal = ddp_platform_get_ntp_enable(ifs, buf, bufLen);
            break;
        case DDP_FIELD_NTP_SERVER:
            retVal = ddp_platform_get_ntp_server(ifs, buf, bufLen);
            break;
        case DDP_FIELD_NTP_SUPPORT:
            retVal = ddp_platform_get_ntp_support(ifs, buf, bufLen);
            break;
        /* DST */
        case DDP_FIELD_DST_SETTING:
            retVal = ddp_platform_get_dst(ifs, buf, bufLen);
            break;
        case DDP_FIELD_SPEC_VERSION:
            retVal = ddp_platform_get_spec_ver(ifs, buf, bufLen);
            break;

        /* Get SNMP */
        case DDP_FIELD_SNMP_GLOBAL_STATE:
            retVal = ddp_platform_get_if_snmp_global_state(ifs, buf, bufLen);
            break;
        case DDP_FIELD_SNMP_COMMUNITY_NAME_RO:
            retVal = ddp_platform_get_if_snmp_community_name_ro(ifs, buf, bufLen);
            break;
        case DDP_FIELD_SNMP_COMMUNITY_NAME_RW:
            retVal = ddp_platform_get_if_snmp_community_name_rw(ifs, buf, bufLen);
            break;
        case DDP_FIELD_SNMP_VIEW_NAME_RO:
            retVal = ddp_platform_get_if_snmp_view_name_ro(ifs, buf, bufLen);
            break;
        case DDP_FIELD_SNMP_VIEW_NAME_RW:
            retVal = ddp_platform_get_if_snmp_view_name_rw(ifs, buf, bufLen);
            break;

        case DDP_FIELD_RESET:
        case DDP_FIELD_FACTORY_RESET:

        default:
            DDP_DEBUG("get field (id %d) is not implemented\n", field);
            break;
    }
    return retVal;
}

INT4
ddp_platform_set_if_system_name
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (ifs == NULL || data == NULL) { return -1; }
    if (dataLen >= DDP_FIELD_LEN_SYSTEM_NAME) { dataLen = DDP_FIELD_LEN_SYSTEM_NAME; }

    FILE *pFileIn = NULL, *pFileOut = NULL;
    INT1 line[255];
    memset(line, 0, strlen(line));

    char *filePath = "/etc/sysconfig/config/hostname";
    char filePathTemp[255];
    memset(filePathTemp, 0, strlen(filePathTemp));
    sprintf(filePathTemp, "%s.tmp", filePath);

    pFileIn = fopen(filePath, "r");
    pFileOut = fopen(filePathTemp, "w");

    if(pFileIn && pFileOut) {
    	while(fgets(line, 255, pFileIn))
    	{
    		char lineOut[255];
    		memset(lineOut, 0, strlen(lineOut));
    		strcpy(lineOut, line);
    		fputs(lineOut, pFileOut);
    	}
    	fclose(pFileIn);
    	fclose(pFileOut);
    	rename(filePathTemp, filePath);
    }
    else {
    	DDP_DEBUG("Open file failed.\n");
    	return -2;
    }
    system("hostname -F /etc/sysconfig/config/hostname");

    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "System name %s -> %s\n", pf_sysName, data);
    memset(pf_sysName, 0, sizeof(pf_sysName));
    memcpy((INT1*)pf_sysName, (INT1*)data, dataLen);
    return 0;
}

INT4
ddp_platform_set_if_ipv4
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
	/* --------------- todo ------------------ */

    if (ifs == NULL || data == NULL) { return -1; }
    /* set new ipv4 address of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "IP %d.%d.%d.%d -> ", *pf_ip, *(pf_ip + 1), *(pf_ip + 2), *(pf_ip + 3));
    memcpy(pf_ip, data, DDP_FIELD_LEN_DEVICE_IP_ADDR);
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "%d.%d.%d.%d\n", *pf_ip, *(pf_ip + 1), *(pf_ip + 2), *(pf_ip + 3));
    return 0;
}

INT4
ddp_platform_set_if_web_service_port
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (ifs == NULL || data == NULL) { return -1; }
    if (dataLen > DDP_FIELD_LEN_WEB_SERVICE_PORT) { dataLen = DDP_FIELD_LEN_WEB_SERVICE_PORT; }

    FILE *pFileIn = NULL, *pFileOut = NULL;
    INT1 line[255];
    memset(line, 0, strlen(line));

    char *filePath = "ddp_config";
    char filePathTemp[255];
    memset(filePathTemp, 0, strlen(filePathTemp));
    sprintf(filePathTemp, "%s.tmp", filePath);

    pFileIn = fopen(filePath, "r");
    pFileOut = fopen(filePathTemp, "w");

    if(pFileIn && pFileOut) {
    	while(fgets(line, 255, pFileIn))
    	{
    		char lineOut[255];
    		memset(lineOut, 0, strlen(lineOut));

    		char *key = "webport";
    		int len = strlen(key);
    		if(strncmp(line, key, len) == 0 && line[len] == '=') {
    			if(line[strlen(line)-1] == '\n')
    				sprintf(lineOut, "%s=%d\n", key, *data);
    			else
    				sprintf(lineOut, "%s=%d", key, *data);
    		}
    		else
    			strcpy(lineOut, line);

    		fputs(lineOut, pFileOut);
    	}
    	fclose(pFileIn);
    	fclose(pFileOut);
    	rename(filePathTemp, filePath);
    }
    else {
    	DDP_DEBUG("Open file failed.\n");
    	return -2;
    }

    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Web service port %d -> %d\n", pf_webPort, (UINT2)(*data));
    pf_webPort = (UINT2)*data;
    return 0;
}

INT4
ddp_platform_set_if_customized_dns
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
	/* --------------- todo ------------------ */

    if (ifs == NULL || data == NULL) { return -1; }
    if (dataLen > DDP_FIELD_LEN_CUSTOMIZED_DNS) { dataLen = DDP_FIELD_LEN_CUSTOMIZED_DNS; }
    /* set the new customized DNS ipv4 address of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Customized DNS %d -> %d\n", pf_customizedDns, *data);
    memset(&pf_customizedDns, 0, sizeof(pf_customizedDns));
    memcpy((INT1*)&pf_customizedDns, (INT1*)data, dataLen);
    return 0;
}

INT4
ddp_platform_set_if_subnet_mask
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (ifs == NULL || data == NULL) { return -1; }
    if (dataLen > DDP_FIELD_LEN_SUBNET_MASK) { dataLen = DDP_FIELD_LEN_SUBNET_MASK; }
    /* set new ipv4 subnet mask of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Subnet mask %d.%d.%d.%d -> ", *pf_subnetMask, *(pf_subnetMask + 1), *(pf_subnetMask + 2), *(pf_subnetMask + 3));
    memcpy(pf_subnetMask, data, DDP_FIELD_LEN_SUBNET_MASK);
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "%d.%d.%d.%d\n", *pf_subnetMask, *(pf_subnetMask + 1), *(pf_subnetMask + 2), *(pf_subnetMask + 3));
    return 0;
}

INT4
ddp_platform_set_if_default_gateway
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
	/* --------------- todo ------------------ */

    if (ifs == NULL || data == NULL) { return -1; }
    if (dataLen > DDP_FIELD_LEN_DEFAULT_GATEWAY) { dataLen = DDP_FIELD_LEN_DEFAULT_GATEWAY; }
    /* set new default gateway ipv4 address of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Default gateway %d.%d.%d.%d -> ", *pf_defaultGateway, *(pf_defaultGateway + 1), *(pf_defaultGateway + 2), *(pf_defaultGateway + 3));
    memcpy(pf_defaultGateway, data, DDP_FIELD_LEN_SUBNET_MASK);
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "%d.%d.%d.%d\n", *pf_defaultGateway, *(pf_defaultGateway + 1), *(pf_defaultGateway + 2), *(pf_defaultGateway + 3));
    return 0;
}

INT4
ddp_platform_set_if_primary_dns
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (ifs == NULL || data == NULL) { return -1; }
    if (dataLen > DDP_FIELD_LEN_PRIMARY_DNS) { dataLen = DDP_FIELD_LEN_PRIMARY_DNS; }

    FILE *pFileIn = NULL, *pFileOut = NULL;
    INT1 line[255];
    memset(line, 0, strlen(line));
    char sDnsAddr[255];
    memset(sDnsAddr, 0, 255);

    if (inet_ntop(AF_INET, (void *)data, sDnsAddr, sizeof(sDnsAddr)) == 0) {
		DDP_DEBUG("inet_ntop\n");
    	return -2;
	}

    char *filePath = "/etc/resolv.conf";
    char filePathTemp[255];
    memset(filePathTemp, 0, strlen(filePathTemp));
    sprintf(filePathTemp, "%s.tmp", filePath);

    pFileIn = fopen(filePath, "r");
    pFileOut = fopen(filePathTemp, "w");

    if(pFileIn && pFileOut) {
    	int count = 0;
    	while(fgets(line, 255, pFileIn))
    	{
    		char *p = strstr(line, "nameserver ");
    		char lineOut[255];
    		memset(lineOut, 0, strlen(lineOut));

    		if(p != NULL && count < 1) {
    			count++;
    			if(line[strlen(line)-1] == '\n')
    				sprintf(lineOut, "nameserver %s\n", sDnsAddr);
    			else
    				sprintf(lineOut, "nameserver %s", sDnsAddr);
    		}
    		else
    			strcpy(lineOut, line);

			fputs(lineOut, pFileOut);
    	}
    	fclose(pFileIn);
    	fclose(pFileOut);
    	rename(filePathTemp, filePath);
    }
    else {
    	DDP_DEBUG("Open file failed.\n");
    	return -2;
    }

    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Primary DNS %d.%d.%d.%d -> ", *pf_primaryDns, *(pf_primaryDns + 1), *(pf_primaryDns + 2), *(pf_primaryDns + 3));
    memcpy(pf_primaryDns, data, DDP_FIELD_LEN_SUBNET_MASK);
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "%d.%d.%d.%d\n", *pf_primaryDns, *(pf_primaryDns + 1), *(pf_primaryDns + 2), *(pf_primaryDns + 3));
    return 0;
}

INT4
ddp_platform_set_if_second_dns
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{

    if (ifs == NULL || data == NULL) { return -1; }
    if (dataLen > DDP_FIELD_LEN_SECONDARY_DNS) { dataLen = DDP_FIELD_LEN_SECONDARY_DNS; }

    FILE *pFileIn = NULL, *pFileOut = NULL;
    INT1 line[255];
    memset(line, 0, strlen(line));
    char sDnsAddr[255];
    memset(sDnsAddr, 0, 255);

    if (inet_ntop(AF_INET, (void *)data, sDnsAddr, sizeof(sDnsAddr)) == 0) {
		DDP_DEBUG("inet_ntop\n");
    	return -2;
	}

    char *filePath = "/etc/resolv.conf";
    char filePathTemp[255];
    memset(filePathTemp, 0, strlen(filePathTemp));
    sprintf(filePathTemp, "%s.tmp", filePath);

    pFileIn = fopen(filePath, "r");
    pFileOut = fopen(filePathTemp, "w");

    if(pFileIn && pFileOut) {
    	int count = 0;
    	while(fgets(line, 255, pFileIn))
    	{
    		char *p = strstr(line, "nameserver ");
    		char lineOut[255];
    		memset(lineOut, 0, strlen(lineOut));

    		if(p != NULL && count < 2) {
    			count++;
    			if(count == 1)
    				strcpy(lineOut, line);
    			else {
    				if(line[strlen(line)-1] == '\n')
    					sprintf(lineOut, "nameserver %s\n", sDnsAddr);
    				else
    					sprintf(lineOut, "nameserver %s", sDnsAddr);
    			}
    		}
    		else
    			strcpy(lineOut, line);

			fputs(lineOut, pFileOut);
    	}
    	fclose(pFileIn);
    	fclose(pFileOut);
    	rename(filePathTemp, filePath);
    }
    else {
    	DDP_DEBUG("Open file failed.\n");
    	return -2;
    }

    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Secondary DNS %d.%d.%d.%d -> ", *pf_secondDns, *(pf_secondDns + 1), *(pf_secondDns + 2), *(pf_secondDns + 3));
    memcpy(pf_secondDns, data, DDP_FIELD_LEN_SUBNET_MASK);
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "%d.%d.%d.%d\n", *pf_secondDns, *(pf_secondDns + 1), *(pf_secondDns + 2), *(pf_secondDns + 3));
    return 0;
}

INT4
ddp_platform_set_if_dhcp
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
	/* --------------- todo ------------------ */

    if (ifs == NULL || data == NULL) { return -1; }
    if (dataLen > DDP_FIELD_DHCP) { dataLen = DDP_FIELD_LEN_DHCP; }
    /* set new ipv4 DHCP status of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "DHCP %d -> %d\n", pf_dhcp, *data);
    memset(&pf_dhcp, 0, sizeof(pf_dhcp));
    memcpy((INT1*)&pf_dhcp, (INT1*)data, dataLen);
    return 0;
}

INT4
ddp_platform_set_if_ipv6
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (ifs == NULL || data == NULL) { return -1; }
    /* set new ipv6 address of the specified interface to platform
     */
    INT4 i = 0;
    if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
        printf("IPv6: ");
        for (i = 0; i < dataLen; i++) {
            printf("%02X", *(data + i));
            if (i < (dataLen - 1)) { printf(":"); }
        }
        printf("\n");
    }
    return 0;
}

INT4
ddp_platform_set_if_ipv6_prefix
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (ifs == NULL || data == NULL) { return -1; }
    /* set new ipv6 prefix of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "IPv6 prefix: %d\n", *data);
    return 0;
}

INT4
ddp_platform_set_if_def_gateway_ipv6
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (ifs == NULL || data == NULL) { return -1; }
    /* set new default gateway ipv6 address of the specified interface to platform
     */
    INT4 i = 0;
    if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
        printf("IPv6 gateway: ");
        for (i = 0; i < dataLen; i++) {
            printf("%02X", *(data + i));
            if (i < (dataLen - 1)) { printf(":"); }
        }
        printf("\n");
    }
    return 0;
}

INT4
ddp_platform_set_if_primary_dns_ipv6
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (ifs == NULL || data == NULL) { return -1; }
    /* set new primary DNS ipv6 address of the specified interface to platform
     */
    INT4 i = 0;
    if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
        printf("IPv6 primary DNS: ");
        for (i = 0; i < dataLen; i++) {
            printf("%02X", *(data + i));
            if (i < (dataLen - 1)) { printf(":"); }
        }
        printf("\n");
    }
    return 0;
}

INT4
ddp_platform_set_if_second_dns_ipv6
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (ifs == NULL || data == NULL) { return -1; }
    /* set new secondary DNS ipv6 address of the specified interface to platform
     */
    INT4 i = 0;
    if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
        printf("IPv6 secondary DNS: ");
        for (i = 0; i < dataLen; i++) {
            printf("%02X", *(data + i));
            if (i < (dataLen - 1)) { printf(":"); }
        }
        printf("\n");
    }
    return 0;
}

INT4
ddp_platform_set_if_dhcp_ipv6
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (ifs == NULL || data == NULL) { return -1; }
    /* set new DHCP ipv6 address of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "IPv6 DHCP: %d\n", *data);
    return 0;
}

INT4
ddp_platform_set_if_reboot
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4   dataLen
)
{
    if (ifs == NULL) { return -1; }
    /* Reboot the system. It should not go back, unless something wrong. */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "SYSTEM IS REBOOTING\n");
    return 0;
}

INT4
ddp_platform_set_if_reset
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4   dataLen
)
{
    if (ifs == NULL) { return -1; }
    /* Reset */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "RESETTING\n");
    return 0;
}

INT4
ddp_platform_set_if_factory_reset
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4   dataLen
)
{
    if (ifs == NULL) { return -1; }
    /* Factory reset */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "FACTORY RESETTING\n");
    return 0;
}

INT4
ddp_platform_set_if_snmp_global_state
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return -1; }
    /* set new SNMP global state of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "SNMP global state %d -> %d\n", pf_snmpGlobalState, *data);
    pf_snmpGlobalState = *data;
    return 0;
}

INT4
ddp_platform_set_if_snmp_community_name_ro
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return -1; }
    if (dataLen >= DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO) {
        dataLen = DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO;
    }
    /* set new SNMP community name (read-only) of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Set SNMP community name RO: %s\n", data);
    INT4 i = 0;
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpCommunityNameRO[i]) == 0) {
            strncpy((INT1*)pf_snmpCommunityNameRO[i], (INT1*)data, dataLen);
            return 0;
        }
    }
    return -5;
}

INT4
ddp_platform_set_if_snmp_community_name_rw
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return -1; }
    if (dataLen >= DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW) {
        dataLen = DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW;
    }
    /* set new SNMP community name (read-write) of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Set SNMP community name RW: %s\n", data);
    INT4 i = 0;
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpCommunityNameRW[i]) == 0) {
            strncpy((INT1*)pf_snmpCommunityNameRW[i], (INT1*)data, dataLen);
            return 0;
        }
    }
    return -5;
}

INT4
ddp_platform_set_if_snmp_view_name_ro
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return -1; }
    if (strlen((INT1*)data) == 0) { return DDP_INVALID_FIELD_SNMP_VIEWNAME_LOST; }
    if (dataLen >= DDP_FIELD_LEN_SNMP_VIEW_NAME_RO) {
        dataLen = DDP_FIELD_LEN_SNMP_VIEW_NAME_RO;
    }
    /* set new SNMP view name (read-only) of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Set SNMP view name RO: %s\n", data);
    INT4 i = 0;
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpViewNameRO[i]) == 0) {
            strncpy((INT1*)pf_snmpViewNameRO[i], (INT1*)data, dataLen);
            return 0;
        }
    }
    return -5;
}

INT4
ddp_platform_set_if_snmp_view_name_rw
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return -1; }
    if (strlen((INT1*)data) == 0) { return DDP_INVALID_FIELD_SNMP_VIEWNAME_LOST; }
    if (dataLen >= DDP_FIELD_LEN_SNMP_VIEW_NAME_RW) {
        dataLen = DDP_FIELD_LEN_SNMP_VIEW_NAME_RW;
    }
    /* set new SNMP view name (read-write) of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Set SNMP view name RW: %s\n", data);
    INT4 i = 0;
    for (i = 0; i < DDP_PLATFORM_SNMP_NAMES_MAX; i++) {
        if (strlen((INT1*)pf_snmpViewNameRW[i]) == 0) {
            strncpy((INT1*)pf_snmpViewNameRW[i], (INT1*)data, dataLen);
            return 0;
        }
    }
    return -5;
}

INT4
ddp_platform_set_date_time
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return DDP_INVALID_FIELD_DATETIME; }
    if (strlen((INT1*)data) == 0) { return DDP_INVALID_FIELD_DATETIME; }
    if (dataLen >= DDP_FIELD_LEN_DATE_TIME) {
        dataLen = DDP_FIELD_LEN_DATE_TIME;
    }
    /* set new date time of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Date time: %s\n", data);
    return 0;
}

INT4
ddp_platform_set_time_zone
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return DDP_INVALID_FIELD_TIMEZONE; }
    if (dataLen >= DDP_FIELD_LEN_TIMEZONE) {
        dataLen = DDP_FIELD_LEN_TIMEZONE;
    }
    /* set new time zone of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Time zone: %02X %02X\n", *data, *(data + 1));
    return 0;
}

INT4
ddp_platform_set_daylight_saving
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return DDP_INVALID_FIELD_DL_SAVING; }
    if (dataLen >= DDP_FIELD_LEN_DAYLIGHT_SAVING) {
        dataLen = DDP_FIELD_LEN_DAYLIGHT_SAVING;
    }
    /* set new daylight saving of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Daylight saving: %X\n", *data);
    return 0;
}

INT4
ddp_platform_set_ntp_enable
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return DDP_INVALID_FIELD_NTP_ENABLE; }
    /* set new NTP enable of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "NTP enable: %d -> %d\n", pf_ntpEnable, *data);
    pf_ntpEnable = *data;
    return 0;
}

INT4
ddp_platform_set_ntp_server
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return DDP_INVALID_FIELD_NTP_SERVER; }
    if (strlen((INT1*)data) == 0) { return DDP_INVALID_FIELD_NTP_SERVER; }
    if (dataLen >= DDP_FIELD_LEN_NTP_SERVER + 1) {
        dataLen = DDP_FIELD_LEN_NTP_SERVER + 1;
    }
    /* set new NTP server of the specified interface to platform
     */
    INT4 i = 0;
    struct ntp_addr *addr = (struct ntp_addr*)data;
    if (addr->format >= 0 && addr->format <= 2) {
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "NTP server format: %d -> %d\n", pf_ntpFormat, addr->format);
    } else {
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "NTP server format error (%d)\n", addr->format);
        return DDP_INVALID_FIELD_NTP_SERVER;
    }
    /* check whether the input format is supported or not */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "NTP server format support: %X, input format is %d\n", vc_ntpSupport, addr->format);
    if (addr->format == 0) {
        if ((vc_ntpSupport & 0x1) != 0) {
            return DDP_INVALID_FIELD_NTP_SERVER;
        }
    } else if (addr->format == 1) {
        if ((vc_ntpSupport & 0x02) != 0) {
            return DDP_INVALID_FIELD_NTP_SERVER;
        }
    } else if (addr->format == 2) {
        if ((vc_ntpSupport & 0x04) != 0) {
            return DDP_INVALID_FIELD_NTP_SERVER;
        }
    }
    if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
        printf("NTP server address: ");
        if (pf_ntpFormat == 0) {
            printf("%s -> ", pf_ntpServer);
        } else if (pf_ntpFormat == 1) {
            printf("%d.%d.%d.%d -> ", pf_ntpServer[0], pf_ntpServer[1], pf_ntpServer[2], pf_ntpServer[3]);
        } else if (pf_ntpFormat == 2) {
            for (i = 0; i < 16; i++) {
                printf("%02X", pf_ntpServer[i]);
                if (i < 14) { printf(":"); }
            }
            printf(" -> ");
        }
        if (addr->format == 0) {
            printf("%s\n", addr->buf);
        } else if (addr->format == 1) {
            printf("%d.%d.%d.%d\n", addr->buf[0], addr->buf[1], addr->buf[2], addr->buf[3]);
        } else if (addr->format == 2) {
            for (i = 0; i < 16; i++) {
                printf("%02X", addr->buf[i]);
                if (i < 14) { printf(":"); }
            }
            printf("\n");
        }
    }
    
    pf_ntpFormat = addr->format;
    memset(pf_ntpServer, 0, DDP_FIELD_LEN_NTP_SERVER);
    memcpy(pf_ntpServer, addr->buf, DDP_FIELD_LEN_NTP_SERVER);
    //DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "NTP server: %s -> %s\n", pf_ntpServer, data);
    //memset(pf_ntpServer, 0, DDP_FIELD_LEN_NTP_SERVER);
    //strncpy((INT1*)pf_ntpServer, (INT1*)data, dataLen);
    return 0;
}

INT4
ddp_platform_set_dst
(
    struct ddp_interface* ifs,
    UINT1* data,
    INT4 dataLen
)
{
    if (data == NULL || dataLen == 0) { return DDP_INVALID_FIELD_DL_SAVING; }
    if (dataLen < sizeof(struct dst_setting)) { return DDP_INVALID_FIELD_DL_SAVING; }
    struct dst_setting *dst = (struct dst_setting*)data;
    /* set new DST setting of the specified interface to the platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "DST enable: %d -> %d\n", pf_dst_enable, dst->enable);
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "DST start: %04d-%02d-%02d %02d:%02d -> %04d-%02d-%02d %02d:%02d\n", pf_dst_start_year, pf_dst_start_month, pf_dst_start_day, pf_dst_start_hour, pf_dst_start_minute, dst->start_year, dst->start_month, dst->start_day, dst->start_hour, dst->start_minute);
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "DST end  : %04d-%02d-%02d %02d:%02d -> %04d-%02d-%02d %02d:%02d\n", pf_dst_end_year, pf_dst_end_month, pf_dst_end_day, pf_dst_end_hour, pf_dst_end_minute, dst->end_year, dst->end_month, dst->end_day, dst->end_hour, dst->end_minute);
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "DST offset: %d -> %d\n", pf_dst_offset, dst->offset);
    pf_dst_enable = dst->enable; pf_dst_offset = dst->offset;
    pf_dst_start_year = dst->start_year; pf_dst_start_month = dst->start_month;
    pf_dst_start_day = dst->start_day; pf_dst_start_hour = dst->start_hour;
    pf_dst_start_minute = dst->start_minute; pf_dst_end_year = dst->end_year;
    pf_dst_end_month = dst->end_month; pf_dst_end_day = dst->end_day;
    pf_dst_end_hour = dst->end_hour; pf_dst_end_minute = dst->end_minute;
    return 0;
}

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
INT4
ddp_platform_set_field
(
    struct ddp_interface* ifs,
    INT4 field,
    UINT1* data,
    INT4 dataLen
)
{
    INT4 retVal = 0;
    switch (field) {
        case DDP_FIELD_SYSTEM_NAME:
            retVal = ddp_platform_set_if_system_name(ifs, data, dataLen);
            break;
        case DDP_FIELD_DEVICE_IP_ADDR:
            retVal = ddp_platform_set_if_ipv4(ifs, data, dataLen);
            break;
        case DDP_FIELD_WEB_SERVICE_PORT:
            retVal = ddp_platform_set_if_web_service_port(ifs, data, dataLen);
            break;
        case DDP_FIELD_CUSTOMIZED_DNS:
            retVal = ddp_platform_set_if_customized_dns(ifs, data, dataLen);
            break;
        case DDP_FIELD_SUBNET_MASK:
            retVal = ddp_platform_set_if_subnet_mask(ifs, data, dataLen);
            break;
        case DDP_FIELD_DEFAULT_GATEWAY:
            retVal = ddp_platform_set_if_default_gateway(ifs, data, dataLen);
            break;
        case DDP_FIELD_PRIMARY_DNS:
            retVal = ddp_platform_set_if_primary_dns(ifs, data, dataLen);
            break;
        case DDP_FIELD_SECONDARY_DNS:
            retVal = ddp_platform_set_if_second_dns(ifs, data, dataLen);
            break;
        case DDP_FIELD_DHCP:
            retVal = ddp_platform_set_if_dhcp(ifs, data, dataLen);
            break;
        case DDP_FIELD_DEVICE_IPV6_ADDR:
            retVal = ddp_platform_set_if_ipv6(ifs, data, dataLen);
            break;
        case DDP_FIELD_DEVICE_IPV6_PREFIX:
            retVal = ddp_platform_set_if_ipv6_prefix(ifs, data, dataLen);
            break;
        case DDP_FIELD_DEFAULT_GATEWAY_IPV6:
            retVal = ddp_platform_set_if_def_gateway_ipv6(ifs, data, dataLen);
            break;
        case DDP_FIELD_PRIMARY_DNS_IPV6:
            retVal = ddp_platform_set_if_primary_dns_ipv6(ifs, data, dataLen);
            break;
        case DDP_FIELD_SECONDARY_DNS_IPV6:
            retVal = ddp_platform_set_if_second_dns_ipv6(ifs, data, dataLen);
            break;
        case DDP_FIELD_DHCP_IPV6:
            retVal = ddp_platform_set_if_dhcp_ipv6(ifs, data, dataLen);
            break;
        case DDP_FIELD_REBOOT:
            retVal = ddp_platform_set_if_reboot(ifs, data, dataLen);
            break;
        case DDP_FIELD_RESET:
            retVal = ddp_platform_set_if_reset(ifs, data, dataLen);
            break;
        case DDP_FIELD_FACTORY_RESET:
            retVal = ddp_platform_set_if_factory_reset(ifs, data, dataLen);
            break;

        /* 0x00D2: set VLAN */
        case DDP_FIELD_VLAN_NAME:
            retVal = ddp_platform_set_if_vlan_name(ifs, data, dataLen);
            break;
        case DDP_FIELD_VLAN_ID:
            retVal = ddp_platform_set_if_vlan_id(ifs, data, dataLen);
            break;
        case DDP_FIELD_VLAN_TAG_PLIST_TYPE:
            retVal = ddp_platform_set_if_vlan_tag_list_type(ifs, data, dataLen);
            break;
        case DDP_FIELD_VLAN_TAG_PLIST:
            retVal = ddp_platform_set_if_vlan_tag_list(ifs, data, dataLen);
            break;
        case DDP_FIELD_VLAN_UNTAG_PLIST_TYPE:
            retVal = ddp_platform_set_if_vlan_untag_list_type(ifs, data, dataLen);
            break;
        case DDP_FIELD_VLAN_UNTAG_PLIST:
            retVal = ddp_platform_set_if_vlan_untag_list(ifs, data, dataLen);
            break;
        case DDP_FIELD_VLAN_LIST:
        case DDP_FIELD_VLAN_STATE:
            break;

        /* 0x00D3: set ASV */
        case DDP_FIELD_ASV_ID:
            retVal = ddp_platform_set_if_asv_id(ifs, data, dataLen);
            break;
        case DDP_FIELD_ASV_PRI:
            retVal = ddp_platform_set_if_asv_priority(ifs, data, dataLen);
            break;
        case DDP_FIELD_ASV_STATE:
            retVal = ddp_platform_set_if_asv_state(ifs, data, dataLen);
            break;
        case DDP_FIELD_ASV_TAG_PLIST_TYPE:
            retVal = ddp_platform_set_if_asv_tag_list_type(ifs, data, dataLen);
            break;
        case DDP_FIELD_ASV_TAG_PLIST:
            retVal = ddp_platform_set_if_asv_tag_list(ifs, data, dataLen);
            break;

        /* 0x00D4: set SNMP */
        case DDP_FIELD_SNMP_GLOBAL_STATE:
            retVal = ddp_platform_set_if_snmp_global_state(ifs, data, dataLen);
            break;
        case DDP_FIELD_SNMP_COMMUNITY_NAME_RO:
            retVal = ddp_platform_set_if_snmp_community_name_ro(ifs, data, dataLen);
            break;
        case DDP_FIELD_SNMP_COMMUNITY_NAME_RW:
            retVal = ddp_platform_set_if_snmp_community_name_rw(ifs, data, dataLen);
            break;
        case DDP_FIELD_SNMP_VIEW_NAME_RO:
            retVal = ddp_platform_set_if_snmp_view_name_ro(ifs, data, dataLen);
            break;
        case DDP_FIELD_SNMP_VIEW_NAME_RW:
            retVal = ddp_platform_set_if_snmp_view_name_rw(ifs, data, dataLen);
            break;

        /* 0x00D5: set user defined MAC information for ASV */
        case DDP_FIELD_ASV_USR_DEF_MAC:
            retVal = ddp_platform_set_if_asv_usr_def_mac(ifs, data, dataLen);
            break;

        case DDP_FIELD_DATE_TIME:
            retVal = ddp_platform_set_date_time(ifs, data, dataLen);
            break;
        case DDP_FIELD_TIMEZONE:
            retVal = ddp_platform_set_time_zone(ifs, data, dataLen);
            break;
        case DDP_FIELD_DAYLIGHT_SAVING:
            retVal = ddp_platform_set_daylight_saving(ifs, data, dataLen);
            break;
        case DDP_FIELD_NTP_ENABLE:
            retVal = ddp_platform_set_ntp_enable(ifs, data, dataLen);
            break;
        case DDP_FIELD_NTP_SERVER:
            retVal = ddp_platform_set_ntp_server(ifs, data, dataLen);
            break;
        /* DST */
        case DDP_FIELD_DST_SETTING:
            retVal = ddp_platform_set_dst(ifs, data, dataLen);
            break;
        case DDP_FIELD_NTP_SUPPORT:

        case DDP_FIELD_ALERT_REPORT:
        case DDP_FIELD_ALERT_REPORT_INFO:
        case DDP_FIELD_REBOOT_WAIT_TIME:
        case DDP_FIELD_FW_LOCAL_PATH:
        case DDP_FIELD_CFG_BACKUP_LOCAL_PATH:
        case DDP_FIELD_CFG_RESTORE_LOCAL_PATH:
        case DDP_FIELD_SPEC_VERSION:
        default:
            DDP_DEBUG("set field (id %d) is not implemented\n", field);
            break;
    }
    return retVal;
}

/* ddp_platform_get_user
 *   function to get user information of the specified user name from platform
 *
 *   ifs : interface
 *   user : struct to hold data written by platform
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_platform_get_user
(
    struct ddp_interface* ifs,
    struct ddp_user* user
)
{
    if (user == NULL) { return -1; }
    if (strlen(user->name) == 0) { return -2; }

    return 0;

//    FILE *pFile = NULL;
//    UINT1 fileBuf[DDP_MAX_LEN_USERNAME + 1];
//	memset(fileBuf, 0, DDP_MAX_LEN_USERNAME + 1);
//    pFile = fopen("userName.txt", "rb");
//    if(pFile) {
//    	fgets((char *)fileBuf, DDP_MAX_LEN_USERNAME + 1, pFile);
//		int len = strlen((char *)fileBuf);
//		if(fileBuf[len - 1] == '\n')
//			fileBuf[len - 1] = '\0';
//    	fclose(pFile);
//    }
//    else {
//    	return -2;
//    }
//
//    if (strlen((INT1*)fileBuf) == 0) {
//    	return -2;
//    }
//
//    if (strcmp((char *)fileBuf, (char *)user->name) == 0) {
//        struct spwd userSpwd;
//        struct spwd *result;
//        char *buf;
//        long bufsize;
//
//        bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
//        if (bufsize == -1)          /* Value was indeterminate */
//            bufsize = 16384;        /* Should be more than enough */
//
//        buf = (char *)malloc(bufsize);
//        if (buf == NULL) {
//            perror("malloc");
//            return -2;
//        }
//
//        int s = getspnam_r((char *)fileBuf, &userSpwd, buf, bufsize, &result);
//        free(buf);
//        if (result != NULL && s != -1) {
//        	strcpy(sysEncPass, userSpwd.sp_pwdp);
//        }
//    	else
//    		return -2;
//
//    	user->type = DDP_USER_TYPE_USER;
//    	return 0;
//    }
//    return -3;
}

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
INT4
ddp_platform_set_user
(
    struct ddp_interface* ifs,
    struct ddp_user* user
)
{
    if (user == NULL) { return -1; }
    if (strlen(user->name) == 0) { return -2; }

    /* todo: can call userctrl to add an user. */
    return -3;
}

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
INT4
ddp_platform_capable_process_report
(
    struct ddp_interface* ifs
)
{
    if (pf_capableProcessReport == -1) {
        DDP_DEBUG("%s (%d) : set capable process report default value\n", __FUNCTION__, __LINE__);
        pf_capableProcessReport = DDP_PROCESS_REPORT_DEFAULT;
    }
    return pf_capableProcessReport;
}

/* ddp_platform_op_support
 *   function to check whether the specified opcode is supported by the device model or not
 *   Different interfaces may support different opcode.
 *   The implementation here is just for demonstration only.
 *   Platform developer could set the different combination of opcodes for different model.
 *
 *   ifs: interface
 *   op: opcode
 *
 *   return: 0 -> not supported, 1 -> supported
 */
INT4
ddp_platform_op_support
(
    struct ddp_interface* ifs,
    UINT2 op
)
{
    /* all devices */
    if (/*op == DDP_OP_GENERAL_REPORT || */op == DDP_OP_DISCOVERY
     || op == DDP_OP_SET_BASIC_INFO || op == DDP_OP_USER_VERIFY
     || op == DDP_OP_CHANGE_ID_PSW  || op == DDP_OP_QUERY_SUPPORT_OPT
     /*|| op == DDP_OP_DEVICE_ALERT_INFO || op == DDP_OP_SNMP_CFG || op == DDP_OP_SNMP_GET
     || op == DDP_OP_FW_UPGRADE || op == DDP_OP_CFG_BACKUP
     || op == DDP_OP_CFG_RESTORE || op == DDP_OP_REBOOT
     || op == DDP_OP_RESET || op == DDP_OP_FACTORY_RESET
     || op == DDP_OP_GET_SYS_DATE_TIME || op == DDP_OP_SET_SYS_DATE_TIME
     || op == DDP_OP_DDP_INFO || op == DDP_OP_SET_IPV4_ADDR || op == DDP_OP_SET_IPV6_ADDR
     || op == DDP_OP_SET_DEVICE_INFO */
    ) {
        return 1;
    }
    /* switch */
    else if (op == DDP_OP_QUERY_NEIGHBOR_INFO || op == DDP_OP_VLAN_CFG || op == DDP_OP_VLAN_GET
          || op == DDP_OP_ASV_CFG || op == DDP_OP_ASV_GET || op == DDP_OP_ASV_USR_MAC_CFG
          || op == DDP_OP_GET_SYS_PORT_NUM || op == DDP_OP_ASV_USR_MAC_GET
    ) {
#ifdef FAKE_SWITCH
        return 1;
#else
        if (strcmp((INT1*)pf_prodCat, prodcat_str_switch) == 0) {
            return 1;
        } else {
            return 0;
        }
#endif /* FAKE_SWITCH */
    }
    /* access point */
    else if (op == DDP_OP_SET_MULTI_SSID || op == DDP_OP_GET_MULTI_SSID) {
        if (strcmp((INT1*)pf_prodCat, prodcat_str_ap) == 0) {
            return 1;
        } else {
            return 0;
        }
    }
    /* network camera */
    /*else if ( ) {
        if (strcmp((INT1*)pf_prodCat, (INT1*)"Network camera") == 0) {
            return 1;
        } else {
            return 0;
        }
    } */
    else {
        return 0;
    }
}

/* ddp_platform_init
 *   function to do platform init.
 *
 *   ddp_entrance will call this function.
 *   please keep this function here.
 *   developer can decide what it do
 */
INT4
ddp_platform_init
(
    void
)
{
    UINT1 u1Cnt = 0;
#ifdef FAKE_SWITCH
    printf("\nProduct category is %s as FAKE_SWITCH\n", vc_prodCat);
#else
    printf("\nProduct category is %s\n", vc_prodCat);
#endif
    /* FOR DEMO OF VIRTUAL CLIENT ONLY
     * copy default field values to buffer
     */
    memset(pf_brandName, 0, DDP_FIELD_LEN_BRAND_NAME);
    strncpy((INT1*)pf_brandName, (INT1*)vc_brandName, strlen((INT1*)vc_brandName));
    memset(pf_prodName, 0, DDP_FIELD_LEN_PRODUCT_NAME);
    strncpy((INT1*)pf_prodName, (INT1*)vc_prodName, strlen((INT1*)vc_prodName));
    memset(pf_modelName, 0, DDP_FIELD_LEN_MODEL_NAME);
    strncpy((INT1*)pf_modelName, (INT1*)vc_modelName, strlen((INT1*)vc_modelName));
    memset(pf_version, 0, DDP_FIELD_LEN_VERSION);
    strncpy((INT1*)pf_version, (INT1*)vc_version, strlen((INT1*)vc_version));
    //memset(pf_prodType, 0, DDP_FIELD_LEN_PRODUCT_TYPE);
    //memcpy((INT1*)pf_prodType, (INT1*)vc_prodType, sizeof(vc_prodType));
    pf_prodType = vc_prodType;
    //memset(pf_prodHwSpec, 0, DDP_FIELD_LEN_PRODUCT_HW_SPEC);
    //memcpy((INT1*)pf_prodHwSpec, (INT1*)vc_prodHwSpec, sizeof(vc_prodHwSpec));
    pf_prodHwSpec = vc_prodHwSpec;
    memset(pf_sysName, 0, DDP_FIELD_LEN_SYSTEM_NAME);
    strncpy((INT1*)pf_sysName, (INT1*)vc_sysName, strlen((INT1*)vc_sysName));
    //memset(pf_webPort, 0, DDP_FIELD_LEN_WEB_SERVICE_PORT);
    //memcpy((INT1*)pf_webPort, (INT1*)vc_webPort, sizeof(vc_webPort));
    pf_webPort = vc_webPort;
    pf_customizedDns = vc_customizedDns;
    memset(pf_subnetMask, 0, DDP_FIELD_LEN_SUBNET_MASK);
    memcpy((INT1*)pf_subnetMask, (INT1*)vc_subnetMask, sizeof(vc_subnetMask));
    memset(pf_defaultGateway, 0, DDP_FIELD_LEN_DEFAULT_GATEWAY);
    memcpy((INT1*)pf_defaultGateway, (INT1*)vc_defaultGateway, sizeof(vc_defaultGateway));
    memset(pf_primaryDns, 0, DDP_FIELD_LEN_PRIMARY_DNS);
    memcpy((INT1*)pf_primaryDns, (INT1*)vc_primaryDns, sizeof(vc_primaryDns));
    memset(pf_secondDns, 0, DDP_FIELD_LEN_SECONDARY_DNS);
    memcpy((INT1*)pf_secondDns, (INT1*)vc_secondDns, sizeof(vc_secondDns));
    memset(pf_ip, 0, DDP_FIELD_LEN_DEVICE_IP_ADDR);
    memcpy((INT1*)pf_ip, (INT1*)vc_ip, DDP_FIELD_LEN_DEVICE_IP_ADDR);
    pf_dhcp = vc_dhcp;
    memset(pf_prodCat, 0, DDP_FIELD_LEN_PRODUCT_CATEGORY);
    strncpy((INT1*)pf_prodCat, (INT1*)vc_prodCat, strlen((INT1*)vc_prodCat));
    memset(pf_hwVersion, 0, DDP_FIELD_LEN_HARDWARE_VERSION);
    strncpy((INT1*)pf_hwVersion, (INT1*)vc_hwVersion, strlen((INT1*)vc_hwVersion));
    memset(pf_serialNumber, 0, DDP_FIELD_LEN_SERIAL_NUMBER);
    strncpy((INT1*)pf_serialNumber, (INT1*)vc_serialNumber, strlen((INT1*)vc_serialNumber));
    memset(pf_interfaceNumber, 0, DDP_FIELD_LEN_INTERFACE_NUMBER);
    memcpy((INT1*)pf_interfaceNumber, (INT1*)vc_interfaceNumber, sizeof(vc_interfaceNumber));
    memset(pf_users, 0, sizeof(pf_users));
    memcpy((INT1*)pf_users, (INT1*)vc_users, sizeof(vc_users));
    pf_rebootWaitTime = vc_rebootWaitTime;
    pf_snmpGlobalState = vc_snmpGlobalState;
    /* clean snmp database */
    for (u1Cnt = 0; u1Cnt < DDP_PLATFORM_SNMP_NAMES_MAX; u1Cnt++) {
        memset(pf_snmpCommunityNameRO[u1Cnt], 0, DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO);
        memset(pf_snmpCommunityNameRW[u1Cnt], 0, DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW);
        memset(pf_snmpViewNameRO[u1Cnt], 0, DDP_FIELD_LEN_SNMP_VIEW_NAME_RO);
        memset(pf_snmpViewNameRW[u1Cnt], 0, DDP_FIELD_LEN_SNMP_VIEW_NAME_RW);
    }
    /* init snmp database */
    for (u1Cnt = 0; u1Cnt < 5; u1Cnt++) {
        sprintf((INT1*)pf_snmpCommunityNameRO[u1Cnt], "COMMUNITY RO %d", u1Cnt);
        sprintf((INT1*)pf_snmpCommunityNameRW[u1Cnt], "COMMUNITY RW %d", u1Cnt);
        sprintf((INT1*)pf_snmpViewNameRO[u1Cnt], "VIEW RO %d", u1Cnt);
        sprintf((INT1*)pf_snmpViewNameRW[u1Cnt], "VIEW RW %d", u1Cnt);
    }
    memset(pf_fwLocalPath, 0, sizeof(pf_fwLocalPath));
    strncpy((INT1*)pf_fwLocalPath, (INT1*)vc_fwLocalPath, sizeof(pf_fwLocalPath));
    memset(pf_cfgBackupLocalPath, 0, sizeof(pf_cfgBackupLocalPath));
    strncpy((INT1*)pf_cfgBackupLocalPath, (INT1*)vc_cfgBackupLocalPath, sizeof(pf_cfgBackupLocalPath));
    memset(pf_cfgRestoreLocalPath, 0, sizeof(pf_cfgRestoreLocalPath));
    strncpy((INT1*)pf_cfgRestoreLocalPath, (INT1*)vc_cfgRestoreLocalPath, sizeof(pf_cfgRestoreLocalPath));
    pf_ntpEnable = vc_ntpEnable;
    strncpy((INT1*)pf_ntpServer, (INT1*)vc_ntpServer, DDP_FIELD_LEN_NTP_SERVER);
    pf_ntpFormat = vc_ntpFormat;
    /* DST */
    pf_dst_enable = vc_dst_enable; pf_dst_offset = vc_dst_offset;
    pf_dst_start_year = vc_dst_start_year; pf_dst_start_month = vc_dst_start_month;
    pf_dst_start_day = vc_dst_start_day; pf_dst_start_hour = vc_dst_start_hour;
    pf_dst_start_minute = vc_dst_start_minute; pf_dst_end_year = vc_dst_end_year;
    pf_dst_end_month = vc_dst_end_month; pf_dst_end_day = vc_dst_end_day;
    pf_dst_end_hour = vc_dst_end_hour; pf_dst_end_minute = vc_dst_end_minute;
    memset((INT1*)ssid_table_24g, 0, sizeof(struct ssid_entry) * SSID_NUMBER);
    memset((INT1*)ssid_table_5g, 0, sizeof(struct ssid_entry) * SSID_NUMBER);
    for (u1Cnt = 0; u1Cnt < SSID_NUMBER; u1Cnt++) {
        sprintf(ssid_table_24g[u1Cnt].name, "SSID-24g-%d", u1Cnt);
        ssid_table_24g[u1Cnt].vid = u1Cnt;
        ssid_table_24g[u1Cnt].security = 1;
        sprintf(ssid_table_24g[u1Cnt].password, "SSID-24g-p%d", u1Cnt);
        sprintf(ssid_table_5g[u1Cnt].name, "SSID-5g-%d", u1Cnt);
        ssid_table_5g[u1Cnt].vid = u1Cnt + 10;
        ssid_table_5g[u1Cnt].security = 2;
        sprintf(ssid_table_5g[u1Cnt].password, "SSID-5g-p%d", u1Cnt);
    }
    /* DDP information */
    memset(pf_specVer, 0, DDP_FIELD_LEN_SPEC_VERSION);
    strcpy((INT1*)pf_specVer, (INT1*)vc_specVer);

    /* 0x00D2 VLAN */
    memset((INT1 *)&gstDDP_PF_VLAN, 0, sizeof(struct stDDP_PF_VLAN));

    memcpy(gstDDP_PF_VLAN.ai1vlan_name, (INT1*)ksDDP_PF_DEFAULT_VLAN_NAME, DDP_FIELD_LEN_VLAN_NAME);
    gstDDP_PF_VLAN.u4vlan_id = kDDP_PF_DEFAULT_VLAN_ID;
    gstDDP_PF_VLAN.u1vlan_tag_list_type = kDDP_PF_DEFAULT_VLAN_TAG_LIST_TYPE;
    gstDDP_PF_VLAN.u1vlan_untag_list_type = kDDP_PF_DEFAULT_VLAN_UNTAG_LIST_TYPE;
    /* global vlan database */
    memset((INT1*)pf_vlan_arr, 0, sizeof(struct stDDP_PF_VLAN) * 4096);
    memcpy((void*)&pf_vlan_arr[kDDP_PF_DEFAULT_VLAN_ID - DDP_VLANID_MIN], (void*)&gstDDP_PF_VLAN, sizeof(struct stDDP_PF_VLAN));
    /* 0x00D3 ASV */
    memset((INT1 *)&gstDDP_PF_ASV, 0, sizeof(struct stDDP_PF_ASV));

    gstDDP_PF_ASV.u4asv_id = kDDP_PF_DEFAULT_ASV_ID;
    gstDDP_PF_ASV.u4asv_priority = kDDP_PF_DEFAULT_ASV_PRIORITY;
    gstDDP_PF_ASV.u1asv_state = kDDP_PF_DEFAULT_ASV_STATE;
    gstDDP_PF_ASV.u1asv_tag_list_type = kDDP_PF_DEFAULT_ASV_TAG_LIST_TYPE;

    /* 0x00D5 User defined MAC for ASV */
    memset((INT1 *)&gastDDP_ASV_MAC_List, 0, sizeof(gastDDP_ASV_MAC_List));

    /* Initialze the 1-based ASV MAC array. */
    for(u1Cnt = 1; u1Cnt <= kDDP_SWITCH_MAX_ENTRY_PER_PACKET; u1Cnt++)
    {
        gastDDP_ASV_MAC_List[u1Cnt].u1Mac_type = kDDP_PF_DEFAULT_ASV_USR_MAC_SRV_TYPE;
        gastDDP_ASV_MAC_List[u1Cnt].u1Mac_Category = kDDP_PF_DEFAULT_ASV_USR_MAC_ADDR_CATG;
        memcpy(gastDDP_ASV_MAC_List[u1Cnt].ai1Mac_addr, gaiDDP_PF_DEFAULT_ASV_USR_MAC_ADDR, DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR);
        memcpy(gastDDP_ASV_MAC_List[u1Cnt].ai1Mac_addr_desc, (INT1*)ksDDP_PF_DEFAULT_ASV_USR_MAC_ADDR_DESC, DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC);
    }

    /* In switch debug mode, let every device handle report. */
#ifdef FAKE_SWITCH
    pf_capableProcessReport = DDP_PROCESS_REPORT_YES;
    g_debugFlag |= DDP_DEBUG_PRINT_SWITCH;
    if (i4fnDDP_proto_switch_Init() != DDP_RET_SWITCH_OK) {
        DDP_DEBUG("\n%s: line<%d> Switch_Init error.\n", __FUNCTION__, __LINE__);
        return DDP_RET_SWITCH_ERROR_INIT;
    }

#else
    /* only Switch would process incoming general report message */
    if (strcmp((INT1*)pf_prodCat, (INT1*)"Switch") == 0) {
        pf_capableProcessReport = DDP_PROCESS_REPORT_YES;
        g_debugFlag |= DDP_DEBUG_PRINT_SWITCH;
        /* Initialize switch function. */
        if ( i4fnDDP_proto_switch_Init() != DDP_RET_SWITCH_OK){
            DDP_DEBUG("\n%s: line<%d> Switch Init error.\n", __FUNCTION__, __LINE__);
            return DDP_RET_SWITCH_ERROR_INIT;
        }
    }
    else {
        pf_capableProcessReport = DDP_PROCESS_REPORT_NO;
        DDP_DEBUG("\n%s: line<%d> Non-Switch device.\n", __FUNCTION__, __LINE__);
    }
#endif
    return 0;
}
/* ddp_platform_free
 *   function to do platform cleanup
 *
 *   it is called when program is going to be terminated.
 *   please keep this function here.
 *   developer can decide what it do.
 */
INT4
ddp_platform_free
(
    void
)
{
    return 0;
}
