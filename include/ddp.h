
#ifndef _DDP_H_
#define _DDP_H_

#include <errno.h>

#include "ddp_general.h"

#ifdef __cplusplus
extern "C" {
#endif

/* UDP port number in client (62976) */
#define UDP_PORT_CLIENT 0xF600
/* UDP port number in server (62992) */
#define UDP_PORT_SERVER 0xF610
/* Size of packet container in bytes */
#define UDP_CONTAINER_SIZE 1200
/* Interval to generate general report packet in seconds */
#define REPORT_PACKET_INTERVAL_DEFAULT 30
/* Interval to read socket in seconds */
#define RECV_THREAD_CHECK_PKT_INTERVAL_DEFAULT 3
/* Interval to check msg queue in seconds */
#define PROC_THREAD_CHECK_QUEUE_INTERVAL_DEFAULT 3

/* macro to print out debug message
 * g_debugFlag is a switch */
enum {
    DDP_DEBUG_NONE                = 0x00000000,
    DDP_DEBUG_GENERAL             = 0x00000001,
    DDP_DEBUG_PRINT_RECV_MSG_HEX  = 0x00000010,
    DDP_DEBUG_PRINT_SRV_MSG_HDR   = 0x00000100,
    DDP_DEBUG_PRINT_SRV_MSG_HEX   = 0x00000200,
    DDP_DEBUG_PRINT_IN_MSG_HEX    = 0x00001000,
    DDP_DEBUG_PRINT_IN_MSG_HDR    = 0x00002000,
    DDP_DEBUG_PRINT_OUT_MSG_HEX   = 0x00010000,
    DDP_DEBUG_PRINT_OUT_MSG_HDR   = 0x00020000,
    DDP_DEBUG_PRINT_OUT_MSG_EXEST = 0x00040000,
    DDP_DEBUG_PRINT_CMD           = 0x00100000,
    DDP_DEBUG_PRINT_PLATFORM      = 0x01000000,
    DDP_DEBUG_PRINT_SWITCH        = 0x02000000
};

enum {
    DDP_INFO_NONE                = 0x00000000,
    DDP_INFO_GENERAL             = 0x00000001
};

#define DDP_DEBUG_LEVEL(level, ...) if (g_debugFlag & level) { printf(__VA_ARGS__); }
#define DDP_DEBUG(...) DDP_DEBUG_LEVEL(DDP_DEBUG_GENERAL, __VA_ARGS__)
#define DDP_DEBUG_SWITCH(...) DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_SWITCH, __VA_ARGS__)
#define DDP_INFO_LEVEL(level, ...) if (g_infoFlag & level) { printf("\nDDP_INFO:"__VA_ARGS__); }
#define DDP_INFO(...) DDP_INFO_LEVEL(DDP_INFO_GENERAL, __VA_ARGS__)

/* An internel signal to terminate threads and exit program. */
#define DDP_OVER_SIG 0x80000000

/* values to define whether device is capable of processing general report message */
enum {
    DDP_PROCESS_REPORT_NO  = 0x0,
    DDP_PROCESS_REPORT_YES = 0x1,
    DDP_PROCESS_REPORT_DEFAULT = DDP_PROCESS_REPORT_NO,
};

#define DDP_SHELL_SOCKET_DIR  "/tmp/ddp"
#define DDP_SHELL_SOCKET_FILE "usock"
/* values of run state */
enum {
    DDP_RUN_STATE_HALT = 0,
    DDP_RUN_STATE_RUN  = 1,

    DDP_RUN_STATE_GET = 0,
    DDP_RUN_STATE_SET = 1,
};

extern INT4 g_iMachEndian;
extern UINT4 g_debugFlag;
extern UINT4 g_infoFlag;
extern INT4 g_reportTimerInterval;
extern INT4 g_iSockfd;
extern INT4 g_iSockfdV6;
extern INT4 g_ipv4Ready;
extern INT4 g_ipv6Ready;
extern struct msg_queue* g_mq;
extern struct ddp_interface* g_ifList;
extern UINT1 MAC_ALL[];
extern UINT1 IPV4_LOCAL[];
extern UINT1 IPV4_BRCAST[];
extern INT1* DDP_IPV6_MCAST;
extern struct op_info g_opList[];
extern INT4 g_upgradeFlag;
extern pthread_mutex_t g_upgradeMutex;
extern UINT4 g_role;
extern INT4 g_iLoop;
extern INT4 g_srvSockfd;
extern INT4 g_srvSockfdV6;
extern INT4 g_srvV1Sockfd;
extern struct msg_queue* g_srvMq;
extern struct msg_queue* g_srvV1Mq;

/* ddp.c */
/* ddp_get_op_number
 *   function to return total number of supported opcode
 *
 *   return : total number of supported opcode
 */
INT4  ddp_get_op_number(void);
/* ddp_get_seq_count
 *   function to get next sequence number
 *
 *   return : sequence number
 */
UINT2 ddp_get_seq_count(void);
/* ddp_get_loop_flag
 *   function to get the flag that represent process is running or not
 *
 *   return : 0 -> stop, 1 -> running
 */
INT4  ddp_get_loop_flag(void);
/* ddp_terminate
 *   function to terminate process
 *
 *   return : 0 -> success, others -> error
 */
INT4  ddp_terminate(void);
/* ddp_entrance
 *   entry point of ddp engine
 *   normally the process will not return from this function
 *
 *   return : 0 -> success, others -> error
 */
INT4  ddp_entrance(INT4 argc, INT1** argv);
/* ddp_run_state
 *   function to get and set run state.
 *
 *   action : DDP_RUN_STATE_GET or DDP_RUN_STATE_SET
 *   value  : new run state if action = DDP_RUN_STATE_SET
 *            0 if action = DDP_RUN_STATE_GET
 *
 *   return : the current state if action = DDP_RUN_STATE_GET
 *            the previous state if action = DDP_RUN_STATE_SET
 *            -1 -> error
 */
INT4 ddp_run_state(INT4 action, INT4 value);

/* ddp_thread.c */
/* ddp_thread_recv_process
 *   major task of recv thread
 *
 *   strThreadName : thread name
 *
 *   return : none
 */
void ddp_thread_recv_process(INT1* strThreadName);
/* ddp_thread_proc_process
 *   major task of proc thread
 *
 *   strThreadName : thread name
 *
 *   return : none
 */
void ddp_thread_proc_process(INT1* strThreadName);
/* ddp_thread_srv_process
 *   major task of srv thread
 *
 *   strThreadName : thread name
 *
 *   return : none
 */
void ddp_thread_srv_process(INT1* strThreadName);

void ddp_thread_srvV1_process(INT1* strThreadName);

/* ddp_srv.c */
/* ddp_srv_process_message
 *   function to process message retrieved from srv message queue.
 * 
 *   inMsg : message from queue
 *
 *   return : 0 -> success, others -> error
 */
INT4 ddp_srv_process_message(struct ddp_message* inMsg);

INT4 ddp_srvv1_proto_process_message(struct ddp_message* inMsg);


/* ddp_srv_process_config
 *   function to parse config file to get the position of proxy.
 *
 * path : path of config file
 *
 * return : 0 -> success, others -> error
 */
INT4 ddp_srv_process_config(INT1* path);

/* ddp_shell.c */
/* ddp_shell_stop
 *   function to clen up unix socket.
 *
 * return : 0
 */
INT4 ddp_shell_stop(void);
/* ddp_shell_start
 *   function to initiate unix socket for commandline
 *
 * return : 0 -> success, others -> error
 */
INT4 ddp_shell_start(void);
/* ddp_shell_thread
 *   thread function to receive command from console.
 *
 * strThreadName : thread name
 *
 * return : 0 -> success, others -> error
 */
INT4 ddp_shell_thread(INT1* strThreadName);

/* ddp_alrm_queue.c */
/* ddp_alrm_queue_add
 *   function to add an alarm object to alrm_queue
 *
 * opcode : opcode
 * interval : the time before alarm is up in seconds
 * msg : alarm object
 *
 * return : 0 -> success, others -> error
 */
INT4 ddp_alrm_queue_add(UINT2 opcode, UINT4 interval, struct ddp_message *msg);
/* ddp_alrm_queue_thread
 *   major task of alarm queue thread
 *
 *   strThreadName : thread name
 *
 *   return : none
 */
void ddp_alrm_queue_thread(INT1* strThreadName);

/* ddp_proto.c */
/* ddp_proto_capable_process_report
 *   function to verify whether the device can handle general report or not
 *
 *   no input argument
 *
 *   return: DDP_PROCESS_REPORT_NO -> no, DDP_PROCESS_REPORT_YES -> yes
 */
INT4 ddp_proto_capable_process_report(struct ddp_interface* ifs);
/* ddp_proto_process_message
 *   dispatch function to process message
 *
 *   inMsg : input raw message
 *
 *   return : 0 -> success, others -> error
 */
INT4  ddp_proto_process_message(struct ddp_message* inMsg);
/* ddp_proto_check_op_authen
 *   function to query whether input opcode requires authentication
 *
 *   opcode : opcode
 *
 *   return : DDP_OP_AUTHEN_ON or DDP_OP_AUTHEN_OFF
 */
UINT1 ddp_proto_check_op_authen(UINT2 opcode);
/* ddp_proto_check_op_enable
 *   function to query whether this opcode is enabled on not
 *
 *   opcode : opcode
 *
 *   return : DDP_OP_ENABLE or DDP_OP_DISABEL
 */
UINT1 ddp_proto_check_op_enable(UINT2 opcode);
/* ddp_proto_query_user_type
 *   function to get the user type of the specified user name
 *
 *   ifs : interface
 *   encUsername : encoded user name
 *   encPassword : encoded password
 *
 *   return : DDP_USER_TYPE_ADMIN, DDP_USER_TYPE_USER, or others (no match)
 */
INT4 ddp_proto_query_user_type(struct ddp_interface* ifs, INT1* encUsername, INT1* encPassword);
/* ddp_proto_verify_authen
 *   function to do authentication of input user name and password
 *
 *   ifs : interface
 *   encUsername : encoded user name
 *   encPassword : encoded password
 *
 *   return : 0 -> pass, others -> fail
 */
INT4 ddp_proto_verify_authen(struct ddp_interface* ifs, INT1* encUsername, INT1* encPassword);

/* util.c */
/* pack_exe_status
 *   pack execution status into buffer in order to be sent
 *
 *   pkt : buffer to be sent
 *   st : ddp_exe_status struct
 *
 *   return : 0 -> success, others -> error
 */
INT4 pack_exe_status (UINT1* pkt, struct ddp_exe_status* st);
/* pack_header
 *   pack ddp_header into buffer in order to be sent
 *
 *   msg : buffer to be snet
 *   hdr : header which contains data
 *
 *   return : 0 -> success, others -> error
 */
INT4 pack_header(UINT1* msg, struct ddp_header* hdr);
/* print_unpack_header
 *   display values which contains in header
 *
 *   hdr : header to be displayed
 *
 *   return : none
 */
void print_unpack_header(struct ddp_header *hdr);
/* extract_header
 *   retrieve values from received packet
 *
 *   data : received packet
 *   dataLen : length of received packet
 *   hdr : header to be fillin
 *
 *   return : 0 -> success, others -> error
 */
INT4 extract_header(UINT1* data, INT4 dataLen, struct ddp_header* hdr);
/* print_pack_header
 *   display values directly from the raw data
 *
 *   data : original packet
 *   dataLen : length of data
 *
 *   return : none
 */
void print_pack_header(UINT1* data, INT4 dataLen);
/* print_exe_status
 *   function to print out the content of execution status
 *
 *   st : pointer to struct ddp_exe_status
 *
 *   return : none
 */
void print_exe_status(struct ddp_exe_status* st);
/* print_message_hex
 *   display raw data in hex format
 *
 *   data : raw data
 *   dataLe : length of data
 *
 *   return : none
 */
void print_message_hex(UINT1* data, INT4 dataLen);
/* print_upgrade_info
 *   display information kept in ddp_upgade_info
 *
 *   info : ddp_upgrade_info struct
 *
 *   return : none
 */
void print_upgrade_info(struct ddp_upgrade_info* info);
/* create_delay_msg
 *   create a delay messags from input arguments for delay timer
 *
 *   ifs : input interface
 *   inHdr : inbound ddp header
 *   inmsg : inbound ddp message
 *
 *   return : a message for delay timer
 */
struct ddp_message* create_delay_msg(struct ddp_interface* ifs, struct ddp_header* inHdr, struct ddp_message* inMsg);
/* string_decode
 *   decode input string according to the specified decode algorithm
 *
 *   inStr : input string
 *   outBuf : output buffer 
 *   outBufLen : size of output buffer
 *   decMethod : decode algorithm
 *
 *   return : 0 -> success, others -> error
 */
INT4 string_decode(INT1* inStr, INT1* outBuf, INT4 outBufLen, INT4  decMethod);
/* string_encode
 *   encode input string according to the specified encode algorithm
 *
 *   inStr : input string
 *   outBuf : output buffer 
 *   outBufLen : size of output buffer
 *   encMethod : encode algorithm
 *
 *   return : 0 -> success, others -> error
 */
INT4 string_encode(INT1* inStr, INT1* outBuf, INT4 outBufLen, INT4  encMethod);
/* snedout_msg
 *   subroutine to send pkt
 *
 *   pkt : input packet which has information about receiver address
 *   outPkt : packet to be sent
 *   outPktLen : length of output packet
 *
 *   return : 0 -> success, others -> error
 */
INT4 sendout_msg(struct ddp_message* pkt, UINT1* outPkt, INT4 outPktLen);

INT4 srvV1_send_req(struct ddp_message* pkt, UINT1* outPkt, INT4 outPktLen);

/* ddp_proto_upgrade.c */
/* extract_upgrade_info
 *   retrieve values from raw data
 *
 *   info : ddp_upgrade_info struct to be fillin
 *   data : raw data
 *
 *   return : 0 -> success, others -> error
 */
INT4 extract_upgrade_info(struct ddp_upgrade_info* info, UINT1* data);
/* ddp_upgrade_get_flag
 *   function to get the state of upgrade flag
 *
 *   return : 0 -> no tftp thread, 1 -> tftp is running
 */
INT4 ddp_upgrade_get_flag(void);

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
i4fnDDP_proto_switch_NeighbDB_Update_AllNode_TTL(void);

/* i4fnDDP_proto_switch_NeighbDB_Init
 *   function to initialize neighbor information database
 *
 *   pgstDDP_Neighb_List : pointer to the db list, current glob list: gstDDP_Neighb_List
 *   pstNewNode : pointer to the new node to insert.
 *
 *   return : DDP_RET_SWITCH_OK -> success, others -> error
 */
INT4 i4fnDDP_proto_switch_NeighbDB_Init(void);

/* i4fnDDP_proto_switch_Init
 *   Initial function for switch api, called by vfnDDP_proto_switch_Init
 *   - Initialize database for neighbor information maintained by switch.
 *
 */
INT4 i4fnDDP_proto_switch_Init(void);

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
 *   u4len : message size
 *
 *   return : DDP_RET_SWITCH_OK -> success, others -> error
 */
INT4 i4fnDDP_proto_switch_Fill_QueryNeighb_Pkt(
        struct ddp_interface* pstDDP_IF,
        INT1* pi1Msg, UINT2* pu2len);

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
);

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
INT4 i4fnDDP_proto_switch_NeighbDB_delete_all(struct stDDP_Neighb_Node* pstDDP_Neighb_Node);

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
);

/*
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
i4fnDDP_proto_switch_NeighbDB_Purge_FromDB(INT1* pai1MAC);

INT4 ddp_srvv1_req_discovery();
INT4 ddp_srvv1_req_basic_info();
INT4 ddp_srvv1_shell_add_sendmsg(INT1* msg);

#ifdef __cplusplus
}
#endif

#endif /* _DDP_H_ */
