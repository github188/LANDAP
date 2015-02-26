#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "ddp.h"
#include "ddp_platform.h"

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

    char buf[100];
    sprintf(buf, "%d.%d.%d.%d", inHdr->ipAddr.ipv4Addr[0], inHdr->ipAddr.ipv4Addr[1],
    							inHdr->ipAddr.ipv4Addr[2], inHdr->ipAddr.ipv4Addr[3]);
    DDP_INFO("%s\n", buf);

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


