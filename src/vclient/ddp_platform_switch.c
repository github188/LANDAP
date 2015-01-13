
#include <stdio.h>
#include <string.h>

#include "ddp.h"
#include "ddp_platform.h"


extern struct stDDP_PF_VLAN gstDDP_PF_VLAN;
extern struct stDDP_PF_VLAN pf_vlan_arr[];
extern struct stDDP_PF_ASV gstDDP_PF_ASV;
extern INT1 gaiDDP_PF_DEFAULT_ASV_USR_MAC_ADDR[];
extern stDDP_ASV_USR_MAC_NODE_t gastDDP_ASV_MAC_List[];

/*****************************************
 * OP:   0x00D2
 * DESC: Set VLAN information
 *
 * Related field:
 *        DDP_FIELD_VLAN_NAME,
 *      DDP_FIELD_VLAN_ID,
 *      DDP_FIELD_VLAN_TAG_PLIST_TYPE,
 *      DDP_FIELD_VLAN_TAG_PLIST,
 *      DDP_FIELD_VLAN_UNTAG_PLIST_TYPE,
 *      DDP_FIELD_VLAN_UNTAG_PLIST
 *****************************************/

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
INT4
ddp_platform_set_if_vlan(struct ddp_interface* ifs)
{
    /* Add driver api here to configure VLAN on machine */
    /* Available information */
#if 0 /* uncomment this if necessary */
    if (ifs == NULL) {
        return DDP_INVALID_FIELD_VLAN_SET;
    }
#endif
    /*
    gstDDP_PF_VLAN.ai1vlan_name[DDP_FIELD_LEN_VLAN_NAME];
    gstDDP_PF_VLAN.u4vlan_id;
    gstDDP_PF_VLAN.u1vlan_tag_list_type;
    gstDDP_PF_VLAN.aivlan_tag_plist[DDP_FIELD_LEN_VLAN_TAG_PLIST];
    gstDDP_PF_VLAN.u1vlan_untag_list_type;
    gstDDP_PF_VLAN.ai1vlan_untag_plist[DDP_FIELD_LEN_VLAN_UNTAG_PLIST];
    */
    memcpy(&pf_vlan_arr[gstDDP_PF_VLAN.u4vlan_id - DDP_VLANID_MIN], &gstDDP_PF_VLAN, sizeof(gstDDP_PF_VLAN));
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_vlan_name
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_vlan_name
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_SET_NAME_DATA;
    }
    */

    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_VLAN_SET_NAME_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_VLAN_NAME) {
        return DDP_INVALID_FIELD_VLAN_SET_NAME_LEN;
    }

    /* set VLAN name of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "VLAN name: %s -> %s\n", gstDDP_PF_VLAN.ai1vlan_name, pu1data);
    memset(gstDDP_PF_VLAN.ai1vlan_name, 0, DDP_FIELD_LEN_VLAN_NAME);
    strncpy(gstDDP_PF_VLAN.ai1vlan_name, (INT1*)pu1data, i4dataLen);
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_vlan_id
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_vlan_id
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_SET_ID_DATA;
    }
    */

    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_VLAN_SET_ID_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_VLAN_ID) {
        return DDP_INVALID_FIELD_VLAN_SET_ID_LEN;
    }

    /* set VLAN id of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "VLAN id: %u -> %u\n", (UINT4)gstDDP_PF_VLAN.u4vlan_id, *(UINT4*)pu1data);
    gstDDP_PF_VLAN.u4vlan_id = DDP_NTOHL(*(UINT4*)pu1data);
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_vlan_tag_list_type
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_vlan_tag_list_type
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_SET_TLIST_TYPE_IN_DATA;
    }
    */
    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_VLAN_SET_TLIST_TYPE_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE) {
        return DDP_INVALID_FIELD_VLAN_SET_TLIST_TYPE_LEN;
    }

    /* set VLAN tag port list type of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "VLAN tag port list type: %d -> %d\n", gstDDP_PF_VLAN.u1vlan_tag_list_type, *pu1data);
    gstDDP_PF_VLAN.u1vlan_tag_list_type = *pu1data;
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_vlan_tag_list
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_vlan_tag_list
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_SET_TLIST_IN_DATA;
    }
    */
    INT4 i = 0;
    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_VLAN_SET_TLIST_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_VLAN_TAG_PLIST) {
        return DDP_INVALID_FIELD_VLAN_SET_TLIST_LEN;
    }

    /* set VLAN tag port list of the specified interface to platform
     */
    //DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "VLAN tag port list: %s -> %s\n", gstDDP_PF_VLAN.ai1vlan_tag_plist, pu1data);
    if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
        printf("VLAN tag port list:\n");
        for (i = 0; i < DDP_FIELD_LEN_VLAN_TAG_PLIST; i++) {
            printf("%02X", gstDDP_PF_VLAN.ai1vlan_tag_plist[i]);
            if (i == DDP_FIELD_LEN_VLAN_TAG_PLIST / 2 - 1) { printf("\n"); }
        }
        printf("\n    |     \n");
        printf("    v     \n");
        for (i = 0; i < DDP_FIELD_LEN_VLAN_TAG_PLIST; i++) {
            printf("%02X", *(pu1data + i));
            if (i == DDP_FIELD_LEN_VLAN_TAG_PLIST / 2 - 1) { printf("\n"); }
        }
        printf("\n");
    }
    memset(gstDDP_PF_VLAN.ai1vlan_tag_plist, 0, DDP_FIELD_LEN_VLAN_TAG_PLIST);
    memcpy(gstDDP_PF_VLAN.ai1vlan_tag_plist, (INT1*)pu1data, i4dataLen);
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_vlan_untag_list_type
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_vlan_untag_list_type
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_SET_ULIST_TYPE_IN_DATA;
    }
    */

    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_VLAN_SET_ULIST_TYPE_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE) {
        return DDP_INVALID_FIELD_VLAN_SET_ULIST_TYPE_LEN;
    }

    /* set VLAN untag port list type of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "VLAN untag port list type: %d -> %d\n", gstDDP_PF_VLAN.u1vlan_untag_list_type, *pu1data);
    gstDDP_PF_VLAN.u1vlan_untag_list_type = *pu1data;
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_vlan_untag_list
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_vlan_untag_list
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_SET_ULIST_IN_DATA;
    }
    */
    INT4 i = 0;
    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_VLAN_SET_ULIST_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_VLAN_UNTAG_PLIST) {
        return DDP_INVALID_FIELD_VLAN_SET_ULIST_LEN;
    }

    /* set VLAN untag port list of the specified interface to platform
     */
    //DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "VLAN untag port list: %s -> %s\n", gstDDP_PF_VLAN.ai1vlan_untag_plist, pu1data);
    if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
        printf("VLAN untag port list:\n");
        for (i = 0; i < DDP_FIELD_LEN_VLAN_UNTAG_PLIST; i++) {
            printf("%02X", gstDDP_PF_VLAN.ai1vlan_untag_plist[i]);
            if (i == DDP_FIELD_LEN_VLAN_UNTAG_PLIST / 2 - 1) { printf("\n"); }
        }
        printf("\n    |     \n");
        printf("    v     \n");
        for (i = 0; i < DDP_FIELD_LEN_VLAN_UNTAG_PLIST; i++) {
            printf("%02X", *(pu1data + i));
            if (i == DDP_FIELD_LEN_VLAN_UNTAG_PLIST / 2 - 1) { printf("\n"); }
        }
        printf("\n");
    }
    memset(gstDDP_PF_VLAN.ai1vlan_untag_plist, 0, DDP_FIELD_LEN_VLAN_UNTAG_PLIST);
    memcpy((INT1*)gstDDP_PF_VLAN.ai1vlan_untag_plist, (INT1*)pu1data, i4dataLen);
    return DDP_RETCODE_OK;
}

/*****************************************
 * OP:   0x00D3
 * DESC: Set ASV (Auto Surveillance VLAN) information
 *
 * Related field:
 *  DDP_FIELD_ASV_ID,
 *  DDP_FIELD_ASV_PRI,
 *  DDP_FIELD_ASV_STATE,
 *  DDP_FIELD_ASV_TAG_PLIST_TYPE,
 *  DDP_FIELD_ASV_TAG_PLIST
 *****************************************/
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
INT4
ddp_platform_set_if_asv_hw(struct ddp_interface* ifs)
{
    /* Add driver api here to configure ASV on machine */
    /* Available information */
#if 0 /* uncomment this if necessary */
    if (ifs == NULL) {
        return DDP_INVALID_FIELD_ASV_SET;
    }
#endif
    /*
    au1vlan_name[DDP_FIELD_LEN_VLAN_NAME];
    u4vlan_id;
    u1vlan_tag_list_type;
    u1vlan_tag_plist[DDP_FIELD_LEN_VLAN_TAG_PLIST];
    u1vlan_untag_list_type;
    au1vlan_untag_plist[DDP_FIELD_LEN_VLAN_UNTAG_PLIST];
    */
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_asv_id
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_asv_id
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_SET_ID_DATA;
    }
    */

    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_ASV_SET_ID_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_ASV_ID) {
        return DDP_INVALID_FIELD_ASV_SET_ID_LEN;
    }

    /* set ASV id of the specified interface to platform*/
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "ASV id: %u -> %u\n", (UINT4)gstDDP_PF_ASV.u4asv_id, *(UINT4 *)pu1data);

    gstDDP_PF_ASV.u4asv_id = DDP_NTOHL(*pu1data);
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_asv_priority
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_asv_priority
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_SET_PRIORITY_DATA;
    }
    */

    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_ASV_SET_PRIORITY_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_ASV_PRI) {
        return DDP_INVALID_FIELD_ASV_SET_PRIORITY_LEN;
    }

    /* set ASV priority of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "ASV priority: %u -> %u\n", (UINT4)gstDDP_PF_ASV.u4asv_priority, *(UINT4*)pu1data);
    gstDDP_PF_ASV.u4asv_priority = DDP_NTOHL(*pu1data);
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_asv_state
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_asv_state
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_SET_STATE_DATA;
    }
    */

    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_ASV_SET_STATE_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_ASV_STATE) {
        return DDP_INVALID_FIELD_ASV_SET_STATE_LEN;
    }

    /* set ASV state of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "ASV state: %d -> %d\n", gstDDP_PF_ASV.u1asv_state, *pu1data);
    gstDDP_PF_ASV.u1asv_state = *pu1data;
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_asv_tag_list_type
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_asv_tag_list_type
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_SET_TLIST_TYPE_IN_DATA;
    }
    */

    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_ASV_SET_TLIST_TYPE_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_ASV_TAG_LIST_TYPE) {
        return DDP_INVALID_FIELD_ASV_SET_TLIST_TYPE_LEN;
    }

    /* set ASV tag port list type of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "ASV tag port list type: %d -> %d\n", gstDDP_PF_ASV.u1asv_tag_list_type, *pu1data);
    gstDDP_PF_ASV.u1asv_tag_list_type = *pu1data;
    return DDP_RETCODE_OK;
}

/* ddp_platform_set_if_asv_tag_list
 *   function to set new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_set_if_asv_tag_list
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_SET_TLIST_IN_DATA;
    }
    */
    INT4 i = 0;
    if (pu1data == NULL || i4dataLen == 0) {
        return DDP_INVALID_FIELD_ASV_SET_TLIST_IN_DATA;
    }

    if (i4dataLen > DDP_FIELD_LEN_ASV_TAG_PLIST) {
        return DDP_INVALID_FIELD_ASV_SET_TLIST_LEN;
    }

    /* set ASV tag port list of the specified interface to platform
     */
    //DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "ASV tag port list: %s -> %s\n", gstDDP_PF_ASV.ai1asv_tag_plist, pu1data);
    if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
        printf("ASV tag port list:\n");
        for (i = 0; i < DDP_FIELD_LEN_ASV_TAG_PLIST; i++) {
            printf("%02X", gstDDP_PF_ASV.ai1asv_tag_plist[i]);
            if (i == DDP_FIELD_LEN_ASV_TAG_PLIST / 2 - 1) { printf("\n"); }
        }
        printf("\n    |     \n");
        printf("    v     \n");
        for (i = 0; i < DDP_FIELD_LEN_ASV_TAG_PLIST; i++) {
            printf("%02X", *(pu1data + i));
            if (i == DDP_FIELD_LEN_ASV_TAG_PLIST / 2 - 1) { printf("\n"); }
        }
        printf("\n");
    }
    memset(gstDDP_PF_ASV.ai1asv_tag_plist, 0, DDP_FIELD_LEN_ASV_TAG_PLIST);
    memcpy((INT1*)gstDDP_PF_ASV.ai1asv_tag_plist, (INT1*)pu1data, i4dataLen);
    return DDP_RETCODE_OK;
}

/*****************************************
 * OP:   0x00D5
 * DESC: Set user defined MAC address for ASV
 *
 * Related field:
 *  DDP_FIELD_ASV_USR_DEF_MAC_SRV_TYPE
 *
 *****************************************/
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
INT4
ddp_platform_set_if_asv_usr_def_mac_hw
(
    struct ddp_interface* pstDDP_IF_ifs
)
{
    /* Add driver api here to configure ASV on machine */
    /* Available information */
#if 0 /* uncomment this if necessary */
    if (ifs == NULL) {
        return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET;
    }
#endif

    /*
    u1Sub_srv_type;
    u1Mac_type;
    u1Mac_Category;
    au1Mac_addr[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR];
    au1Mac_addr_desc[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC];
    */
    return DDP_RETCODE_OK;
}

/*****************************************
 * OP:   0x00D5
 * DESC: Set user defined MAC address for ASV
 *
 * Related field:
 *  DDP_FIELD_ASV_USR_DEF_MAC_SRV_TYPE,
 *  DDP_FIELD_ASV_USR_DEF_MAC_ADDR_TYPE,
 *  DDP_FIELD_ASV_USR_DEF_MAC_ADDR_CATG,
 *  DDP_FIELD_ASV_USR_DEF_MAC_ADDR,
 *  DDP_FIELD_ASV_USR_DEF_MAC_ADDR_DESC
 *****************************************/
/* ddp_platform_set_if_asv_usr_def_mac
 *   function to set user-defined ASV MAC information.
 *
 *   This function directly accesses PF gstDDP_PF_ASV_USR_MAC.
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data:        input data, struct stDDP_PF_ASV_USR_MAC
 *   i4dataLen:      data length, sizeof(struct stDDP_PF_ASV_USR_MAC)
 *
 *   return : DDP_RETCODE_OK -> success, others -> error
 */
INT4
ddp_platform_set_if_asv_usr_def_mac
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4 i4dataLen
)
{
    UINT4 u4Cnt = 0;
    UINT4 u4CurrentEntry = 0;
    struct stDDP_PF_ASV_USR_MAC *pstDDP_ASV_MAC_Cur = NULL;

    /* Add driver api here to configure ASV on machine */
    /* Available information */
#if 0 /* uncomment this if necessary */
    if (ifs == NULL) {
        return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET;
    }
#endif
    if((pu1data == NULL) || (i4dataLen <= 0))
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : Invalid parameters.\n", __FUNCTION__, __LINE__);
        return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET;
    }

    if(i4dataLen < sizeof(struct stDDP_PF_ASV_USR_MAC))
    {
        DDP_DEBUG_SWITCH("\n%s (%d) : Invalid length<%d>.\n", __FUNCTION__, __LINE__, i4dataLen);
        return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET;
    }

    /*
    u1Sub_srv_type;
    u1Mac_type;
    u1Mac_Category;
    au1Mac_addr[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR];
    au1Mac_addr_desc[DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC];
    */
    pstDDP_ASV_MAC_Cur = (struct stDDP_PF_ASV_USR_MAC *)pu1data;

    switch(pstDDP_ASV_MAC_Cur->u1Sub_srv_type)
    {
        case DDP_ASV_USR_MAC_SUB_TYPE_ADD:
            DDP_DEBUG_SWITCH("\n=== %s: Action add, mac[%02x:%02x:%02x:%02x:%02x:%02x]\n", __FUNCTION__,
                    pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[0], pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[1],
                    pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[2], pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[3],
                    pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[4], pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[5]);
             /* Check duplication */
            u4CurrentEntry = 0;
            for(u4Cnt=1; u4Cnt<=kDDP_SWITCH_MAX_ENTRY_PER_PACKET; u4Cnt++)
            {
                if(strcmp(gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr, pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr) == 0)
                {
                    DDP_DEBUG_SWITCH("\n%s (%d) : input mac [%02x:%02x:%02x:%02x:%02x:%02x] is duplicate at entry<%d>\n", __FUNCTION__, __LINE__,
                            gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[0], gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[1],
                            gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[2], gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[3],
                            gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[4], gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[5], u4Cnt);
                    return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET_DUPLICATE;
                }
            }

            /* Scan for empty entries. */
            u4CurrentEntry = 0;
            for(u4Cnt=1; u4Cnt<=kDDP_SWITCH_MAX_ENTRY_PER_PACKET; u4Cnt++)
            {
                /* Find empty entry and add. */
                if(strcmp(gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr, gaiDDP_PF_DEFAULT_ASV_USR_MAC_ADDR) == 0)
                {
                    DDP_DEBUG_SWITCH("\n%s: Add mac to entry<%d>\n", __FUNCTION__, u4Cnt);
                    u4CurrentEntry = u4Cnt;
                    gastDDP_ASV_MAC_List[u4CurrentEntry].u1Mac_type = pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.u1Mac_type;
                    gastDDP_ASV_MAC_List[u4CurrentEntry].u1Mac_Category = pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.u1Mac_Category;
                    memcpy(gastDDP_ASV_MAC_List[u4CurrentEntry].ai1Mac_addr, pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr, DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR);
                    memcpy(gastDDP_ASV_MAC_List[u4CurrentEntry].ai1Mac_addr_desc, pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr_desc, DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC);
                    break;
                }
                else
                {
                    DDP_DEBUG_SWITCH("\n%s (%d) : entry<%d> has [%02x:%02x:%02x:%02x:%02x:%02x]\n", __FUNCTION__, __LINE__, u4Cnt,
                            gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[0], gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[1],
                            gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[2], gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[3],
                            gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[4], gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[5]);
                }
            }

            if(u4CurrentEntry == 0)
            {
                DDP_DEBUG_SWITCH("\n%s (%d) : No empty entry, table is full of <%d> entries.\n", __FUNCTION__, __LINE__, kDDP_SWITCH_MAX_ENTRY_PER_PACKET);
                return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET_TABLE_FULL;
            }
            else
            {
                DDP_DEBUG_SWITCH("\n%s (%d) : set entry<%d> mac[%02x:%02x:%02x:%02x:%02x:%02x]\n", __FUNCTION__, __LINE__, u4CurrentEntry,
                        gastDDP_ASV_MAC_List[u4CurrentEntry].ai1Mac_addr[0], gastDDP_ASV_MAC_List[u4CurrentEntry].ai1Mac_addr[1],
                        gastDDP_ASV_MAC_List[u4CurrentEntry].ai1Mac_addr[2], gastDDP_ASV_MAC_List[u4CurrentEntry].ai1Mac_addr[3],
                        gastDDP_ASV_MAC_List[u4CurrentEntry].ai1Mac_addr[4], gastDDP_ASV_MAC_List[u4CurrentEntry].ai1Mac_addr[5]);
            }

            break;

        case DDP_ASV_USR_MAC_SUB_TYPE_DEL:
            DDP_DEBUG_SWITCH("\n=== %s: Action delete, mac[%02x:%02x:%02x:%02x:%02x:%02x]\n", __FUNCTION__,
                    pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[0], pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[1],
                    pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[2], pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[3],
                    pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[4], pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[5]);

            /* Scan for empty entries. */
            u4CurrentEntry = 0;
            for(u4Cnt=1; u4Cnt<=kDDP_SWITCH_MAX_ENTRY_PER_PACKET; u4Cnt++)
            {
                /* Find entry to delete */
                if(strcmp(gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr, pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr) == 0)
                {
                    DDP_DEBUG_SWITCH("\n%s: Delete entry<%d>\n", __FUNCTION__, u4Cnt);
                    u4CurrentEntry = u4Cnt;
                    /* Delete the entry and set to default. */
                    gastDDP_ASV_MAC_List[u4CurrentEntry].u1Mac_type = kDDP_PF_DEFAULT_ASV_USR_MAC_SRV_TYPE;
                    gastDDP_ASV_MAC_List[u4CurrentEntry].u1Mac_Category = kDDP_PF_DEFAULT_ASV_USR_MAC_ADDR_CATG;
                    memcpy(gastDDP_ASV_MAC_List[u4CurrentEntry].ai1Mac_addr, gaiDDP_PF_DEFAULT_ASV_USR_MAC_ADDR, DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR);
                    memcpy(gastDDP_ASV_MAC_List[u4CurrentEntry].ai1Mac_addr_desc, (INT1*)ksDDP_PF_DEFAULT_ASV_USR_MAC_ADDR_DESC, DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC);
                    break;
                }
            }

            if(u4CurrentEntry == 0)
            {
                DDP_DEBUG_SWITCH("\n%s (%d) : No such entry[%02x:%02x:%02x:%02x:%02x:%02x].\n", __FUNCTION__, __LINE__,
                    pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[0], pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[1],
                    pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[2], pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[3],
                    pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[4], pstDDP_ASV_MAC_Cur->stDDP_ASV_USR_MAC_NODE.ai1Mac_addr[5]);
                return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_SET_NO_SUCH_ENTRY;
            }

            break;

        default:
            DDP_DEBUG_SWITCH("\n%s (%d) : Invalid sub-type<%d>.\n", __FUNCTION__, __LINE__, pstDDP_ASV_MAC_Cur->u1Sub_srv_type);
            break;
    }

    return DDP_RETCODE_OK;
}









/*****************************************
 * OP:   0x00D2
 * DESC: Get VLAN information
 *
 * Related field:
 *      DDP_FIELD_VLAN_NAME,
 *      DDP_FIELD_VLAN_ID,
 *      DDP_FIELD_VLAN_TAG_PLIST_TYPE,
 *      DDP_FIELD_VLAN_TAG_PLIST,
 *      DDP_FIELD_VLAN_UNTAG_PLIST_TYPE,
 *      DDP_FIELD_VLAN_UNTAG_PLIST
 *      DDP_FIELD_VLAN_LIST
 *      DDP_FIELD_VLAN_STATE
 *****************************************/

/* ddp_platform_get_if_vlan_name
 *   function to get new value of the specified field from platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : value buffer
 *   i4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_vlan_name
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_SET_NAME_DATA;
    }
    */
    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_VLAN_GET_NAME_IN_DATA;
    }

    /*if (*pi4dataLen > DDP_FIELD_LEN_VLAN_NAME) {
        return DDP_INVALID_FIELD_VLAN_GET_NAME_LEN;
    }*/
    struct vlan_container *vc = (struct vlan_container*)pu1data;printf("VLAN ID   %d *******\n", vc->vlanId);
    if (vc->vlanId < DDP_VLANID_MIN || vc->vlanId > DDP_VLANID_MAX) {
        return DDP_INVALID_FIELD_VLAN_GET_NAME_IN_DATA;
    }
    memcpy((INT1*)vc->buf, pf_vlan_arr[vc->vlanId - DDP_VLANID_MIN].ai1vlan_name, DDP_FIELD_LEN_VLAN_NAME);

    /*if(strlen(gstDDP_PF_VLAN.ai1vlan_name) > *pi4dataLen)
    {
        if(strlen(gstDDP_PF_VLAN.ai1vlan_name) > DDP_FIELD_LEN_VLAN_NAME){
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM,
                            "%s: line<%d> length(%d) limit(%d) is reached. \n",
                            __FUNCTION__, __LINE__,
                            strlen(gstDDP_PF_VLAN.ai1vlan_name),DDP_FIELD_LEN_VLAN_NAME);
            *pi4dataLen = DDP_FIELD_LEN_VLAN_NAME;
        }
        else{
            *pi4dataLen = strlen(gstDDP_PF_VLAN.ai1vlan_name);
        }
    }*/

    /* get VLAN name of the specified interface to platform
     */
    /*DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get VLAN name: %s \n", gstDDP_PF_VLAN.ai1vlan_name);
    memset(pu1data, 0, DDP_FIELD_LEN_VLAN_NAME);
    strncpy((INT1*)pu1data, gstDDP_PF_VLAN.ai1vlan_name, *pi4dataLen);*/
    
    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_vlan_id
 *   function to get new value of the specified field from platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_vlan_id
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_GET_ID_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_VLAN_GET_ID_IN_DATA;
    }

    if (*pi4dataLen > DDP_FIELD_LEN_VLAN_ID) {
        return DDP_INVALID_FIELD_VLAN_GET_ID_LEN;
    }

    /* get VLAN id of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get VLAN id: %u\n", (UINT4)gstDDP_PF_VLAN.u4vlan_id);
    *(UINT4 *)pu1data = DDP_NTOHL(gstDDP_PF_VLAN.u4vlan_id);
    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_vlan_tag_list_type
 *   function to get new value of the specified field from platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_vlan_tag_list_type
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_GET_TLIST_TYPE_IN_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_VLAN_GET_TLIST_TYPE_IN_DATA;
    }
    struct vlan_container *vc = (struct vlan_container*)pu1data;
    if (vc->vlanId < DDP_VLANID_MIN || vc->vlanId > DDP_VLANID_MAX) {
        return DDP_INVALID_FIELD_VLAN_GET_TLIST_TYPE_IN_DATA;
    }
    memcpy(vc->buf, &pf_vlan_arr[vc->vlanId - DDP_VLANID_MIN].u1vlan_tag_list_type, DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE);

    /*if (*pi4dataLen > DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE) {
        return DDP_INVALID_FIELD_VLAN_GET_TLIST_TYPE_LEN;
    }*/

    /* get VLAN tag port list type of the specified interface to platform
     */
    /*DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get VLAN tag port list type: %d\n", gstDDP_PF_VLAN.u1vlan_tag_list_type);
    *pu1data = gstDDP_PF_VLAN.u1vlan_tag_list_type;*/
    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_vlan_tag_list
 *   function to get new value of the specified field from platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_vlan_tag_list
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_GET_TLIST_IN_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_VLAN_GET_TLIST_IN_DATA;
    }
    struct vlan_container *vc = (struct vlan_container*)pu1data;
    if (vc->vlanId < DDP_VLANID_MIN || vc->vlanId > DDP_VLANID_MAX) {
        return DDP_INVALID_FIELD_VLAN_GET_TLIST_IN_DATA;
    } 
    memcpy(vc->buf, pf_vlan_arr[vc->vlanId - DDP_VLANID_MIN].ai1vlan_tag_plist, DDP_FIELD_LEN_VLAN_TAG_PLIST);
    /*if (*pi4dataLen > DDP_FIELD_LEN_VLAN_TAG_PLIST) {
        return DDP_INVALID_FIELD_VLAN_GET_TLIST_LEN;
    }*/

    /* get VLAN tag port list of the specified interface to platform
     */
    /*if(gstDDP_PF_VLAN.u1vlan_tag_list_type == 0){
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get VLAN tag port list: %s\n", gstDDP_PF_VLAN.ai1vlan_tag_plist);
    }else{
        print_message_hex((UINT1 *)gstDDP_PF_VLAN.ai1vlan_tag_plist, DDP_FIELD_LEN_VLAN_TAG_PLIST);
    }
    memset(pu1data, 0, DDP_FIELD_LEN_VLAN_TAG_PLIST);
    strncpy((INT1*)pu1data, gstDDP_PF_VLAN.ai1vlan_tag_plist, *pi4dataLen);*/
    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_vlan_untag_list_type
 *   function to get new value of the specified field from platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_vlan_untag_list_type
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_GET_ULIST_TYPE_IN_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_VLAN_GET_ULIST_TYPE_IN_DATA;
    }
    struct vlan_container *vc = (struct vlan_container*)pu1data;
    if (vc->vlanId < DDP_VLANID_MIN || vc->vlanId > DDP_VLANID_MAX) {
        return DDP_INVALID_FIELD_VLAN_GET_ULIST_TYPE_IN_DATA;
    }
    memcpy(vc->buf, &pf_vlan_arr[vc->vlanId - DDP_VLANID_MIN].u1vlan_untag_list_type, DDP_FIELD_LEN_VLAN_UNTAG_LIST_TYPE);
    /*if (*pi4dataLen > DDP_FIELD_LEN_VLAN_TAG_LIST_TYPE) {
        return DDP_INVALID_FIELD_VLAN_GET_ULIST_TYPE_LEN;
    }*/

    /* get VLAN untag port list type of the specified interface to platform
     */
    /*DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get VLAN untag port list type: %d\n", gstDDP_PF_VLAN.u1vlan_untag_list_type);
    *pu1data = gstDDP_PF_VLAN.u1vlan_untag_list_type;*/
    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_vlan_untag_list
 *   function to get new value of the specified field from platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_vlan_untag_list
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_VLAN_GET_ULIST_IN_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_VLAN_GET_ULIST_IN_DATA;
    }
    struct vlan_container *vc = (struct vlan_container*)pu1data;
    if (vc->vlanId < DDP_VLANID_MIN || vc->vlanId > DDP_VLANID_MAX) {
        return DDP_INVALID_FIELD_VLAN_GET_ULIST_IN_DATA;
    }
    memcpy(vc->buf, pf_vlan_arr[vc->vlanId - DDP_VLANID_MIN].ai1vlan_untag_plist, DDP_FIELD_LEN_VLAN_UNTAG_PLIST);
    /*if (*pi4dataLen > DDP_FIELD_LEN_VLAN_UNTAG_PLIST) {
        return DDP_INVALID_FIELD_VLAN_GET_ULIST_LEN;
    }*/

    /* get VLAN untag port list of the specified interface to platform
     */
    /*if(gstDDP_PF_VLAN.u1vlan_untag_list_type == 0){
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get VLAN untag port list: %s\n", gstDDP_PF_VLAN.ai1vlan_untag_plist);
    }else{
        print_message_hex((UINT1 *)gstDDP_PF_VLAN.ai1vlan_untag_plist, DDP_FIELD_LEN_VLAN_UNTAG_PLIST);
    }*/

//    memset(pu1data, 0, DDP_FIELD_LEN_VLAN_UNTAG_PLIST);
//    strncpy((INT1*)pu1data, gstDDP_PF_VLAN.ai1vlan_untag_plist, *pi4dataLen);
    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_vlan_list
 *   function to get vlan list of the specified field from platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_vlan_list
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return -1;
    }
    */
    UINT4 i = 0;
    INT4 byte = 0;
    INT4 offset = 0;
    if (pu1data == NULL || pi4dataLen == NULL) {
        return -2;
    }
    for (i = DDP_VLANID_MIN; i <= DDP_VLANID_MAX; i++) {
        /* u4vlan_id is used as enable flag in simulator
         * u4vlan_id: 0 -> disabled, others -> enabled
         */
        if (pf_vlan_arr[i - DDP_VLANID_MIN].u4vlan_id == 0) {
            continue;
        }
        byte = i / 8;
        offset = i % 8;
        *(pu1data + byte) |= (0x1 << (8 - offset));
    }

    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_vlan_state
 *   function to get vlan state of the specified field from platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_vlan_state
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return -1;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return -2;
    }
    struct vlan_container *vc = (struct vlan_container*)pu1data;
    if (vc->vlanId < DDP_VLANID_MIN || vc->vlanId > DDP_VLANID_MAX) {
        return -3;
    }
    if (pf_vlan_arr[vc->vlanId - DDP_VLANID_MIN].u4vlan_id > 0) {
        *(vc->buf) = 1;
    } else {
        *(vc->buf) = 0;
    }

    return DDP_RETCODE_OK;
}


/*****************************************
 * OP:   0x00D3
 * DESC: Get ASV (Auto Surveillance VLAN) information
 *
 * Related field:
 *  DDP_FIELD_ASV_ID,
 *  DDP_FIELD_ASV_PRI,
 *  DDP_FIELD_ASV_STATE,
 *  DDP_FIELD_ASV_TAG_PLIST_TYPE,
 *  DDP_FIELD_ASV_TAG_PLIST
 *****************************************/

/* ddp_platform_get_if_asv_id
 *   function to get new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_asv_id
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_GET_ID_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_ASV_GET_ID_IN_DATA;
    }

    if (*pi4dataLen > DDP_FIELD_LEN_ASV_ID) {
        return DDP_INVALID_FIELD_ASV_GET_ID_LEN;
    }

    /* get ASV id of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get ASV id: %u\n", (UINT4)gstDDP_PF_ASV.u4asv_id);
    *(UINT4 *)pu1data = DDP_NTOHL((UINT4)gstDDP_PF_ASV.u4asv_id);
    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_asv_priority
 *   function to get new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_asv_priority
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_GET_PRIORITY_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_ASV_GET_PRIORITY_IN_DATA;
    }

    if (*pi4dataLen > DDP_FIELD_LEN_ASV_PRI) {
        return DDP_INVALID_FIELD_ASV_GET_PRIORITY_LEN;
    }

    /* get ASV priority of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get ASV priority: %u\n", (UINT4)gstDDP_PF_ASV.u4asv_priority);
    *(UINT4 *)pu1data = DDP_NTOHL((UINT4)gstDDP_PF_ASV.u4asv_priority);
    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_asv_state
 *   function to get new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_asv_state
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_GET_STATE_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_ASV_GET_STATE_IN_DATA;
    }

    if (*pi4dataLen > DDP_FIELD_LEN_ASV_STATE) {
        return DDP_INVALID_FIELD_ASV_GET_STATE_LEN;
    }

    /* get ASV state of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get ASV state: %d\n", gstDDP_PF_ASV.u1asv_state);
    *pu1data = gstDDP_PF_ASV.u1asv_state;
    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_asv_tag_list_type
 *   function to get new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_asv_tag_list_type
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_GET_TLIST_TYPE_IN_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_ASV_GET_TLIST_TYPE_IN_DATA;
    }

    if (*pi4dataLen > DDP_FIELD_LEN_ASV_TAG_LIST_TYPE) {
        return DDP_INVALID_FIELD_ASV_GET_TLIST_TYPE_LEN;
    }

    /* get ASV tag port list type of the specified interface to platform
     */
    DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get ASV tag port list type: %d\n", gstDDP_PF_ASV.u1asv_tag_list_type);
    *pu1data = gstDDP_PF_ASV.u1asv_tag_list_type;
    return DDP_RETCODE_OK;
}

/* ddp_platform_get_if_asv_tag_list
 *   function to get new value of the specified field to platform
 *
 *   pstDDP_IF_ifs : interface
 *   pu1data : new value buffer
 *   *pi4dataLen : the buffer size of data
 *
 *   return : DDP_RETCODE_OK -> success, other -> error
 */
INT4
ddp_platform_get_if_asv_tag_list
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_GET_TLIST_IN_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_INVALID_FIELD_ASV_GET_TLIST_IN_DATA;
    }

    if (*pi4dataLen > DDP_FIELD_LEN_ASV_TAG_PLIST) {
        return DDP_INVALID_FIELD_ASV_GET_TLIST_LEN;
    }

    /* get ASV tag port list of the specified interface to platform
     */
    if(gstDDP_PF_ASV.u1asv_tag_list_type == 0){
        DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_PLATFORM, "Get ASV tag port list: %s\n", gstDDP_PF_ASV.ai1asv_tag_plist);
    }else{
        print_message_hex((UINT1 *)gstDDP_PF_ASV.ai1asv_tag_plist, DDP_FIELD_LEN_ASV_TAG_PLIST);
    }

    memset(pu1data, 0, DDP_FIELD_LEN_ASV_TAG_PLIST);
    strncpy((INT1*)pu1data, gstDDP_PF_ASV.ai1asv_tag_plist, *pi4dataLen);
    return DDP_RETCODE_OK;
}

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
ddp_platform_get_if_asv_usr_def_mac
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    UINT4 u4Offset = 0;
    UINT4 u4TmpLen = 0;
    UINT4 u4TotalEntry = 0;
    UINT4 u4Cnt = 0;

    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_GET_SRV_TYPE_IN_DATA;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_RET_SWITCH_ERROR_PARAMETER;
    }

    if (*pi4dataLen < DDP_REPLY_LEN_ASV_USR_DEF_MAC_GET) {
        DDP_DEBUG_SWITCH("\n%s: line<%d> Invalid reply length<%d>\n",
                  __FUNCTION__, __LINE__, *pi4dataLen);
        return DDP_INVALID_FIELD_ASV_USR_DEF_MAC_GET_LEN;
    }

    /* get ASV user defined MAC service type of the specified interface to platform
     */
    memset(pu1data, 0, *pi4dataLen);

    /* Scan for valid entries. */
    for(u4Cnt=1; u4Cnt<=kDDP_SWITCH_MAX_ENTRY_PER_PACKET; u4Cnt++)
    {
        if(strcmp(gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr, gaiDDP_PF_DEFAULT_ASV_USR_MAC_ADDR) != 0)
        {
            u4TotalEntry++;
        }
    }

    /* Fill in the entries. */
    u4Offset = 0;
    u4TmpLen = 0;

    /* Number of entries in this packet. */
    u4TmpLen = DDP_FIELD_LEN_ASV_USR_DEF_MAC_ENTRY_NUM;

    *(pu1data+u4Offset) = (INT1)u4TotalEntry;
    u4Offset+= u4TmpLen;

    if( (u4TotalEntry == 0) || (u4TotalEntry > kDDP_SWITCH_MAX_ENTRY_PER_PACKET) )
    {
        DDP_DEBUG_SWITCH("\n==== %s: line<%d>\nNo valid entry, entry<%d>\n",
                          __FUNCTION__, __LINE__, u4TotalEntry);
    }
    else
    {
        for(u4Cnt = 1; u4Cnt <= kDDP_SWITCH_MAX_ENTRY_PER_PACKET; u4Cnt++){
            if(strcmp(gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr, gaiDDP_PF_DEFAULT_ASV_USR_MAC_ADDR) == 0){
                DDP_DEBUG_SWITCH("\t line<%d>Skip empty entry<%d>.\n\n",
                    __LINE__, u4Cnt);
                continue;
            }

            /* MAC address type */
            u4TmpLen = DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_TYPE;
            if(u4Offset +u4TmpLen > *pi4dataLen){
                DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                        __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Cnt, *pi4dataLen);
                break;
            }

            *(pu1data+u4Offset) = (INT1) gastDDP_ASV_MAC_List[u4Cnt].u1Mac_type;
            u4Offset+= u4TmpLen;

            /* MAC address Category */
            u4TmpLen = DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_CATG;
            if(u4Offset +u4TmpLen > *pi4dataLen){
                DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                        __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Cnt, *pi4dataLen);
                break;
            }

            *(pu1data+u4Offset) = (INT1) gastDDP_ASV_MAC_List[u4Cnt].u1Mac_Category;
            u4Offset+= u4TmpLen;

            /* MAC address */
            u4TmpLen = DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR;
            if(u4Offset +u4TmpLen > *pi4dataLen){
                DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                        __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Cnt, *pi4dataLen);
                break;
            }

            memcpy(pu1data+u4Offset, &(gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr[0]), u4TmpLen);
            u4Offset+= u4TmpLen;

            /* MAC Address Description */
            u4TmpLen = DDP_FIELD_LEN_ASV_USR_DEF_MAC_ADDR_DESC;
            if(u4Offset +u4TmpLen > *pi4dataLen){
                DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for entry<%d>, limit size<%d>\n",
                        __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Cnt, *pi4dataLen);
                break;
            }

            memcpy(pu1data+u4Offset, &(gastDDP_ASV_MAC_List[u4Cnt].ai1Mac_addr_desc[0]), u4TmpLen);
            u4Offset+= u4TmpLen;
        }
    }

    /* Update data length */
    if(u4Offset < *pi4dataLen){
        *pi4dataLen = u4Offset;
        DDP_DEBUG_SWITCH("\t line<%d>Total entry<%d>, updated size<%d>\n\n",
                          __LINE__, u4TotalEntry, *pi4dataLen);
    }

    return DDP_RETCODE_OK;
}

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
ddp_platform_get_if_port_number
(
    struct ddp_interface* pstDDP_IF_ifs,
    UINT1* pu1data,
    INT4*  pi4dataLen
)
{
    INT4  i4RetVal = 0;
    UINT4 u4Offset = 0;
    UINT4 u4TmpLen = 0;
    UINT4 u4Cnt    = 0;
    stDDP_PORT_LIST_t stPortList;
    stDDP_PORT_NODE_t *pstCurUnit = NULL;
    /* No need interface currently, un-comment this if necessary */
    /*
    if (pstDDP_IF_ifs == NULL)
    {
        return DDP_INVALID_FIELD_PORT_NUM_GET;
    }
    */

    if (pu1data == NULL || pi4dataLen == NULL) {
        return DDP_RET_SWITCH_ERROR_PARAMETER;
    }

    DDP_DEBUG_SWITCH("\n====Enter %s, reserved len<%d>\n", __FUNCTION__, *pi4dataLen);

    if (*pi4dataLen < DDP_REPLY_LEN_PORT_NUMBER_GET) {
        DDP_DEBUG_SWITCH("\n%s: line<%d> Invalid reply length<%d>\n",
                  __FUNCTION__, __LINE__, *pi4dataLen);
        return DDP_INVALID_FIELD_PORT_NUM_GET_LEN;
    }

    /* get port number
     */
    memset(pu1data, 0, *pi4dataLen);
    memset(&stPortList, 0, sizeof(stDDP_PORT_LIST_t));

    /**********************************/
    /*
     * Reserved for vendor:
     * API for getting system port numbers.
     *
     * Each reply packet contains max
     * kDDP_SWITCH_MAX_UNIT_PER_PACKET unit entries.
     *
     * Required input:
     * *stDDP_PORT_LIST_t - List of switch unit and
     *                     corresponding port numbers
     *     consists of
     *         UINT1 u1TotalUnit;
     *         stDDP_PORT_NODE_t stDDP_PortNode;
     *               consists of
     *                    UINT1 u1Unit_ID;
     *                    UINT1 u1Port_Number;
     *                    stDDP_PORT_NODE_t *pstNextUnit;
     *         UINT1 u1Unit_ID;
     *         UINT1 u1Port_Number;
     *
     * *
     */
    /**********************************/
    i4RetVal = i4fnddp_platform_get_if_port_number_hw(&stPortList);

    if((i4RetVal != DDP_RET_SWITCH_OK) || (stPortList.u1TotalUnit == 0))
    {
        DDP_DEBUG_SWITCH("\n==== %s: line<%d>\nFailed to get device port number. error<%d>\n",
                          __FUNCTION__, __LINE__, i4RetVal);
        return DDP_RETCODE_FAIL;
    }

    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("\tGot port list\n");
        printf("\tTotal Units:%d\n", stPortList.u1TotalUnit);
        printf("\tUnit ID:%d\n", stPortList.stDDP_PortNode.u1Unit_ID);
        printf("\tPort Number:%d\n", stPortList.stDDP_PortNode.u1Port_Number);
        printf("\tNext Node<0x%x>\n", (unsigned int)stPortList.stDDP_PortNode.pstNextUnit);
    }

    /* Fill in the first entries. */
    u4Offset = 0;
    u4TmpLen = 0;
    u4Cnt    = stPortList.stDDP_PortNode.u1Unit_ID;

    /* Number of units in this packet. */
    u4TmpLen = DDP_FIELD_LEN_PORT_NUM_TOTAL_UNIT;
    if(u4Offset +u4TmpLen > *pi4dataLen){
        DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for unit<%d>, limit size<%d>\n",
                __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Cnt, *pi4dataLen);
        return DDP_INVALID_FIELD_PORT_NUM_GET_TRUNCATE;
    }

    *(pu1data+u4Offset) = stPortList.u1TotalUnit;
    u4Offset+= u4TmpLen;

    /* unit number on first unit */
    u4TmpLen = DDP_FIELD_LEN_PORT_NUM_UNIT_ID;
    if(u4Offset +u4TmpLen > *pi4dataLen){
        DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for unit<%d>, limit size<%d>\n",
                __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Cnt, *pi4dataLen);
        return DDP_INVALID_FIELD_PORT_NUM_GET_TRUNCATE;
    }

    *(pu1data+u4Offset) = stPortList.stDDP_PortNode.u1Unit_ID;
    u4Offset+= u4TmpLen;

    /* port number on first unit */
    u4TmpLen = DDP_FIELD_LEN_PORT_NUM_UNIT_PORTNUM;
    if(u4Offset +u4TmpLen > *pi4dataLen){
        DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for unit<%d>, limit size<%d>\n",
                __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Cnt, *pi4dataLen);
        return DDP_INVALID_FIELD_PORT_NUM_GET_TRUNCATE;
    }

    *(pu1data+u4Offset) = stPortList.stDDP_PortNode.u1Port_Number;
    u4Offset+= u4TmpLen;

    pstCurUnit = stPortList.stDDP_PortNode.pstNextUnit;

    while(pstCurUnit != NULL)
    {
        /* unit number on this unit */
        u4TmpLen = DDP_FIELD_LEN_PORT_NUM_UNIT_ID;
        if(u4Offset +u4TmpLen > *pi4dataLen){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for unit<%d>, limit size<%d>\n",
                    __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Cnt, *pi4dataLen);
            break;
        }

        *(pu1data+u4Offset) = stPortList.stDDP_PortNode.u1Unit_ID;
        u4Offset+= u4TmpLen;

        /* port number on this unit */
        u4TmpLen = DDP_FIELD_LEN_PORT_NUM_UNIT_PORTNUM;
        if(u4Offset +u4TmpLen > *pi4dataLen){
            DDP_DEBUG_SWITCH("\n%s: line<%d>Over size<%d> for unit<%d>, limit size<%d>\n",
                    __FUNCTION__, __LINE__, u4Offset +u4TmpLen, u4Cnt, *pi4dataLen);
            break;
        }

        *(pu1data+u4Offset) = stPortList.stDDP_PortNode.u1Port_Number;
        u4Offset+= u4TmpLen;


        pstCurUnit = pstCurUnit->pstNextUnit;
    }

    /* Update data length */
    if(u4Offset < *pi4dataLen){
        *pi4dataLen = u4Offset;
        DDP_DEBUG_SWITCH("\t line<%d>Total units<%d>, updated size<%d>\n\n",
                          __LINE__, stPortList.u1TotalUnit, *pi4dataLen);
    }

    return DDP_RETCODE_OK;
}

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
i4fnddp_platform_get_if_port_number_hw(stDDP_PORT_LIST_t *pstPortList)
{
    if (pstPortList == NULL) {
        return DDP_RET_SWITCH_ERROR_PARAMETER;
    }

    /* Fake data */
    pstPortList->u1TotalUnit = 1;
    pstPortList->stDDP_PortNode.u1Unit_ID = 1;
    pstPortList->stDDP_PortNode.u1Port_Number = 12;
    pstPortList->stDDP_PortNode.pstNextUnit = NULL;

    return DDP_RETCODE_OK;
}
