
#ifndef _DDP_GENERAL_H_
#define _DDP_GENERAL_H_

#include <netinet/in.h>


/* data type */
#define INT1  char
#define INT2  short
#define INT4  int

#define UINT1 unsigned char
#define UINT2 unsigned short
#define UINT4 unsigned int

/* Data in packet are stored in little-endian.
 * The following macro are used for transformation.
 */
#define DDP_NTOHS(x) g_iMachEndian == 0 ? (UINT2)(((x & 0xFF00) >> 8) | \
                                                  ((x & 0x00FF) << 8))  \
                                                  : (UINT2)(x)
#define DDP_NTOHL(x) g_iMachEndian == 0 ? (UINT4)(((x & 0xFF000000) >> 24) | \
                                                  ((x & 0x00FF0000) >> 8)  | \
                                                  ((x & 0x0000FF00) << 8 ) | \
                                                  ((x & 0x000000FF) << 24))  \
                                                  : (UINT4)(x)
#define DDP_HTONL(x) (UINT4)(DDP_NTOHL(x))
#define DDP_HTONS(x) (UINT2)(DDP_NTOHS(x))

/* value of ip version */
#define IPV4_FLAG  4
#define IPV6_FLAG  6

/* length of addresses */
#define MAC_ADDRLEN  6
#define IPV4_ADDRLEN 4
#define IPV6_ADDRLEN 16

#define DDP_IPV6_MULTI_ADDR "FF02::1"

/* string table of product category */
#define DDP_PRODCAT_SWITCH          "Switch"
#define DDP_PRODCAT_AP              "Access point"
#define DDP_PRODCAT_NETCAM          "Network camera"
#define DDP_PRODCAT_VIDEOENCORDER   "Video encoder"
#define DDP_PRODCAT_NVR             "Network video recoder"
#define DDP_PRODCAT_NAS             "NAS"
#define DDP_PRODCAT_SERVROUTE       "Service router"
#define DDP_PRODCAT_WLC             "Wireless controller"
#define DDP_PRODCAT_WLS             "Wireless switch"
#define DDP_PRODCAT_WLR             "Wireless router"
#define DDP_PRODCAT_EPOS            "EPOS"
#define DDP_PRODCAT_AAA             "AAA policy server"

/* timezone table */
enum {
    DDP_TZ_GMT_MINUS_1200 = 1,
    DDP_TZ_GMT_MINUS_1100,
    DDP_TZ_GMT_MINUS_1000,
    DDP_TZ_GMT_MINUS_0900,
    DDP_TZ_GMT_MINUS_0800_PACIFIC,

    DDP_TZ_GMT_MINUS_0800_BAJACA,
    DDP_TZ_GMT_MINUS_0700_CHIHUAHUA,
    DDP_TZ_GMT_MINUS_0700_MTN,
    DDP_TZ_GMT_MINUS_0700_AZ,
    DDP_TZ_GMT_MINUS_0600_CENAM,

    DDP_TZ_GMT_MINUS_0600_MX,
    DDP_TZ_GMT_MINUS_0600_SASK,
    DDP_TZ_GMT_MINUS_0600_CENTRAL,
    DDP_TZ_GMT_MINUS_0500_BOGOTA,
    DDP_TZ_GMT_MINUS_0500_EASTERN,

    DDP_TZ_GMT_MINUS_0500_INDIANA,
    DDP_TZ_GMT_MINUS_0400_CARACAS,
    DDP_TZ_GMT_MINUS_0400_ATLANTIC,
    DDP_TZ_GMT_MINUS_0400_SANTIAGO,
    DDP_TZ_GMT_MINUS_0400_GEORGETOWN,

    DDP_TZ_GMT_MINUS_0330,
    DDP_TZ_GMT_MINUS_0300_BUENOS,
    DDP_TZ_GMT_MINUS_0300_BR,
    DDP_TZ_GMT_MINUS_0300_GREENLAND,
    DDP_TZ_GMT_MINUS_0300_MONTEVIDEO,

    DDP_TZ_GMT_MINUS_0200,
    DDP_TZ_GMT_MINUS_0100_AZORES,
    DDP_TZ_GMT_MINUS_0100_CAPEVERDE,
    DDP_TZ_GMT_LONDON,
    DDP_TZ_GMT_MONROVIA,

    DDP_TZ_GMT_PLUS_0100_PRAGUE,
    DDP_TZ_GMT_PLUS_0100_CENAF,
    DDP_TZ_GMT_PLUS_0100_WARSAW,
    DDP_TZ_GMT_PLUS_0100_PARIS,
    DDP_TZ_GMT_PLUS_0100_BERLIN,

    DDP_TZ_GMT_PLUS_0200_HELSINKI,
    DDP_TZ_GMT_PLUS_0200_ATHENS,
    DDP_TZ_GMT_PLUS_0200_BEIRUT,
    DDP_TZ_GMT_PLUS_0200_HARARE,
    DDP_TZ_GMT_PLUS_0200_CAIRO,

    DDP_TZ_GMT_PLUS_0200_MINSK,
    DDP_TZ_GMT_PLUS_0200_AMMAN,
    DDP_TZ_GMT_PLUS_0200_WINDHOEK,
    DDP_TZ_GMT_PLUS_0200_JERUSALEM,
    DDP_TZ_GMT_PLUS_0300_BAGHDAD,

    DDP_TZ_GMT_PLUS_0300_MOSCOW,
    DDP_TZ_GMT_PLUS_0300_TBILISI,
    DDP_TZ_GMT_PLUS_0300_NAIROBI,
    DDP_TZ_GMT_PLUS_0300_KUWAIT,
    DDP_TZ_GMT_PLUS_0330,

    DDP_TZ_GMT_PLUS_0400_BAKU,
    DDP_TZ_GMT_PLUS_0400_ABUDHABI,
    DDP_TZ_GMT_PLUS_0400_YEREVAN,
    DDP_TZ_GMT_PLUS_0430,
    DDP_TZ_GMT_PLUS_0500_EKATERINBURG,

    DDP_TZ_GMT_PLUS_0500_ISLAMABAD,
    DDP_TZ_GMT_PLUS_0530_NEWDELHI,
    DDP_TZ_GMT_PLUS_0530_SRIJAYAWARDENEPURA,
    DDP_TZ_GMT_PLUS_0545,
    DDP_TZ_GMT_PLUS_0600_DHAKA,

    DDP_TZ_GMT_PLUS_0600_ALMATY,
    DDP_TZ_GMT_PLUS_0630,
    DDP_TZ_GMT_PLUS_0700_KRASNOYARSK,
    DDP_TZ_GMT_PLUS_0700_BANGKOK,
    DDP_TZ_GMT_PLUS_0800_HK,

    DDP_TZ_GMT_PLUS_0800_TPE,
    DDP_TZ_GMT_PLUS_0800_IRKUTSK,
    DDP_TZ_GMT_PLUS_0800_PERTH,
    DDP_TZ_GMT_PLUS_0800_SGP,
    DDP_TZ_GMT_PLUS_0900_YAKUTSK,

    DDP_TZ_GMT_PLUS_0900_TOKYO,
    DDP_TZ_GMT_PLUS_0900_SEOUL,
    DDP_TZ_GMT_PLUS_0930_ADELAIDE,
    DDP_TZ_GMT_PLUS_0930_DARWIN,
    DDP_TZ_GMT_PLUS_1000_HOBART,

    DDP_TZ_GMT_PLUS_1000_BRISBANE,
    DDP_TZ_GMT_PLUS_1000_VLADIVOSTOK,
    DDP_TZ_GMT_PLUS_1000_SYDNEY,
    DDP_TZ_GMT_PLUS_1000_GUAM,
    DDP_TZ_GMT_PLUS_1100,

    DDP_TZ_GMT_PLUS_1200_FIJI,
    DDP_TZ_GMT_PLUS_1200_AUCKLAND,
    DDP_TZ_GMT_PLUS_1300,
};

/* values of ddp role */
enum {
    DDP_ROLE_NONE   	= 0		,
    DDP_ROLE_CLIENT 	= 1		,
    DDP_ROLE_SERVER 	= 1 << 1,
	DDP_ROLE_SERVER_V1 	= 1 << 2
};

/* values of header identifier (2 bytes) */
enum {
    IPV4_REQ    = 0xFDFD,
    IPV4_REPLY  = 0xFEEE,
    IPV4_REPORT = 0xFFFF,
    IPV4_RELAY  = 0xFDDD,

    IPV6_REQ    = 0x0DFD,
    IPV6_REPLY  = 0x0EEE,
    IPV6_REPORT = 0x0FFF,
    IPV6_RELAY  = 0x0DDD,
};

/* values request mode (2 bytes) */
enum {
    REQ_MODE_ALL        = 0xFFFF,
    REQ_MODE_UNICAST    = 0x7777
};

/* range of seq number (2 bytes) */
#define MAX_SEQ 0xFFFF
#define MIN_SEQ 0x0

/* offset of header field */
enum {
    HDR_IDENT_OFFSET      = 0,
    HDR_SEQ_OFFSET        = 2,
    HDR_OPCODE_OFFSET     = 4,
    HDR_MAC_OFFSET        = 6,
    HDR_IP_OFFSET         = 12,
    HDR_RETCODE_V4_OFFSET = 16,
    HDR_RETCODE_V6_OFFSET = 28,
    HDR_PVER_V4_OFFSET    = 18,
    HDR_PVER_V6_OFFSET    = 30,
    HDR_BDLEN_V4_OFFSET   = 20,
    HDR_BDLEN_V6_OFFSET   = 32,
    HDR_END_V4_OFFSET     = 22,
    HDR_END_V6_OFFSET     = 34,
};

/* op code */
enum {
    DDP_OP_GENERAL_REPORT       = 0x00A0, /* 160 */
    DDP_OP_DISCOVERY            = 0x00A1, /* 161 */
    DDP_OP_SET_BASIC_INFO       = 0x00A2, /* 162 */
    DDP_OP_USER_VERIFY          = 0x00A3, /* 163 */
    DDP_OP_CHANGE_ID_PSW        = 0x00A4, /* 164 */
    DDP_OP_QUERY_SUPPORT_OPT    = 0x00A5, /* 165 */
    DDP_OP_DEVICE_ALERT_INFO    = 0x0200, /* 512 */
    DDP_OP_SNMP_CFG             = 0x00D4, /* 212 */
    DDP_OP_SNMP_GET             = 0x00DA, /* 218 */
    DDP_OP_FW_UPGRADE           = 0x0100, /* 256 */
    DDP_OP_CFG_BACKUP           = 0x0101, /* 257 */
    DDP_OP_CFG_RESTORE          = 0x0102, /* 258 */
    DDP_OP_REBOOT               = 0x0103, /* 259 */
    DDP_OP_GET_SYS_DATE_TIME    = 0x00AC, /* 172 */
    DDP_OP_SET_SYS_DATE_TIME    = 0x00AD, /* 173 */

    DDP_OP_QUERY_NEIGHBOR_INFO  = 0x00D0, /* 208 */
    DDP_OP_SET_MULTI_SSID       = 0x00D1, /* 209 */
    DDP_OP_VLAN_CFG             = 0x00D2, /* 210 */
    DDP_OP_ASV_CFG              = 0x00D3, /* 211 */
    DDP_OP_ASV_USR_MAC_CFG      = 0x00D5, /* 213 */
    DDP_OP_ASV_USR_MAC_GET      = 0x00D6, /* 214 */
    DDP_OP_GET_SYS_PORT_NUM     = 0x00D7, /* 215 */
    DDP_OP_GET_MULTI_SSID       = 0x00D8, /* 216 */
    DDP_OP_ASV_GET              = 0x00DB, /* 219 */
    DDP_OP_VLAN_GET             = 0x00DC, /* 220 */
    
    DDP_OP_RESET                = 0x00AA, /* 170 */
    DDP_OP_FACTORY_RESET        = 0x00AB, /* 171 */
    DDP_OP_DDP_INFO             = 0x00E0, /* 224 */
    DDP_OP_SET_IPV4_ADDR        = 0x00E1, /* 225 */
    DDP_OP_SET_IPV6_ADDR        = 0x00E2, /* 226 */
    DDP_OP_SET_DEVICE_INFO      = 0x00E3, /* 227 */
};

/* ret code */
enum {
    DDP_RETCODE_OK              = 0,
    DDP_RETCODE_FAIL            = 1,
    DDP_RETCODE_AUTHEN_FAIL     = 2,
    DDP_RETCODE_CLI_NOT_SUPPORT = 3,
};

/* protocol version of DDP */
enum {
    DDP_PROTO_V1 = 1,
    DDP_PROTO_V2 = 2,
};

/* body length of request message */
enum {
    DDP_REQ_LEN_GENERAL_REPORT      = 0,
    DDP_REQ_LEN_DISCOVERY           = 0,
/*#define DDP_SET_BASIC_INFO_REQ_LEN_v1   217*/
    DDP_REQ_LEN_SET_BASIC_INFO      = 283, /* 155 + 128 */
    DDP_REQ_LEN_USER_VERIFY         = 128,
    DDP_REQ_LEN_CHANGE_ID_PSW       = 256, /* 128 + 128 */
    DDP_REQ_LEN_QUERY_SUPPORT_OPT   = 128, /* version 1 only */
    DDP_REQ_LEN_RESET               = 132, /* 128 + 4 */ 
    DDP_REQ_LEN_FACTORY_RESET       = 132, /* 128 + 4 */
    DDP_REQ_LEN_QUERY_NEIGHBOR_INFO = 0,
    DDP_REQ_LEN_VLAN_CFG            = 294, /* 128 + 166 */
    DDP_REQ_LEN_VLAN_GET            = 5,
    DDP_REQ_LEN_ASV_CFG             = 202, /* 128 + 74 */
    DDP_REQ_LEN_ASV_GET             = 0,
    DDP_REQ_LEN_ASV_USR_DEF_MAC_CFG = 157, /* 128 + 29 */
    DDP_REQ_LEN_ASV_USR_DEF_MAC_GET = 0,
    DDP_REQ_LEN_GET_SYS_PORT_NUM    = 0,
    DDP_REQ_LEN_SNMP_CFG            = 257, /* 128 + 129 */
    DDP_REQ_LEN_SNMP_GET            = 0,
    DDP_REQ_LEN_FW_UPGRADE_IPV4     = 263, /* 128 + 135 */
    DDP_REQ_LEN_FW_UPGRADE_IPV6     = 275, /* 128 + 147 */
    DDP_REQ_LEN_CFG_BACKUP_IPV4     = 263, /* 128 + 135 */
    DDP_REQ_LEN_CFG_BACKUP_IPV6     = 275, /* 128 + 147 */
    DDP_REQ_LEN_CFG_RESTORE_IPV4    = 263, /* 128 + 135 */
    DDP_REQ_LEN_CFG_RESTORE_IPV6    = 275, /* 128 + 147 */
    DDP_REQ_LEN_REBOOT              = 132, /* 128 + 4 */
    DDP_REQ_LEN_DEVICE_ALERT_INFO   = 0,
    DDP_REQ_LEN_GET_SYS_DATE_TIME   = 128,
    DDP_REQ_LEN_SET_SYS_DATE_TIME   = 228, /* 128 + 100 */
    DDP_REQ_LEN_SET_MULTI_SSID      = 577, /* 128 + 449 */
    DDP_REQ_LEN_GET_MULTI_SSID      = 1,
    DDP_REQ_LEN_DDP_INFO            = 0,
    DDP_REQ_LEN_SET_IPV4_ADDR       = 142, /* 128 + 14, the maximal size */
    DDP_REQ_LEN_SET_IPV6_ADDR       = 163, /* 128 + 35, the maximal size */
    DDP_REQ_LEN_SET_DEVICE_INFO     = 65,  /* 128 + 65, the maximal size */
};

/* body length of reply message */
enum {
    DDP_REPLY_LEN_GENERAL_REPORT       = 295,
	DDP_DISCOVERY_REPLY_LEN_v1         = 312,
    DDP_REPLY_LEN_DISCOVERY            = 483,
    DDP_REPLY_LEN_SET_BASIC_INFO       = 16,
    DDP_REPLY_LEN_USER_VERIFY          = 4,
    DDP_REPLY_LEN_CHANGE_ID_PSW        = 16,
    //DDP_QUERY_SUPPORT_OPT_REPLY_LEN     = (DDP_END + 1)*sizeof(UINT2) /*Depend on supported function no.#*/
    DDP_REPLY_LEN_RESET                = 16,
    DDP_REPLY_LEN_FACTORY_RESET        = 16,
/*    DDP_REPLY_LEN_QUERY_NEIGHBOR_INFO  = 231, */ /* Defined for variable length */
    DDP_REPLY_LEN_VLAN_CFG             = 16,
    DDP_REPLY_LEN_VLAN_GET             = 513, /* depend on Type. reply len could be 513 or 168 */
    DDP_REPLY_LEN_ASV_CFG              = 16,
    DDP_REPLY_LEN_ASV_GET              = 75, /* depend on result. reply len could be 1 or 75 */
    DDP_REPLY_LEN_ASV_USR_DEF_MAC_CFG  = 16,
    DDP_REPLY_LEN_SNMP_CFG             = 16,
    DDP_REPLY_LEN_SNMP_GET             = 1061, /* 1 + (number of snmp)*33 + 4, max number is 32 per packet */
    DDP_REPLY_LEN_FW_UPGRADE           = 130,
    DDP_REPLY_LEN_CFG_BACKUP           = 130,
    DDP_REPLY_LEN_CFG_RESTORE          = 130,
    DDP_REPLY_LEN_REBOOT               = 16,
    DDP_REPLY_LEN_DEVICE_ALERT_INFO    = 128,
    DDP_REPLY_LEN_GET_SYS_DATE_TIME    = 100,
    DDP_REPLY_LEN_SET_SYS_DATE_TIME    = 16,
    DDP_REPLY_LEN_SET_MULTI_SSID       = 16,
    DDP_REPLY_LEN_GET_MULTI_SSID       = 451, /* 452 bytes is the max size. */
    DDP_REPLY_LEN_DDP_INFO             = 16,
    DDP_REPLY_LEN_SET_IPV4_ADDR        = 129,
    DDP_REPLY_LEN_SET_IPV6_ADDR        = 129,
    DDP_REPLY_LEN_SET_DEVICE_INFO      = 129,
};

/* (231 - 1)*5 */
#define DDP_REPLY_LEN_QUERY_NEIGHBOR_INFO ( (sizeof(struct stDDP_neighbor_entry) - sizeof(UINT1)) * kDDP_SWITCH_MAX_ENTRY_PER_PACKET )

/* (29 - 1)*5 */
#define DDP_REPLY_LEN_ASV_USR_DEF_MAC_GET ( (sizeof(stDDP_ASV_USR_MAC_t) - sizeof(UINT1)) * kDDP_SWITCH_MAX_ENTRY_PER_PACKET )

/* (3 - 1)*5 */
#define DDP_REPLY_LEN_PORT_NUMBER_GET ( (sizeof(stDDP_ASV_USR_MAC_t) - sizeof(UINT1)) * kDDP_SWITCH_MAX_UNIT_PER_PACKET )

/* id of frequency band */
enum {
    DDP_WL_BAND_24GHZ = 0x00,
    DDP_WL_BAND_5GHZ  = 0x01,
    DDP_WL_BAND_ALL   = 0xFF,
};

/* field id */
enum {
    DDP_FIELD_BRAND_NAME = 0,
    DDP_FIELD_PRODUCT_NAME,
    DDP_FIELD_MODEL_NAME,
    DDP_FIELD_VERSION,
    DDP_FIELD_PRODUCT_TYPE,
    DDP_FIELD_PRODUCT_HW_SPEC,
    DDP_FIELD_MAC_ADDR,
    DDP_FIELD_SYSTEM_NAME,
    DDP_FIELD_DEVICE_IP_ADDR,
    DDP_FIELD_WEB_SERVICE_PORT,
    DDP_FIELD_RESERVED_1,
    DDP_FIELD_CUSTOMIZED_DNS,
    DDP_FIELD_SUBNET_MASK,
    DDP_FIELD_DEFAULT_GATEWAY,
    DDP_FIELD_PRIMARY_DNS,
    DDP_FIELD_SECONDARY_DNS,
    DDP_FIELD_DHCP,
    DDP_FIELD_DATE_TIME,
    DDP_FIELD_TIMEZONE,
    DDP_FIELD_DAYLIGHT_SAVING,
    DDP_FIELD_PRODUCT_CATEGORY,
    DDP_FIELD_HARDWARE_VERSION,
    DDP_FIELD_SERIAL_NUMBER,
    DDP_FIELD_INTERFACE_NUMBER,
    DDP_FIELD_DEVICE_IPV6_ADDR,
    DDP_FIELD_DEVICE_IPV6_PREFIX,
    DDP_FIELD_DEFAULT_GATEWAY_IPV6,
    DDP_FIELD_PRIMARY_DNS_IPV6,
    DDP_FIELD_SECONDARY_DNS_IPV6,
    DDP_FIELD_DHCP_IPV6,
    DDP_FIELD_ALERT_REPORT,
    DDP_FIELD_NTP_ENABLE,
    DDP_FIELD_NTP_SERVER,
    DDP_FIELD_NTP_FORMAT,
    DDP_FIELD_NTP_SUPPORT,

    /* Daylight Saving Time */
    DDP_FIELD_DST_SETTING,

    /* 0x00D2 */
    DDP_FIELD_VLAN_NAME,
    DDP_FIELD_VLAN_ID,
    DDP_FIELD_VLAN_TAG_PLIST_TYPE,
    DDP_FIELD_VLAN_TAG_PLIST,
    DDP_FIELD_VLAN_UNTAG_PLIST_TYPE,
    DDP_FIELD_VLAN_UNTAG_PLIST,
    DDP_FIELD_VLAN_TYPE,
    DDP_FIELD_VLAN_STATE,
    DDP_FIELD_VLAN_LIST,

    /* 0x00D3 */
    DDP_FIELD_ASV_ID,
    DDP_FIELD_ASV_PRI,
    DDP_FIELD_ASV_STATE,
    DDP_FIELD_ASV_TAG_PLIST_TYPE,
    DDP_FIELD_ASV_TAG_PLIST,
    DDP_FIELD_ASV_DEFINED,

    DDP_FIELD_SNMP_GLOBAL_STATE,
    DDP_FIELD_SNMP_COMMUNITY_NAME_RO,
    DDP_FIELD_SNMP_COMMUNITY_NAME_RW,
    DDP_FIELD_SNMP_VIEW_NAME_RO,
    DDP_FIELD_SNMP_VIEW_NAME_RW,

    /* 0x00D5, 0x00D6 */
    DDP_FIELD_ASV_USR_DEF_MAC,

    /* 0x00D7 */
    DDP_FIELD_PORT_NUM_GET,

    DDP_FIELD_ALERT_REPORT_INFO,
    DDP_FIELD_REBOOT,
    DDP_FIELD_REBOOT_WAIT_TIME,
    DDP_FIELD_RESET,
    DDP_FIELD_FACTORY_RESET,

    DDP_FIELD_SUB_SERVICE_TYPE,
    DDP_FIELD_SERVER_TYPE,
    DDP_FIELD_SERVER_IP_ADDR_TYPE,
    DDP_FIELD_SERVER_IP_ADDR,
    DDP_FIELD_FW_NAME_URL,
    DDP_FIELD_FW_LOCAL_PATH,
    DDP_FIELD_CFG_BACKUP_LOCAL_PATH,
    DDP_FIELD_CFG_RESTORE_LOCAL_PATH,
    /* DDP information */
    DDP_FIELD_SPEC_VERSION,

    DDP_FIELD_DELAY_TIME,
};

/* length of fields (in bytes) */
enum {
    DDP_FIELD_LEN_BRAND_NAME              = 64, /* brand name (string) */
    DDP_FIELD_LEN_PRODUCT_NAME            = 64, /* product name (string) */
    DDP_FIELD_LEN_MODEL_NAME              = 32, /* model number (string) */
    DDP_FIELD_LEN_VERSION                 = 32, /* version of firmware (string) */
    DDP_FIELD_LEN_PRODUCT_TYPE            = 2,  /* product type */
    DDP_FIELD_LEN_PRODUCT_HW_SPEC         = 4,  /* hw spec of the product */
    DDP_FIELD_LEN_MAC_ADDR                = 6,  /* mac address */
    DDP_FIELD_LEN_SYSTEM_NAME             = 64, /* system name (string) */
    DDP_FIELD_LEN_DEVICE_IP_ADDR          = 4,  /* ipv4 address */
    DDP_FIELD_LEN_WEB_SERVICE_PORT        = 2,  /* port of web service */
    DDP_FIELD_LEN_RESERVED_1              = 1,  /* reserved */
    DDP_FIELD_LEN_CUSTOMIZED_DNS          = 1,  /* 0: DNS is given, 1: DNS is specified by user.
                                                   This value is valid in case DHCP is enabled. */
    DDP_FIELD_LEN_SUBNET_MASK             = 4,  /* subnet mask */
    DDP_FIELD_LEN_DEFAULT_GATEWAY         = 4,  /* default gateway */
    DDP_FIELD_LEN_PRIMARY_DNS             = 4,  /* primary DNS */
    DDP_FIELD_LEN_SECONDARY_DNS           = 4,  /* secondary DNS */
    DDP_FIELD_LEN_DHCP                    = 1,  /* device enabling DHCP (0: disable, 1: enable) */
    DDP_FIELD_LEN_DATE_TIME               = 16, /* format: YYYYMMDDHHmmss (string) */
    /* DST */
    DDP_FIELD_LEN_DST_YEAR                = 2,
    DDP_FIELD_LEN_DST_MONTH               = 1,
    DDP_FIELD_LEN_DST_DAY                 = 1,
    DDP_FIELD_LEN_DST_HOUR                = 1,
    DDP_FIELD_LEN_DST_MINUTE              = 1,
    DDP_FIELD_LEN_DST_OFFSET              = 1,
    DDP_FIELD_LEN_TIMEZONE                = 2,  /* if high byte is 0, low byte is time-zone ID (refer to time-zone table.
                                                   if low byte is 0, high byte is the differential hour woth GMT (+1: 0x1, -1: 0xFF) */
    DDP_FIELD_LEN_DAYLIGHT_SAVING         = 1,  /* 0: disable, 1: enable, 0xFF: not support */
    DDP_FIELD_LEN_PRODUCT_CATEGORY        = 32, /* product type 
                                                   1: switch,  2: access point,  3: network camera,  4: video encoder
                                                   5: network video recoder,  6: NAS,  7: service router,  8: wireless controller,
                                                   9: wireless switch,  10: wireless router,  11: EPOS
                                                   (string) */
    DDP_FIELD_LEN_HARDWARE_VERSION        = 32, /* hardware version of device (string) */
    DDP_FIELD_LEN_SERIAL_NUMBER           = 32, /* serial number of device (string) */
    DDP_FIELD_LEN_INTERFACE_NUMBER        = 8,  /* interface 
                                                   value: Port 1, LAN1, WAN1, DMZ1 */
    DDP_FIELD_LEN_DEVICE_IPV6_ADDR        = 16, /* ipv6 address */
    DDP_FIELD_LEN_DEVICE_IPV6_PREFIX      = 1,  /* ipv6 address prefix if it is supported */
    DDP_FIELD_LEN_DEFAULT_GATEWAY_IPV6    = 16, /* default gateway in ipv6 */
    DDP_FIELD_LEN_PRIMARY_DNS_IPV6        = 16,
    DDP_FIELD_LEN_SECONDARY_DNS_IPV6      = 16,
    DDP_FIELD_LEN_DHCP_IPV6               = 1,
    DDP_FIELD_LEN_ALERT_REPORT            = 1,  /* set as 1 if device has critical event (0: no alert, 1: alert occurs) */
    DDP_FIELD_LEN_NTP_ENABLE              = 1,
    DDP_FIELD_LEN_NTP_SERVER              = 64,
    DDP_FIELD_LEN_NTP_FORMAT              = 1,
    DDP_FIELD_LEN_NTP_SUPPORT             = 1,
    DDP_FIELD_LEN_RESERVED_16             = 16,

    /* 0xD0: Query Neighbor */
    DDP_FIELD_LEN_QUERY_NBR_ENTRY_NUM     = 1,

    /* 0xD2: Configure vlan fields */
    DDP_FIELD_LEN_VLAN_NAME               = 32,
    DDP_FIELD_LEN_VLAN_ID                 = 4,
    DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE      = 1,
    DDP_FIELD_LEN_VLAN_TAG_PLIST          = 64,
    DDP_FIELD_LEN_VLAN_UNTAG_LIST_TYPE    = 1,
    DDP_FIELD_LEN_VLAN_UNTAG_PLIST        = 64,
    DDP_FIELD_LEN_VLAN_TYPE               = 1,
    DDP_FIELD_LEN_VLAN_STATE              = 1,
    DDP_FIELD_LEN_VLAN_LIST               = 512,

    /* 0xD3: Configure auto surveillance vlan fields */
    DDP_FIELD_LEN_ASV_ID                  = 4,
    DDP_FIELD_LEN_ASV_PRI                 = 4,
    DDP_FIELD_LEN_ASV_STATE               = 1,
    DDP_FIELD_LEN_ASV_TAG_LIST_TYPE       = 1,
    DDP_FIELD_LEN_ASV_TAG_PLIST           = 64,
    DDP_FIELD_LEN_ASV_DEFINED             = 1,

    DDP_FIELD_LEN_SNMP_GLOBAL_STATE       = 1,
    DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RO  = 32,
    DDP_FIELD_LEN_SNMP_COMMUNITY_NAME_RW  = 32,
    DDP_FIELD_LEN_SNMP_VIEW_NAME_RO       = 32,
    DDP_FIELD_LEN_SNMP_VIEW_NAME_RW       = 32,

    /* 0xD5: Configure for auto surveillance VLAN MAC fields */
    DDP_FIELD_LEN_ASV_USR_DEF_MAC_SRV_TYPE   = 1,
    DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_TYPE  = 1,
    DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_CATG  = 1,
    DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR       = 6,
    DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC  = 20,

    /* 0xD6: Get ASV MAC fields */
    DDP_FIELD_LEN_ASV_USR_DEF_MAC_ENTRY_NUM  = 1,

    /* 0xD7: Get Device Port Number fields */
    DDP_FIELD_LEN_PORT_NUM_TOTAL_UNIT     = 1,
    DDP_FIELD_LEN_PORT_NUM_UNIT_ID        = 1,
    DDP_FIELD_LEN_PORT_NUM_UNIT_PORTNUM   = 1,

    DDP_FIELD_LEN_ALERT_REPORT_INFO       = 128, /* alert report information (string) */
    DDP_FIELD_LEN_REBOOT                  = 0,
    DDP_FIELD_LEN_REBOOT_WAIT_TIME        = 4,
    DDP_FIELD_LEN_RESET                   = 0,
    DDP_FIELD_LEN_FACTORY_RESET           = 0,

    DDP_FIELD_LEN_SUB_SERVICE_TYPE        = 1,
    DDP_FIELD_LEN_SERVER_TYPE             = 1,
    DDP_FIELD_LEN_SERVER_IP_ADDR_TYPE     = 1,
    DDP_FIELD_LEN_UPGRADE_FILE_URL        = 128,
    DDP_FIELD_LEN_FW_LOCAL_PATH           = 128,
    DDP_FIELD_LEN_UPGRADE_REPLY_STATUS    = 1,
    DDP_FIELD_LEN_UPGRADE_FAIL_STR        = 128,
    DDP_FIELD_LEN_CFG_BACKUP_LOCAL_PATH   = 128,
    DDP_FIELD_LEN_CFG_RESTORE_LOCAL_PATH  = 128,

    /* 0x00D1, 0x00D8 */
    DDP_FIELD_LEN_SSID_NAME               = 32,
    DDP_FIELD_LEN_SSID_SEC_TYPE           = 4,
    DDP_FIELD_LEN_SSID_SEC_PASSWD         = 16,
    /* DDP information */
    DDP_FIELD_LEN_SPEC_VERSION            = 16,

    DDP_FIELD_LEN_DELAY_TIME              = 4,
};

/* define for authentication */
#define DDP_MAX_LEN_USERNAME     31
#define DDP_MAX_LEN_PASSWORD     31
#define DDP_FIELD_LEN_USERNAME   64
#define DDP_FIELD_LEN_PASSWORD   64
#define DDP_FIELD_LEN_USER_TYPE  4
/* user type */
enum {
    DDP_USER_TYPE_MIN    = 1,
    DDP_USER_TYPE_ADMIN  = DDP_USER_TYPE_MIN,
    DDP_USER_TYPE_USER   = 2,
    DDP_USER_TYPE_MAX    = DDP_USER_TYPE_USER, /* equal to the type with the largest value */
};

/* encrytion method */
enum {
    DDP_ENC_BASE64 = 1,
};

/* error code */
enum {
    DDP_ALL_FIELD_SUCCESS                 = 0,   /* All fields in the command are correctly committed to Device. */
    DDP_INVALID_FIELD_NAME                = 1,   /* Set_Basic_Settings: Device Name is error */
    DDP_INVALID_FIELD_IP                  = 2,   /* Set_Basic_Settings: Device IP is error */
    DDP_INVALID_FIELD_PORT                = 3,   /* Set_Basic_Settings: Web Port Number is error */
    DDP_INVALID_FIELD_NETMASK             = 4,   /* Set_Basic_Settings: Subnet Mask is error */
    DDP_INVALID_FIELD_GATEWAY             = 5,   /* Set_Basic_Settings: Default Gateway is error */
    DDP_INVALID_FIELD_PRIMARY_DNS         = 6,   /* Set_Basic_Settings: Primary DNS is error */
    DDP_INVALID_FIELD_SECONDARY_DNS       = 7,   /* Set_Basic_Settings: Secondary DNS is error */
    DDP_INVALID_FIELD_DHCP                = 8,   /* Set_Basic_Settings: DHCP is error */

    DDP_INVALID_FIELD_DATETIME            = 101, /* Set_Systime_Datetime: Date-time is error */
    DDP_INVALID_FIELD_TIMEZONE            = 102, /* Set_Systime_Datetime: Time-zone is error */
    DDP_INVALID_FIELD_DL_SAVING           = 103, /* Set_Systime_Datetime: Daylight Saving Time is error */
    DDP_INVALID_FIELD_NTP_ENABLE          = 104, /* Set_Systime_Datetime: NTP Enabled is error */
    DDP_INVALID_FIELD_NTP_SERVER          = 105, /* Set_Systime_Datetime: NTP Server is error */

    DDP_INVALID_FIELD_WIRELESS_ENABLE     = 201, /* Set_Wireless, Wireless_Simple_Setting : Enable Wireless is error */
    DDP_INVALID_FIELD_CHANNEL             = 202, /* Set_Wireless, Wireless_Simple_Setting: Wireless Channel is error */
    DDP_INVALID_FIELD_ESSID               = 203, /* Set_Wireless, Wireless_Simple_Setting: SSID is error */
    DDP_INVALID_FIELD_WIRELESS_MODE       = 204, /* Set_Wireless, Wireless_Simple_Setting: Wireless Mode is error */
    DDP_INVALID_FIELD_WIRELESS_ENCRYPTION = 205, /* Set_Wireless, Wireless_Simple_Setting: Encryption is error */
    DDP_INVALID_FIELD_WIRELESS_AUTH       = 206, /* Set_Wireless, Wireless_Simple_Setting: Authentication is error */
    DDP_INVALID_FIELD_WIRELESS_KEY_LEN    = 207, /* Set_Wireless: WEP Key Length Mode s is error */
    DDP_INVALID_FIELD_WIRELESS_KEY_FORMAT = 208, /* Set_Wireless: WEP Key Format is error */
    DDP_INVALID_FIELD_WIRELESS_TX_RATE    = 209, /* Set_Wireless: TX Rate is error */
    DDP_INVALID_FIELD_WIRELESS_WPA_KEY    = 210, /* Set_Wireless: WPA Key is error */
    DDP_INVALID_FIELD_WIRELESS_WEP_KEY    = 211, /* Set_Wireless: One of WEP Key1, WEP Key2, WEP Key3, WEP Key4 is error */
    DDP_INVALID_FIELD_WIRELESS_KEY        = 212, /* Wireless_Simple_Setting: Field Key is error */
    DDP_INVALID_FIELD_WIRELESS_WEP_SUPPORT= 213, /* Set_Wireless: AP does not support WEP */
    DDP_INVALID_FIELD_WIRELESS_AP_MANAGED = 214, /* Set_Wireless: Managed AP does not change SSID config by DDP */
    DDP_INVALID_FIELD_WIRELESS_VAP_ENABLE = 215, /* Set_Wireless, Wireless_Simple_Setting: Enable VAP is error */
    DDP_INVALID_FIELD_WIRELESS_SSID_VLAN  = 216, /* Set_Wireless: SSID's VLAN is error */
    DDP_INVALID_FIELD_WIRELESS_SECURITY   = 217, /* Set_Wireless: SSID's security is error */

    DDP_INVALID_FIELD_PPPOE_ENABLE        = 301, /* Set_Advance_Network: PPPoE Setting is error */
    DDP_INVALID_FIELD_PPPOE_USERNAME      = 302, /* Set_Advance_Network: PPPoE User's ID is error */
    DDP_INVALID_FIELD_PPPOE_PASSWORD      = 303, /* Set_Advance_Network: PPPoE User's Password is error */
    DDP_INVALID_FIELD_DDNS_INDEX          = 304, /* Set_Advance_Network: Set DDNS Provider Index is error */
    DDP_INVALID_FIELD_DDNS_USERNAME       = 305, /* Set_Advance_Network: DDNS User's ID is error */
    DDP_INVALID_FIELD_DDNS_PASSWORD       = 306, /* Set_Advance_Network: DDNS User's Password is error */
    DDP_INVALID_FIELD_DDNS_HOSTNAME       = 307, /* Set_Advance_Network: DDNS Server is error */

    DDP_INVALID_FIELD_CHANGEID_ADMIN                 = 1001,  /* Change_ID: The name of administrator's account can't be changed. */
    DDP_INVALID_FIELD_CHANGEID_ADMIN_LENGTH          = 1002,  /* Change_ID: The length of of administrator's name is
                                                                 too long. Meanwhile the maximum character is stored in
                                                                 field Extension of Execution_Status structure */
    DDP_INVALID_FIELD_CHANGEID_ADMIN_ILLEGAL_CHAR    = 1003,  /* Change_ID: The proposed value of administrator's name
                                                                 contains illegal character. */
    DDP_INVALID_FIELD_CHANGEID_PASSWORD              = 1011,  /* Change_ID: The password of administrator's account
                                                                 can't be changed. */
    DDP_INVALID_FIELD_CHANGEID_PASSWORD_LENGTH       = 1012,  /* Change_ID: The length of of administrator's password
                                                                 is too long. Meanwhile the maximum character is stored in
                                                                 field Extension of Execution_Status structure */
    DDP_INVALID_FIELD_CHANGEID_PASSWORD_ILLEGAL_CHAR = 1013,  /* Change_ID: The proposed value of administrator's
                                                                 password contains illegal character. */

    DDP_INVALID_FIELD_SNMP_VIEWNAME_LOST             = 2001,  /* SNMP_Configuration: Lack of SNMP view name. This error
                                                                 number is used to notify the server that the SNMP view
                                                                 name must be configured in the request packet. */

    DDP_INVALID_FIELD_VLAN_SET                       = 3001,  /* VLAN Configuration Error Code */
    DDP_INVALID_FIELD_VLAN_SET_NAME_IN_DATA                ,
    DDP_INVALID_FIELD_VLAN_SET_NAME_LEN                    ,
    DDP_INVALID_FIELD_VLAN_SET_ID_IN_DATA                  ,
    DDP_INVALID_FIELD_VLAN_SET_ID_LEN                      ,
    DDP_INVALID_FIELD_VLAN_SET_TLIST_TYPE_IN_DATA 	       ,
    DDP_INVALID_FIELD_VLAN_SET_TLIST_TYPE_LEN              ,
    DDP_INVALID_FIELD_VLAN_SET_TLIST_IN_DATA               ,
    DDP_INVALID_FIELD_VLAN_SET_TLIST_LEN                   ,
    DDP_INVALID_FIELD_VLAN_SET_ULIST_TYPE_IN_DATA          ,
    DDP_INVALID_FIELD_VLAN_SET_ULIST_TYPE_LEN              ,
    DDP_INVALID_FIELD_VLAN_SET_ULIST_IN_DATA               ,
    DDP_INVALID_FIELD_VLAN_SET_ULIST_LEN                   ,
    DDP_INVALID_FIELD_VLAN_SET_DRIVER,

    DDP_INVALID_FIELD_VLAN_GET_NAME_IN_DATA                ,
    DDP_INVALID_FIELD_VLAN_GET_NAME_LEN                    ,
    DDP_INVALID_FIELD_VLAN_GET_ID_IN_DATA                  ,
    DDP_INVALID_FIELD_VLAN_GET_ID_LEN                      ,
    DDP_INVALID_FIELD_VLAN_GET_TLIST_TYPE_IN_DATA 	       ,
    DDP_INVALID_FIELD_VLAN_GET_TLIST_TYPE_LEN              ,
    DDP_INVALID_FIELD_VLAN_GET_TLIST_IN_DATA               ,
    DDP_INVALID_FIELD_VLAN_GET_TLIST_LEN                   ,
    DDP_INVALID_FIELD_VLAN_GET_ULIST_TYPE_IN_DATA          ,
    DDP_INVALID_FIELD_VLAN_GET_ULIST_TYPE_LEN              ,
    DDP_INVALID_FIELD_VLAN_GET_ULIST_IN_DATA               ,
    DDP_INVALID_FIELD_VLAN_GET_ULIST_LEN                   ,

    DDP_INVALID_FIELD_ASV_SET                        = 3101,  /* ASV Configuration */
    DDP_INVALID_FIELD_ASV_SET_ID_IN_DATA                   ,
    DDP_INVALID_FIELD_ASV_SET_ID_LEN                       ,
    DDP_INVALID_FIELD_ASV_SET_PRIORITY_IN_DATA             ,
    DDP_INVALID_FIELD_ASV_SET_PRIORITY_LEN                 ,
    DDP_INVALID_FIELD_ASV_SET_STATE_IN_DATA                ,
    DDP_INVALID_FIELD_ASV_SET_STATE_LEN                    ,
    DDP_INVALID_FIELD_ASV_SET_TLIST_TYPE_IN_DATA 	       ,
    DDP_INVALID_FIELD_ASV_SET_TLIST_TYPE_LEN               ,
    DDP_INVALID_FIELD_ASV_SET_TLIST_IN_DATA                ,
    DDP_INVALID_FIELD_ASV_SET_TLIST_LEN                    ,
    DDP_INVALID_FIELD_ASV_SET_DRIVER                       ,

    DDP_INVALID_FIELD_ASV_GET_ID_IN_DATA                   ,
    DDP_INVALID_FIELD_ASV_GET_ID_LEN                       ,
    DDP_INVALID_FIELD_ASV_GET_PRIORITY_IN_DATA             ,
    DDP_INVALID_FIELD_ASV_GET_PRIORITY_LEN                 ,
    DDP_INVALID_FIELD_ASV_GET_STATE_IN_DATA                ,
    DDP_INVALID_FIELD_ASV_GET_STATE_LEN                    ,
    DDP_INVALID_FIELD_ASV_GET_TLIST_TYPE_IN_DATA 	       ,
    DDP_INVALID_FIELD_ASV_GET_TLIST_TYPE_LEN               ,
    DDP_INVALID_FIELD_ASV_GET_TLIST_IN_DATA                ,
    DDP_INVALID_FIELD_ASV_GET_TLIST_LEN                    ,

    DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET            = 3201,  /* User Defined MAC for ASV Configuration */
    DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET_DRIVER,
    DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET_TABLE_FULL,
    DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET_NO_SUCH_ENTRY,
    DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET_DUPLICATE,

    DDP_INVALID_FIELD_ASV_USR_DEF_MAC_GET            = 3301,  /* User Defined MAC for ASV GET */
    DDP_INVALID_FIELD_ASV_USR_DEF_MAC_GET_LEN,

    DDP_INVALID_FIELD_PORT_NUM_GET                   = 3301,  /* Get device port number */
    DDP_INVALID_FIELD_PORT_NUM_GET_LEN,
    DDP_INVALID_FIELD_PORT_NUM_GET_TRUNCATE,
    
    DDP_INVALID_FIELD_QUERY_NEIGHBOR_IN_DATA         = 4000,
    DDP_INVALID_FIELD_DUPLICATED_ENTRY               = 65534, /* General: This return number is used to notify server
                                                                 that this entry had been created. */

    DDP_INVALID_FIELD_SERVER_MESSAGE_TYPE_ERROR      = 65535, /* General: This return number is used to notify server
                                                                 that client can not understand this format. */
};

/* data structure of DDP header
 *   ipVer : ipv4 or ipv6
 *   identifier :
 *   seq        : sequence number
 *   opcode     :
 *   macAddr    :
 *   ipAddr     : ip address for v4/v6
 *   retCode    : refer to ret code above
 *   protoVer   : version of ddp protocol
 *   bodyLen    : length of messgae body after header
 */
struct ddp_header {
    UINT1 ipVer; // internal use

    UINT2 identifier;
    UINT2 seq;
    UINT2 opcode;
    UINT1 macAddr[MAC_ADDRLEN];
    union {
          UINT1  ipv4Addr[IPV4_ADDRLEN];
          UINT1  ipv6Addr[IPV6_ADDRLEN];
    }     ipAddr;
    UINT2 retCode;
    UINT2 protoVer;
    UINT2 bodyLen;
};

/* data structure of interface list
 *   macAddr : mac address of this interface
 *   ifindex : index number
 *   name    : interface name
 *   next    : pointer to next interface
 */
struct ddp_interface {
    UINT1  macAddr[MAC_ADDRLEN];
    UINT4  ifindex;
    INT1*  name;
    struct ddp_interface* next;
};

/* data structure of msg queue node 
 *   ifindex : interface index
 *   sender  : sender address
 *   size    : size of data in bytes
 *   payload : content of message
 */
struct ddp_message {
    UINT4  ifindex;
    struct sockaddr_storage sender;
    INT4   size;
    UINT1* payload;
};

enum {
    DDP_OP_DISABLE    = 0,
    DDP_OP_ENABLE     = 1,

    DDP_OP_AUTHEN_OFF = 0,
    DDP_OP_AUTHEN_ON  = 1,
};
/* data structure of op code information 
 *   opcode : opcode number
 *   enable : enable opcode or not (0: no, 1: yes)
 *   authen : require authen or not (0: no, 1: yes)
 */
struct op_info {
    UINT2 opcode;
    UINT2 enable;
    UINT4 authen;
};

/* data structure of user information
 *   name : username (max length 31 char)
 *   pass : password (max length 31 char)
 *   type : 1 -> admin , 2 -> user
 */
struct ddp_user {
    INT1 name[DDP_MAX_LEN_USERNAME + 1];
    INT1 pass[DDP_MAX_LEN_PASSWORD + 1];
    INT4 type;
};

/* values of operation status in execution status structure
 */
enum {
    DDP_EXE_ST_DONE    = 0,
    DDP_EXE_ST_RENEWAL = 1,
    DDP_EXE_ST_WAIT    = 2
};

enum enumRet_Switch{
    DDP_RET_SWITCH_OK,
    DDP_RET_SWITCH_FAIL,
    DDP_RET_SWITCH_ERROR_PARAMETER,
    DDP_RET_SWITCH_ERROR_MEM,
    DDP_RET_SWITCH_ERROR_INIT,
    DDP_RET_SWITCH_ERROR_INIT_DB,
    DDP_RET_SWITCH_ERROR_INSERT_NODE,
    DDP_RET_SWITCH_ERROR_PURGE_NODE,
    DDP_RET_SWITCH_ERROR_UPDATE_TTL,
    DDP_RET_SWITCH_ERROR_MAX_NODE_PER_PKT,
    DDP_RET_SWITCH_ERROR_EMPTY_DB
};

/* data structure of execution status
 *   errNo     : refer to error number table
 *   opStatus  : 0 -> Done, 1 -> Renewal, 2 -> Wait
 *   waitTimer : in milli-second (>= 0)
 *   extension : for special case
 */
struct ddp_exe_status {
    UINT4  errNo;
    UINT4  opStatus;
    UINT4  waitTimer;
    UINT4  extension;
};

/* Upgrade and backup
 */
#define DDP_MAX_LEN_FILE_URL 128
/* Type values
 */
enum {
    DDP_UPGRADE_CONFIG     = 1,
    DDP_UPGRADE_QUERY      = 2,

    DDP_UPGRADE_TYPE_TFTP  = 1,
    DDP_UPGRADE_TYPE_HTTP  = 2,

    DDP_UPGRADE_IPV4_ADDR  = 1,
    DDP_UPGRADE_IPV6_ADDR  = 2,
};


/*
 * ASV user defined MAC
 *
 */
enum {
    DDP_ASV_USR_MAC_SUB_TYPE_ADD    = 0,
    DDP_ASV_USR_MAC_SUB_TYPE_DEL    = 1,

    DDP_ASV_USR_MAC_MAC_TYPE_MAC    = 0,
    DDP_ASV_USR_MAC_MAC_TYPE_OUI,

    DDP_ASV_USR_MAC_CATG_VMS_SERVER = 0,
    DDP_ASV_USR_MAC_CATG_VMS_CLIENT,
    DDP_ASV_USR_MAC_CATG_VIDEO_ENCODER,
    DDP_ASV_USR_MAC_CATG_NAS,
    DDP_ASV_USR_MAC_CATG_OTHER
};

/* Upgrade status values
 */
enum {
    DDP_UPGRADE_SUCCESS = 0,
    DDP_UPGRADE_FAIL,
    DDP_UPGRADE_UNDER_PROCESS,
    DDP_UPGRADE_PROCESS_DONE,
};
/* Data structure of upgrade and backup
 *   subServiceType : 1 -> parameter configure, 2 -> download status query
 *   serverType     : 1 -> TFTP, 2 -> HTTP
 *   ipType         : 1 -> IPV4, 2 -> IPV6
 *   ipAddr         : ip address of server
 *   url            : file path in server
 */
struct ddp_upgrade_info {
    UINT1 subServiceType;
    UINT1 serverType;
    UINT1 ipType;
    union {
          UINT1  ipv4Addr[IPV4_ADDRLEN];
          UINT1  ipv6Addr[IPV6_ADDRLEN];
    }     ipAddr;
    UINT1 url[DDP_MAX_LEN_FILE_URL];
};

#define DDP_VLANID_MIN 1
#define DDP_VLANID_MAX 4096

/* 0x00D2 VLAN */
/* data structure of global PF VLAN information
 * au1vlan_name[DDP_FIELD_LEN_VLAN_NAME]: vlan name
 * u4vlan_id: vlan id
 * u1vlan_tag_list_type: type of tag port member list
 * u1vlan_tag_plist[DDP_FIELD_LEN_VLAN_TAG_PLIST]: tag port member list
 * u1vlan_untag_list_type: type of untag port member list
 * au1vlan_untag_plist[DDP_FIELD_LEN_VLAN_UNTAG_PLIST]: untag port member list
 */
struct stDDP_PF_VLAN {
     INT1 ai1vlan_name[DDP_FIELD_LEN_VLAN_NAME];
     INT1 ai1vlan_tag_plist[DDP_FIELD_LEN_VLAN_TAG_PLIST];
     INT1 ai1vlan_untag_plist[DDP_FIELD_LEN_VLAN_UNTAG_PLIST];
    UINT4 u4vlan_id;
    UINT1 u1vlan_tag_list_type;
    UINT1 u1vlan_untag_list_type;
};

/* 0x00D3 ASV */
/* data structure of global PF ASV information
 * u4asv_id: ASV id
 * u4asv_priority: ASV priority
 * u1asv_state: state of ASV
 * u1asv_tag_list_type: type of tag port list
 * u1asv_tag_plist[DDP_FIELD_LEN_VLAN_TAG_PLIST]: tag port member list
 */
struct stDDP_PF_ASV {
     INT1 ai1asv_tag_plist[DDP_FIELD_LEN_ASV_TAG_PLIST];
    UINT4 u4asv_id;
    UINT4 u4asv_priority;
    UINT1 u1asv_state;
    UINT1 u1asv_tag_list_type;
};

struct _stDDP_ASV_USR_MAC_NODE {
    UINT1 u1Mac_type;
    UINT1 u1Mac_Category;
     INT1 ai1Mac_addr[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR];
     INT1 ai1Mac_addr_desc[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC];
};

typedef struct _stDDP_ASV_USR_MAC_NODE stDDP_ASV_USR_MAC_NODE_t;

/* 0x00D5 user define MAC address for ASV */
/* data structure of global PF ASV MAC information
 * u1Sub_srv_type: add(0) / delete(1)
 * u1Mac_type: type of user-defined address
 * u1Mac_Category: category of address
 * au1Mac_addr[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR]: user-defined address
 * au1Mac_addr_desc[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC]: description of address
 */
struct stDDP_PF_ASV_USR_MAC {
    UINT1 u1Sub_srv_type;
    stDDP_ASV_USR_MAC_NODE_t stDDP_ASV_USR_MAC_NODE;
};

/* 0x00D6 get user define MAC address for ASV */
/* data structure of ASV MAC information
 * u1TotalEntry: total mac entries
 * u1Mac_type: type of user-defined address
 * u1Mac_Category: category of address
 * au1Mac_addr[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR]: user-defined address
 * au1Mac_addr_desc[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC]: description of address
 */
struct stDDP_ASV_USR_MAC {
	UINT1 u1TotalEntry;
    UINT1 u1Mac_type;
    UINT1 u1Mac_Category;
     INT1 ai1Mac_addr[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR];
     INT1 ai1Mac_addr_desc[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC];
};

typedef struct stDDP_ASV_USR_MAC stDDP_ASV_USR_MAC_t;

/* 0x00D7 get device port number */
/* data structure of port number
 * u1TotalEntry: total units
 * u1Unit_ID: Unit identifier, 0xFF for standalone.
 * u1Port_Number: total port number on certain unit.
 * pstNextUnit: next unit
 *
 */
struct _stDDP_PORT_NODE {
    UINT1 u1Unit_ID;
    UINT1 u1Port_Number;
    struct _stDDP_PORT_NODE *pstNextUnit;
};

typedef struct _stDDP_PORT_NODE stDDP_PORT_NODE_t;

struct _stDDP_PORT_LIST
{
    UINT1 u1TotalUnit;
    stDDP_PORT_NODE_t stDDP_PortNode;
};

typedef struct _stDDP_PORT_LIST stDDP_PORT_LIST_t;

/* Data structure of neighbor device
 *   u1TotalEntry : the number of entries in current packet.
 *   au1MAC       : device MAC address
 *   au1IPv4      : device system ip (IPv4)
 *   au1IPv6      : device system ip (IPv6)
 *   au1Mask      : device subnet mask
 *   au1PortNum   : device port/interface number
 *   ai1Name      : product name
 *   ai1SysName   : system name
 *   ai1Category  : product category
 *   ai1FwVersion : firmware version
 */
struct stDDP_neighbor_entry {
    UINT1 u1TotalEntry;
    INT1  ai1Name[DDP_FIELD_LEN_PRODUCT_NAME];
    INT1  ai1Category[DDP_FIELD_LEN_PRODUCT_CATEGORY];
    INT1  ai1FwVersion[DDP_FIELD_LEN_VERSION];
    INT1  ai1MAC[MAC_ADDRLEN];
    INT1  ai1SysName[DDP_FIELD_LEN_SYSTEM_NAME];
    INT1  ai1IPv4[IPV4_ADDRLEN];
    INT1  ai1Mask[DDP_FIELD_LEN_SUBNET_MASK];
    INT1  ai1IPv6[IPV6_ADDRLEN];
    INT1  ai1PortNumBmp[DDP_FIELD_LEN_INTERFACE_NUMBER];
};

/* Data structure of neighbor node information
 *   u2InIF : index number of incoming port
 *   u2TTL  : time to live of node information
 *   stDDP_Neighb_Entry : Entry information
 *   pstPreNode  : pointer to previous node
 *   pstNextNode : pointer to next node
 */
struct stDDP_Neighb_Node
{
    UINT2 u2InIF;
    /* If the kept node isn't updated in certain period, purge it.*/
    UINT2 u2TTL;
    struct stDDP_neighbor_entry stDDP_Neighb_Entry;
    struct stDDP_Neighb_Node* pstPreNode;
    struct stDDP_Neighb_Node* pstNextNode;
};

/* Data structure of neighbor node information list
 *   u4TotalNodes : Record how many nodes on this incoming port
 *   pstFirstNode : pointer to the first node on the list
 *   pstTailNode  : pointer to the last node on the list
 */
struct stDDP_Neighb_List
{
    UINT4  u4TotalNodes;
    struct stDDP_Neighb_Node* pstFirstNode;
    struct stDDP_Neighb_Node* pstTailNode;
};

/* Data structure of general report
 *   ai1ProductName[DDP_FIELD_LEN_PRODUCT_NAME]       : product name
 *   ai1Category[DDP_FIELD_LEN_PRODUCT_CATEGORY]      : product type
 *   ai1FwVersion[DDP_FIELD_LEN_VERSION]              : firmware version
 *   ai1MAC[MAC_ADDRLEN]                              : device MAC
 *   ai1HwVersion[DDP_FIELD_LEN_HARDWARE_VERSION]     : hardware version
 *   ai1SN[DDP_FIELD_LEN_SERIAL_NUMBER]               : serial number
 *   ai1SysName[DDP_FIELD_LEN_SYSTEM_NAME]            : system name
 *   ai1IPv4[IPV4_ADDRLEN]                            : ipv4 address
 *   ai1Mask[DDP_FIELD_LEN_SUBNET_MASK]               : subnet mask
 *   ai1PortNumBmp[DDP_FIELD_LEN_INTERFACE_NUMBER]    : src port number
 *   ai1IPv6[IPV6_ADDRLEN]                            : ipv6 address
 *   ai1IPv6Prefix[DDP_FIELD_LEN_DEVICE_IPV6_PREFIX]  : ipv6 prefix
 */
struct stDDP_general_report {
	INT1  ai1ProductName[DDP_FIELD_LEN_PRODUCT_NAME];
    INT1  ai1Category[DDP_FIELD_LEN_PRODUCT_CATEGORY];
    INT1  ai1FwVersion[DDP_FIELD_LEN_VERSION];
    INT1  ai1MAC[MAC_ADDRLEN];
    INT1  ai1HwVersion[DDP_FIELD_LEN_HARDWARE_VERSION];
    INT1  ai1SN[DDP_FIELD_LEN_SERIAL_NUMBER];
    INT1  ai1SysName[DDP_FIELD_LEN_SYSTEM_NAME];
    INT1  ai1IPv4[IPV4_ADDRLEN];
    INT1  ai1Mask[DDP_FIELD_LEN_SUBNET_MASK];
    INT1  ai1PortNumBmp[DDP_FIELD_LEN_INTERFACE_NUMBER];
    INT1  ai1IPv6[IPV6_ADDRLEN];
    INT1  ai1IPv6Prefix[DDP_FIELD_LEN_DEVICE_IPV6_PREFIX];

};
#endif /* _DDP_GENERAL_H_ */
