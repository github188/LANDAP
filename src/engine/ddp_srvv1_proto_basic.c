#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "ddp.h"
#include "ddp_platform.h"

typedef struct {
	INT1 	brandName[DDP_FIELD_LEN_BRAND_NAME];
	INT1 	productName[DDP_FIELD_LEN_PRODUCT_NAME];
	INT1 	modelName[DDP_FIELD_LEN_MODEL_NAME];
	INT1 	version[DDP_FIELD_LEN_VERSION];
	UINT2 	prodType;
	UINT4 	hwSpec;
	UINT1 	macAddr[DDP_FIELD_LEN_MAC_ADDR];
	INT1 	sysName[DDP_FIELD_LEN_SYSTEM_NAME];
	UINT1 	ipAddr[DDP_FIELD_LEN_DEVICE_IP_ADDR];
	UINT2 	webPort;
} srvv1_reply_discovery_body;

INT4
ddp_srvv1_proto_discovery
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = 0;

    if (	ifs == NULL
    		|| inHdr == NULL
			|| inMsg == NULL
    		/*|| inHdr->ipVer == IPV6_FLAG*/)
    	return -1;

    UINT1* inBody = NULL;
	if (inHdr->bodyLen > 0) {
		inBody = inMsg->payload + HDR_END_V4_OFFSET;
	}

    if (g_debugFlag & DDP_DEBUG_PRINT_OUT_MSG_HEX) {
        printf("output packet in hex\n");
        print_message_hex(inBody, inMsg->size - HDR_END_V4_OFFSET);
    }
    //DDP_INFO("%d\n", inMsg->size);

    srvv1_reply_discovery_body reply_content;
    INT4 pos = 0;
    INT4 len = 0;
    pos = 0; len = DDP_FIELD_LEN_BRAND_NAME;
    strncpy(reply_content.brandName, (INT1*)(inBody + pos), len);
    pos += DDP_FIELD_LEN_BRAND_NAME; len = DDP_FIELD_LEN_PRODUCT_NAME;
    strncpy(reply_content.productName, (INT1*)(inBody + pos), len);
    pos += DDP_FIELD_LEN_PRODUCT_NAME; len = DDP_FIELD_LEN_MODEL_NAME;
    strncpy(reply_content.modelName, (INT1*)(inBody + pos), len);
    pos += DDP_FIELD_LEN_MODEL_NAME; len = DDP_FIELD_LEN_VERSION;
    strncpy(reply_content.version, (INT1*)(inBody + pos), len);
    pos += DDP_FIELD_LEN_VERSION; len = DDP_FIELD_LEN_PRODUCT_TYPE;
    reply_content.prodType = *(UINT2*)(inBody + pos);
    pos += DDP_FIELD_LEN_PRODUCT_TYPE; len = DDP_FIELD_LEN_PRODUCT_HW_SPEC;
    reply_content.hwSpec = *(UINT4*)(inBody + pos);
    pos += DDP_FIELD_LEN_PRODUCT_HW_SPEC; len = DDP_FIELD_LEN_MAC_ADDR;
    memcpy(reply_content.macAddr, (INT1*)(inBody + pos), len);
    pos += DDP_FIELD_LEN_MAC_ADDR; len = DDP_FIELD_LEN_SYSTEM_NAME;
    strncpy(reply_content.sysName, (INT1*)(inBody + pos), len);
    pos += DDP_FIELD_LEN_SYSTEM_NAME; len = DDP_FIELD_LEN_DEVICE_IP_ADDR;
    memcpy(reply_content.ipAddr, (INT1*)(inBody + pos), len);
    pos += DDP_FIELD_LEN_DEVICE_IP_ADDR; len = DDP_FIELD_LEN_WEB_SERVICE_PORT;
    reply_content.webPort = *(UINT2*)(inBody + pos);

    INT1 msg[1024];
    memset(msg, 0, sizeof(msg));
    sprintf(msg,
    		"%s|%s|%s|%d.%d.%d.%d|%02x:%02x:%02x:%02x:%02x:%02x|%d",
			reply_content.brandName,
			reply_content.modelName,
			reply_content.sysName,
			reply_content.ipAddr[0], reply_content.ipAddr[1],
    		reply_content.ipAddr[2], reply_content.ipAddr[3],
			reply_content.macAddr[0],
			reply_content.macAddr[1],
			reply_content.macAddr[2],
			reply_content.macAddr[3],
			reply_content.macAddr[4],
			reply_content.macAddr[5],
			reply_content.webPort
			);

    //DDP_INFO("%s\n", msg);
    ddp_srvv1_shell_add_sendmsg(msg);

    return ret;
}

/* ddp_srvv1_proto_set_basic_info (A2)
 *   function to process set basic info packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input set basic info request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_srvv1_proto_set_basic_info
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = 0;

    if (	ifs == NULL
    		|| inHdr == NULL
			|| inMsg == NULL
    		/*|| inHdr->ipVer == IPV6_FLAG*/)
    	return -1;

    UINT1* inBody = NULL;
	if (inHdr->bodyLen > 0) {
		inBody = inMsg->payload + HDR_END_V4_OFFSET;
	}

    if(inBody != NULL) {
    	DDP_INFO("ret:%d %d\n", inHdr->retCode, atoi((char *)inBody));
    }
    else {
    	DDP_INFO("ret:%d\n", inHdr->retCode);
    }

    return ret;
}

/* ddp_srvv1_proto_user_verify (A3)
 *   function to process user verify packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input user verify request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_srvv1_proto_user_verify
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = 0;

    if (	ifs == NULL
    		|| inHdr == NULL
			|| inMsg == NULL
    		/*|| inHdr->ipVer == IPV6_FLAG*/)
    	return -1;

	DDP_INFO("ret:%d\n", inHdr->retCode);

    return ret;
}

/* ddp_srvv1_proto_change_id_psw (A4)
 *   function to process change id password packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input change id psw request message
 *
 *   return : 0 -> success, others -> error
 */
INT4
ddp_srvv1_proto_change_id_psw
(
    struct ddp_interface* ifs,
    struct ddp_header* inHdr,
    struct ddp_message* inMsg
)
{
    INT4 ret = 0;

    if (	ifs == NULL
    		|| inHdr == NULL
			|| inMsg == NULL
    		/*|| inHdr->ipVer == IPV6_FLAG*/)
    	return -1;

    UINT1* inBody = NULL;
	if (inHdr->bodyLen > 0) {
		inBody = inMsg->payload + HDR_END_V4_OFFSET;
	}

    if(inBody) {
    	DDP_INFO("ret:%d %d\n", inHdr->retCode, atoi((char *)inBody));
    }
    else {
    	DDP_INFO("ret:%d\n", inHdr->retCode);
    }

    return ret;
}

/* ddp_proto_query_support_opt (A5)
 *   function to process query support opt packet and send through the incoming interfaces
 *
 *   ifs : interface
 *   inHdr : input header
 *   inMsg : input query support opt request message
 *
 *   return : 0 -> success, others -> error
 */
//INT4
//ddp_proto_query_support_opt
//(
//    struct ddp_interface* ifs,
//    struct ddp_header* inHdr,
//    struct ddp_message* inMsg
//)
//{
//    INT4 ret  = DDP_ALL_FIELD_SUCCESS;
//    return ret;
//}


