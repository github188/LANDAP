
#ifndef _TFTP_CLI_H_
#define _TFTP_CLI_H_

#ifdef __cplusplus
extern "C" {
#endif 

/* define tftp data types to make porting easily */
#define TFTP_INT1 char
#define TFTP_INT2 short
#define TFTP_INT4 int
#define TFTP_UINT1 unsigned char
#define TFTP_UINT2 unsigned short
#define TFTP_UINT4 unsigned int

/* udp port of tftp */
#define DDP_TFTP_PORT 69
/* length of data block of TFTP */
#define SEGSIZE 512
/* size of data packet
 * opcode (2 bytes) + block number (2 bytes) + data (512 bytes)
 */
#define PKTSIZE 516 /* SEGSIZE + 4 */
/* default timeout interval of tftp (in seconds) */
#define TFTP_DEFAULT_TIMEOUT 10

/* tftp mode string */
extern TFTP_INT1* g_tftpMode_bin;
extern TFTP_INT1* g_tftpMode_ascii;
/* timeout interval */
extern TFTP_INT4 g_tftpTimeout;

enum {
    TFTP_DEBUG_NONE    = 0x00000000,
    TFTP_DEBUG_GENERAL = 0x00000001,
    TFTP_DEBUG_PACKET  = 0x00000010,
};
/* tftp debug flag */
extern TFTP_INT4 g_tftpDebug;

/* opcode of TFTP */
enum {
    RRQ = 1,
    WRQ,
    DATA,
    ACK,
    ERROR,
};

/* action of tftp */
enum {
    DDP_TFTP_GET = 1,
    DDP_TFTP_PUT,
};

/* struct tftp_info
 *   To launch a tftp action, please fillin 
 *   serv : server address
 *   action : get or put
 *   mode : a pointer to mode string
 *   fremote : file path in remote server
 *   flocal : file path in local disk
 */
struct tftp_info {
    struct sockaddr_storage serv;
    TFTP_INT2   action;
    TFTP_INT1*  mode;
    TFTP_INT1   fremote[128]; /* the same size as that of url in request packet */
    TFTP_INT1   flocal[128];
    TFTP_UINT4  blk;
    TFTP_UINT4  fsize;
    TFTP_INT2   status;
    TFTP_INT1   statusStr[64];
};

/* The tftp launching function
 *   Fillin the necessary data in struct tftp_info,
 *   then pass to this function.
 *
 *   info : information passed to tftp worker thread
 *
 *   return : 0 -> success, others -> error
 */
TFTP_INT4 tftp_cli(struct tftp_info* info);

/* tftp_terminate
 *   stop the tftp cli thread
 *
 *   return : none
 */
void tftp_terminate(void);

#ifdef __cplusplus
}
#endif

#endif /* _TFTP_CLI_H_ */
