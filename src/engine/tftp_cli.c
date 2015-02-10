
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>

#include "tftp_cli.h"
/* Porting : ddp_general.h has some defines used in order to do thread status
 *           transfer. Developer can modify/remove those define for your own purpose.
 */
#include "ddp_general.h"
/* ddp_upgrade_set_flag is a function defined in ddp_proto_upgrade.c.
 * it is used to synchronize tftp activity that only 1 tftp connection is allowed.
 * When porting, you may remove this function.
 */
INT4 ddp_upgrade_set_flag(INT4 flag);


/* timeout interval in seconds */
TFTP_INT4 g_tftpTimeout = TFTP_DEFAULT_TIMEOUT;
/* time interval of inner do-while loop in case of no data received (in msec) */
TFTP_INT4 g_tftpLoopInterval = 200;
/* tftp mode string */
TFTP_INT1* g_tftpMode_bin   = (TFTP_INT1*)"octet";
TFTP_INT1* g_tftpMode_ascii = (TFTP_INT1*)"netascii";
/* flag to stop tftp process prematurely */
TFTP_INT1 g_tftpRun = 1;
/* to keep tftp thread id */
//extern pthread_t g_upgradeThreadId;

/* debug switch */
TFTP_INT4 g_tftpDebug = TFTP_DEBUG_GENERAL;
/* macro to printout debug string */
#define TFTP_DEBUG(...)  if (g_tftpDebug) { printf(__VA_ARGS__); }

/* counter method to replace alarm method */
#define TFTP_TIMEOUT_COUNTER 1

/* flag to enable using system tftp */
#define USE_SYS_TFTP
/* path of system tftp */
#define TFTP_PATH "/usr/bin/tftp"


/* tftp_print_packet
 *   print out field values of tftp packet.
 *
 *   but : raw tftp packet
 *
 *   return : none
 */
void
tftp_print_packet
(
    TFTP_UINT1* buf
)
{
    TFTP_UINT2 op = 0;
    TFTP_UINT2 tmp = 0;
    TFTP_INT1* ptr = NULL;

    if (buf == NULL) { return; }
    printf("TFTP packet:\n");
    memcpy(&op, buf, sizeof(op));
    op = ntohs(op);
    if (op == RRQ || op == WRQ) {
        if (op == RRQ) { printf("  op   : RRQ\n"); }
        else { printf("  op   : WRQ\n"); }
        ptr = (TFTP_INT1*)(buf + sizeof(op));
        printf("  name : %s\n", ptr);
        ptr = (TFTP_INT1*)(buf + sizeof(op) + strlen(ptr) + 1);
        printf("  mode : %s\n", ptr);
    }
    else if (op == DATA) {
        printf("  op  : DATA\n");
        memcpy(&tmp, buf + sizeof(op), sizeof(tmp));
        tmp = ntohs(tmp);
        printf("  blk : %d\n", tmp);
    }
    else if (op == ACK) {
        printf("  op  : ACK\n");
        memcpy(&tmp, buf + sizeof(op), sizeof(tmp));
        tmp = ntohs(tmp);
        printf("  blk : %d\n", tmp);
    }
    else if (op == ERROR) {
        printf("  op      : ERROR\n");
        memcpy(&tmp, buf + sizeof(op), sizeof(tmp));
        tmp = ntohs(tmp);
        printf("  errno   : %d\n", tmp);
        printf("  err str : %s\n", buf + sizeof(op) + sizeof(tmp));
    }
}

/* tftp_terminate
 *   stop the tftp cli thread
 *
 *   return : none
 */
void
tftp_terminate
(
    void
)
{
    g_tftpRun = 0;
}

/* tftp_alarm_handler
 *   timeout alarm handler
 *
 *   sig : alarm signal id
 *
 *   return : none
 */
void
tftp_alarm_handler
(
    TFTP_INT4 sig
)
{
    if (sig == SIGALRM) {
        TFTP_DEBUG(">>>>> Tftp timeout <<<<<\n");
        tftp_terminate();
    }
}

/* tftp_open_local_file
 *   subroutine to open file in local area
 *
 *   path   : file path
 *   action : DDP_TFTP_GET or FFP_TFTP_PUT
 *
 *   return : file descriptor
 */
TFTP_INT4
tftp_open_local_file
(
    TFTP_INT1* path,
    TFTP_INT4 action
)
{
    TFTP_INT4 fd = 0;
    if (path == NULL) {
        TFTP_DEBUG("%s (%d) : null local file path\n", __FILE__, __LINE__);
        return fd;
    }
    umask(0);
    if (action == DDP_TFTP_GET) {
        fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);
        if (fd < 0) {
            if (errno == EEXIST) {//printf("exist\n");
                fd = open(path, O_WRONLY | O_TRUNC);
                if (fd < 0) {
                    TFTP_DEBUG("%s (%d) : fail to open local file %s for get (%d)\n", __FILE__, __LINE__, path, errno);
                }
            } else if (errno == EACCES) {//printf("PERMMMM\n");
                fd = open(path, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);
                if (fd < 0) {
                    TFTP_DEBUG("%s (%d) : fail to create file %s for get (%d)\n", __FILE__, __LINE__, path, errno)
                }
            } else {
                TFTP_DEBUG("%s (%d) : open local file %s for get fail (%d)\n", __FILE__, __LINE__, path, errno);
            }
        }//else printf("OKKKKK\n");
    }
    else if (action == DDP_TFTP_PUT) {
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            TFTP_DEBUG("%s (%d) : open local file %s for put fail (%d)\n", __FILE__, __LINE__, path, errno);
        }
    }
    return fd;
}

/* tftp_get
 *   get action of tftp (receive file from remote server)
 *
 *   args : information to be passed to tftp thread
 *
 *   return : a pointer to fit api
 */
void
*tftp_get
(
    void* args
)
{
    TFTP_INT4 ret = 0;
    struct tftp_info* info = NULL;
    TFTP_UINT1 buf[PKTSIZE];
    TFTP_UINT1 ack[4];
    TFTP_INT4 sock = 0;
    TFTP_UINT2 op = 0;
    TFTP_UINT2 tmp = 1;
    TFTP_INT1 first = 1;
    TFTP_INT4 pos = 0;
#ifdef __CYGWIN__
    TFTP_INT4 addrlen = 0;
#else
    TFTP_UINT4 addrlen = 0;
#endif
    TFTP_INT4 n = 0;
    TFTP_INT4 flag = MSG_DONTWAIT;
    TFTP_INT4 fd = 0;
    TFTP_UINT4 count = 0;
    TFTP_INT4 bytesWritten = 0;

    if (args == NULL) {
        ret = -1;
        goto tftp_get_over;
    }
    TFTP_DEBUG("Tftp cli thread starts to receive\n");
    info = (struct tftp_info*)args;
    /* create socket */
    if (info->serv.ss_family == AF_INET) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock <= 0) {
            TFTP_DEBUG("%s (%d) : create socket fail\n", __FILE__, __LINE__);
            ret = -2;
            goto tftp_get_over;
        }
        addrlen = sizeof(struct sockaddr_in);
    } else if (info->serv.ss_family == AF_INET6) {
        sock = socket(AF_INET6, SOCK_DGRAM, 0);
        if (sock <= 0) {
            TFTP_DEBUG("%s (%d) : create socket ipv6 fail\n", __FILE__, __LINE__);
            ret = -2;
            goto tftp_get_over;
        }
        addrlen = sizeof(struct sockaddr_in6);
    } else {
        TFTP_DEBUG("%s (%d) : address family is not supported (%d)\n", __FILE__, __LINE__, info->serv.ss_family);
        ret = -2;
        goto tftp_get_over;
    }
    /* open file to write downloaded file */
    fd = tftp_open_local_file(info->flocal, info->action);
    if (fd < 0) {
        ret = -3;
        goto tftp_get_over;
    }
    /* set timeout alarm if counter method is not enabled */
    if (! TFTP_TIMEOUT_COUNTER) {
        signal(SIGALRM, tftp_alarm_handler);
    }
    /* init */
    g_tftpRun = 1;
    info->status = DDP_UPGRADE_UNDER_PROCESS;
    info->blk = 1;
    info->fsize = 0;
    do {
        memset(buf, 0, sizeof(buf));
        memset(ack, 0, sizeof(ack));
        if (first) {
            /* issue RRQ */
            op = RRQ; op = htons(op);
            pos = 0;
            memcpy(buf, &op, sizeof(op));
            pos += sizeof(op);
            strcpy((INT1*)buf + pos, info->fremote);
            pos += (strlen(info->fremote) + 1);
            strcpy((INT1*)buf + pos, info->mode);
            pos += (strlen(info->mode) + 1);
            if (g_tftpDebug & TFTP_DEBUG_PACKET) {
                tftp_print_packet(buf);
            }
            if (sendto(sock, buf, pos, 0, (struct sockaddr*)&info->serv, addrlen) == -1) {
                TFTP_DEBUG("%s (%d) : sendto fail (err %d)\n", __FILE__, __LINE__, errno);
            }
            first = 0;
        } else {
            /* issue ACK */
            op = ACK; op = htons(op);
            pos = 0;
            memcpy(ack, &op, sizeof(op));
            tmp = htons(info->blk);
            pos += sizeof(op);
            memcpy(ack + sizeof(op), &tmp, sizeof(tmp));
            pos += sizeof(tmp);
            if (g_tftpDebug & TFTP_DEBUG_PACKET) {
                tftp_print_packet(ack);
            }
            if (sendto(sock, ack, pos, 0, (struct sockaddr*)&info->serv, addrlen) == -1) {
                TFTP_DEBUG("%s (%d) : sendto fail (err %d)\n", __FILE__, __LINE__, errno);
            }
        }
        /* start timeout count down */
        if (TFTP_TIMEOUT_COUNTER) {
            count = 0;
        } else {
            alarm(g_tftpTimeout);
        }
        /* receive packet from tftp server */
        do {
            n = recvfrom(sock, buf, sizeof(buf), flag, (struct sockaddr*)&info->serv, &addrlen);
            if (n > 0) {
                memcpy(&op, buf, sizeof(op));
                op = ntohs(op);
                if (op == DATA) {
                    memcpy(&tmp, buf + sizeof(op), sizeof(tmp));
                    tmp = ntohs(tmp);
                    TFTP_DEBUG("TFTP GET : recv %d bytes, op %d, blk %d\n", n, op, tmp);
                    if (fd > 0) {
                        if ((n - 4) > 0) {
                            bytesWritten = write(fd, buf + 4, n - 4);
                            if (g_tftpDebug & TFTP_DEBUG_PACKET) {
                                if (bytesWritten >= 0) {
                                    printf("%s (%d) : write %d bytes to file\n", __FILE__, __LINE__, bytesWritten);
                                }
                            }
                            if (bytesWritten < 0) {
                                TFTP_DEBUG("%s (%d) : write file fail (error %d)\n", __FILE__, __LINE__, errno);
                            }
                        }
                    }
                    info->fsize += (n - 4);
                    if (tmp == (info->blk + 1)) { info->blk = tmp; }
                }
                else if (op == ERROR) {
                    tftp_print_packet(buf);
                }
            } else {
                usleep(1000 * g_tftpLoopInterval);
            }
            /* check timeout flag */
            if (g_tftpRun == 0) {
                TFTP_DEBUG(">>>>> Tftp timeout <<<<<\n");
                ret = -10;
                snprintf(info->statusStr, sizeof(info->statusStr), "TFTP timeout at downloading");
                goto tftp_get_over;
            }
            /* check timeout counter */
            if (TFTP_TIMEOUT_COUNTER) {
                if (count > (g_tftpTimeout * 1000 / g_tftpLoopInterval)) {
                     tftp_terminate();
                }
                count++;
            }
        } while (n <= 0);
        /* stop timeout alarm */
        if (! TFTP_TIMEOUT_COUNTER) {
            alarm(0);
        }
    } while (n == PKTSIZE);

tftp_get_over:
    /* disable alarm */
    if (! TFTP_TIMEOUT_COUNTER) {
        alarm(0);
    }
    if (fd > 0) {
        close(fd); fd = 0;
    }
    if (ret != 0) {
        info->status = DDP_UPGRADE_FAIL;
    } else {
        /* TRADEOFF
         * According to DDP spec, the state is PROCESS_DONE because there is following action "flashing".
         * When flashing is done successfully, the state is transit to SUCCESS.
         * My opinion is to issue an opcode "FLASH" to do the flashing if downloading is successful.
         */
        info->status = DDP_UPGRADE_SUCCESS;//PROCESS_DONE;
    }
    if (sock > 0) {
        close(sock); sock = 0;
    }
    /* clear upgrade flag */
    ddp_upgrade_set_flag(0);
    TFTP_DEBUG("Tftp get thread stops\n");

    return args;
}

/* tftp_put
 *   put action of tftp (send file to remote server)
 *
 *   args : information to be passed to tftp thread
 *
 *   return : a pointer to fit api
 */
void
*tftp_put
(
    void* args
)
{
    TFTP_INT4 ret = 0;
    struct tftp_info* info = NULL;
    TFTP_UINT1 buf[PKTSIZE];
    TFTP_UINT1 ack[100]; /* set ack uffer size larger in order to contain ERROR packet */
    TFTP_INT4 sock = 0;
    TFTP_UINT2 op = 0;
    TFTP_UINT2 blk = 0;
    TFTP_INT4 sizeReadFile = 0;
    TFTP_INT4 pos = 0;
#ifdef __CYGWIN__
    TFTP_INT4 addrlen = 0;
#else
    TFTP_UINT4 addrlen = 0;
#endif
    TFTP_INT4 n = 0;
    TFTP_INT4 flag = MSG_DONTWAIT;
    TFTP_INT4 fd = 0;
    TFTP_UINT4 fpos = 0;
    struct stat st;
    TFTP_UINT4 count = 0;

    if (args == NULL) {
        ret = -1;
        goto tftp_put_over;
    }
    TFTP_DEBUG("Tftp cli thread starts to send\n");
    info = (struct tftp_info*)args;
    if (info->serv.ss_family == AF_INET) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock <= 0) {
            TFTP_DEBUG("%s (%d) : create socket fail\n", __FILE__, __LINE__);
            snprintf(info->statusStr, sizeof(info->statusStr), "Fail to create ipv4 socket for tftp");
            ret = -2;
            goto tftp_put_over;
        }
        addrlen = sizeof(struct sockaddr_in);
    }
    else if (info->serv.ss_family == AF_INET6) {
        sock = socket(AF_INET6, SOCK_DGRAM, 0);
        if (sock <= 0) {
            TFTP_DEBUG("%s (%d) : create socket ipv6 fail\n", __FILE__, __LINE__);
            snprintf(info->statusStr, sizeof(info->statusStr), "Fail to ipv6 create socket for tftp");
            ret = -2;
            goto tftp_put_over;
        }
        addrlen = sizeof(struct sockaddr_in6);
    }
    else {
        TFTP_DEBUG("%s (%d) : address family is not supported (%d)\n", __FILE__, __LINE__, info->serv.ss_family);
        snprintf(info->statusStr, sizeof(info->statusStr), "Fail to create socket for tftp");
        ret = -2;
        goto tftp_put_over;
    }
    /* open file to read for uploading */
    fd = tftp_open_local_file(info->flocal, info->action);
    if (fd <= 0) {
        snprintf(info->statusStr, sizeof(info->statusStr), "Fail to open local file for tftp");
        ret = -3;
        goto tftp_put_over;
    } else {
        stat(info->flocal, &st);
        info->fsize = st.st_size;
    }
    /* set timeout alarm handler */
    if (! TFTP_TIMEOUT_COUNTER) {
        signal(SIGALRM, tftp_alarm_handler);
    }
    /* set file position */
    if (lseek(fd, 0, SEEK_SET) < 0) {
        snprintf(info->statusStr, sizeof(info->statusStr), "Fail to set file position for tftp");
        ret = -4;
        goto tftp_put_over;
    }
    /* init */
    g_tftpRun = 1;
    info->status = DDP_UPGRADE_UNDER_PROCESS;
    do {
        memset(buf, 0, sizeof(buf));
        memset(ack, 0, sizeof(ack));
        if (blk == 0) {
            /* issue WRQ */
            op = WRQ; op = htons(op);
            pos = 0;
            memcpy(buf, &op, sizeof(op));
            pos += sizeof(op);
            strcpy((INT1*)buf + pos, info->fremote);
            pos += (strlen(info->fremote) + 1);
            strcpy((INT1*)buf + pos, info->mode);
            pos += (strlen(info->mode) + 1);
            if (g_tftpDebug & TFTP_DEBUG_PACKET) {
                tftp_print_packet(buf);
            }
            if (sendto(sock, buf, pos, 0, (struct sockaddr*)&info->serv, addrlen) == -1) {
                TFTP_DEBUG("%s (%d) : sendto fail (err %d)\n", __FILE__, __LINE__, errno);
            }
        } else {
            /* issue DATA */
            if (blk == 0) { blk = 1; }
            fpos = (blk - 1) * SEGSIZE;
            if ((info->fsize - fpos) >= 0) {
                if (lseek(fd, fpos, SEEK_SET) < 0) {
                    ret = -5;
                    goto tftp_put_over;
                }
            } else {
                TFTP_DEBUG("%s (%d) : read position is beyond file size\n", __FILE__, __LINE__);
                snprintf(info->statusStr, sizeof(info->statusStr), "Fail to read file for tftp");
                ret = -5;
                goto tftp_put_over;
            }
            op = DATA; op = htons(op);
            pos = 0;
            memcpy(buf, &op, sizeof(op));
            pos += sizeof(op);
            blk = htons(blk);
            memcpy(buf + pos, &blk, sizeof(blk));
            pos += sizeof(blk);
            sizeReadFile = read(fd, (TFTP_INT1*)(buf + pos), SEGSIZE);
            pos += sizeReadFile;
            if (g_tftpDebug & TFTP_DEBUG_PACKET) {
                tftp_print_packet(buf);
            }
            if (sendto(sock, buf, pos, 0, (struct sockaddr*)&info->serv, addrlen) == -1) {
                TFTP_DEBUG("%s (%d) : sendto fail (err %d)\n", __FILE__, __LINE__, errno);
            }
        }
        /* start timeout count down */
        if (TFTP_TIMEOUT_COUNTER) {
            count = 0;
        } else {
            alarm(g_tftpTimeout);
        }
        /* receive packet from server */
        do {
            n = recvfrom(sock, ack, sizeof(ack), flag, (struct sockaddr*)&info->serv, &addrlen);
            if (n > 0) {
                memcpy(&op, ack, sizeof(op));
                op = ntohs(op);
                if (op == ACK) {
                    memcpy(&blk, ack + sizeof(op), sizeof(blk));
                    blk = ntohs(blk);
                    TFTP_DEBUG("TFTP PUT : recv %d bytes, op %d, blk %d\n", n, op, blk);
                    if (blk == 0) { blk = 1; }
                    else { blk++; }
                }
                else if (op == ERROR) {
                    tftp_print_packet(ack);
                    memcpy(&blk, ack + sizeof(op), sizeof(blk));
                    blk = ntohs(blk);
                    snprintf(info->statusStr, sizeof(info->statusStr), "TFTP error: (%d) %s", blk, ack + 4);
                    ret = -6;
                    goto tftp_put_over;
                }
            } else {
                usleep(1000 * g_tftpLoopInterval);
            }
            /* check timeout flag */
            if (g_tftpRun == 0) {
                TFTP_DEBUG(">>>>> Tftp timeout <<<<<\n");
                snprintf(info->statusStr, sizeof(info->statusStr), "TFTP timeout at uploading");
                ret = -7;
                goto tftp_put_over;
            }
            /* check timeout counter */
            if (TFTP_TIMEOUT_COUNTER) {
                if (count > (g_tftpTimeout * 1000 / g_tftpLoopInterval)) {
                     tftp_terminate();
                }
                count++;
            }
        } while (n <= 0);
        /* stop timeout alarm */
        if (! TFTP_TIMEOUT_COUNTER) {
            alarm(0);
        }
    } while (sizeReadFile == SEGSIZE || blk == 1);

tftp_put_over:
    /* disable alarm */
    if (! TFTP_TIMEOUT_COUNTER) {
        alarm(0);
    }
    if (fd > 0) {
        close(fd); fd = 0;
    }
    if (ret != 0) {
        info->status = DDP_UPGRADE_FAIL;
    } else {
        info->status = DDP_UPGRADE_SUCCESS;//PROCESS_DONE;
    }
    if (sock > 0) {
        close(sock); sock = 0;
    }
    /* clear upgrade flag */
    ddp_upgrade_set_flag(0);
    TFTP_DEBUG("Tftp put thread stops\n");

    return args;
}

void
*sys_tftp_get
(
    void* args
)
{
    INT4 ret = 0;
    struct tftp_info* info = NULL;
    struct sockaddr_storage *server = NULL;
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    INT1 buf[1024];
    INT1 addr[64];
    //INT2 port = 0;

    if (args == NULL) {
        ret = -1;
        goto sys_tftp_get_over;
    }
    info = (struct tftp_info*)args;
    memset(addr, 0, sizeof(addr));
    server = (struct sockaddr_storage*)&(info->serv);
    if (info->serv.ss_family == AF_INET) {
        inAddr = (struct sockaddr_in*)server;
        if (inet_ntop(AF_INET, &inAddr->sin_addr, addr, sizeof(addr)) == NULL) {
            ret = -3;
            goto sys_tftp_get_over;
        }
        //port = inAddr->sin_port;
    } else if (info->serv.ss_family == AF_INET6) {
        inAddr6 = (struct sockaddr_in6*)server;
        if (inet_ntop(AF_INET6, &inAddr6->sin6_addr, addr, sizeof(addr)) == NULL) {
            ret = -4;
            goto sys_tftp_get_over;
        }
        //port = inAddr6->sin6_port;
    } else {
        ret = -2;
        goto sys_tftp_get_over;
    }
    /* download */
    info->status = DDP_UPGRADE_UNDER_PROCESS;
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%s -l %s -r %s -g %s", TFTP_PATH, info->flocal, info->fremote, addr);
    TFTP_DEBUG("TFTP_CMD: %s\n", buf);
    ret = system(buf);
    ret = WEXITSTATUS(ret);
    if (ret != 0) {
        goto sys_tftp_get_over;
    }

    /* flashing */

sys_tftp_get_over:
    if (ret != 0) {
        info->status = DDP_UPGRADE_FAIL;
    } else {
        info->status = DDP_UPGRADE_SUCCESS;//PROCESS_DONE;
    }
    ddp_upgrade_set_flag(0);
    TFTP_DEBUG("Sys tftp get thread stops\n");

    return args;
}

void
*sys_tftp_put
(
    void* args
)
{
    INT4 ret = 0;
    struct tftp_info* info = NULL;
    struct sockaddr_storage *server = NULL;
    struct sockaddr_in* inAddr = NULL;
    struct sockaddr_in6* inAddr6 = NULL;
    INT1 buf[1024];
    INT1 addr[64];
    //INT2 port = 0;

    if (args == NULL) {
        ret = -1;
        goto sys_tftp_put_over;
    }
    info = (struct tftp_info*)args;
    memset(addr, 0, sizeof(addr));
    server = (struct sockaddr_storage*)&(info->serv);
    if (info->serv.ss_family == AF_INET) {
        inAddr = (struct sockaddr_in*)server;
        if (inet_ntop(AF_INET, &inAddr->sin_addr, addr, sizeof(addr)) == NULL) {
            ret = -3;
            goto sys_tftp_put_over;
        }
        //port = inAddr->sin_port;
    } else if (info->serv.ss_family == AF_INET6) {
        inAddr6 = (struct sockaddr_in6*)server;
        if (inet_ntop(AF_INET6, &inAddr6->sin6_addr, addr, sizeof(addr)) == NULL) {
            ret = -4;
            goto sys_tftp_put_over;
        }
        //port = inAddr6->sin6_port;
    } else {
        ret = -2;
        goto sys_tftp_put_over;
    }
    /* Before uploading, please prepare the file erady for uploading and write premission of tftp server. */
    /* upload */
    info->status = DDP_UPGRADE_UNDER_PROCESS;
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%s -l %s -r %s -p %s", TFTP_PATH, info->flocal, info->fremote, addr);
    TFTP_DEBUG("TFTP_CMD: %s\n", buf);
    ret = system(buf);
    ret = WEXITSTATUS(ret);
    if (ret != 0) {
        goto sys_tftp_put_over;
    }

sys_tftp_put_over:
    if (ret != 0) {
        info->status = DDP_UPGRADE_FAIL;
    } else {
        info->status = DDP_UPGRADE_SUCCESS;//PROCESS_DONE;
    }
    ddp_upgrade_set_flag(0);
    TFTP_DEBUG("Sys tftp put thread stops\n");
    return args;
}

/* tftp_cli?
 *   launch function of tftp client mode
 *   start a thread and run it without waiting for its termination
 *
 *   info : pointer of struct tftp_info
 *
 *   return : an integer (0 -> success, others -> error)
 */
TFTP_INT4
tftp_cli
(
    struct tftp_info* info
)
{
    TFTP_INT4 ret = 0;
    pthread_t tftpThread;
    pthread_attr_t attr;

    if (info == NULL) { return -1; }
    /* validate input data */
    if (info->action < DDP_TFTP_GET || DDP_TFTP_PUT < info->action) {
        info->status = DDP_UPGRADE_FAIL;
        strncpy(info->statusStr, "TFTP op is not supported", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
        ret = -2;
        goto tftp_cli_over;
    }
    if (info->mode == NULL) {
        info->status = DDP_UPGRADE_FAIL;
        strncpy(info->statusStr, "TFTP mode is not supported", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
        ret = -2;
        goto tftp_cli_over;
    }
    if (strlen(info->fremote) == 0) {
        info->status = DDP_UPGRADE_FAIL;
        strncpy(info->statusStr, "TFTP remote file is not set", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
        ret = -2;
        goto tftp_cli_over;
    }
    /* set thread as detached */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    /* get */
    if (info->action == DDP_TFTP_GET) {
#ifdef USE_SYS_TFTP
        if (pthread_create(&tftpThread, &attr, &sys_tftp_get, (void*)info) == 0) {
#else
        if (pthread_create(&tftpThread, &attr, &tftp_get, (void*)info) == 0) {
#endif
            //g_upgradeThreadId = tftpThread;
        } else {
            info->status = DDP_UPGRADE_FAIL;
            strncpy(info->statusStr, "Fail to create get thread", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
            ret = -3;
            ddp_upgrade_set_flag(0); /* in case of failing to create thread */
        }
    }
    /* put */
    else if (info->action == DDP_TFTP_PUT) {
#ifdef USE_SYS_TFTP
        if (pthread_create(&tftpThread, &attr, &sys_tftp_put, (void*)info) == 0) {
#else
        if (pthread_create(&tftpThread, &attr, &tftp_put, (void*)info) == 0) {
#endif /* USE_SYS_TFTP */
            //g_upgradeThreadId = tftpThread;
        } else {
            info->status = DDP_UPGRADE_FAIL;
            strncpy(info->statusStr, "Fail to create put thread", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
            ret = -3;
            ddp_upgrade_set_flag(0); /* in case of failing to craete thread */
        }
    }
    else {
        info->status = DDP_UPGRADE_FAIL;
        strncpy(info->statusStr, "Tftp action is not supported", DDP_FIELD_LEN_UPGRADE_FAIL_STR);
        ret = -3;
    }

tftp_cli_over:
    pthread_attr_destroy(&attr);
    /* do not wait tftp thread */
    return ret;
}
