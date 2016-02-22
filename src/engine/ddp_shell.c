
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ddp.h"
#include "version.h"

#define MSG_BUFFER_LENGTH	4096

/* unix socket id for commandline */
INT4 g_shellSock = 0;
INT1 msg_buf[MSG_BUFFER_LENGTH];

/* ddp_shell_stop
 *   function to clen up unix socket.
 *
 * return : 0
 */
INT4
ddp_shell_stop
(
    void
)
{
    INT1 buf[40];

    if (g_shellSock > 0) { close(g_shellSock); g_shellSock = 0; }
    /* unlink socket file */
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%s/%s", DDP_SHELL_SOCKET_DIR, DDP_SHELL_SOCKET_FILE);
    unlink(buf);
    /* delete socket file */
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "rm -f %s/%s", DDP_SHELL_SOCKET_DIR, DDP_SHELL_SOCKET_FILE);
    system(buf);
    
    return 0;
}

/* ddp_shell_start
 *   function to initiate unix socket for ccommandline
 *
 * return : 0 -> success, others -> error
 */
INT4
ddp_shell_start
(
    void
)
{
    INT4 ret = 0;
    INT1 cmd[256];
    INT1 path[20];
    struct sockaddr_un addr;

    /* create unix socket */
    g_shellSock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (g_shellSock <= 0) { return -2; }
    /* create tmp dir */
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "if [ ! -d %s ]; then mkdir -p %s; fi", DDP_SHELL_SOCKET_DIR, DDP_SHELL_SOCKET_DIR);
    system(cmd);
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "if [ -e %s/%s ]; then rm -f %s/%s; fi", DDP_SHELL_SOCKET_DIR, DDP_SHELL_SOCKET_FILE, DDP_SHELL_SOCKET_DIR, DDP_SHELL_SOCKET_FILE);
    system(cmd);
    /* bind unix socket to tmp file */
    memset(path, 0, sizeof(path));
    sprintf(path, "%s/%s", DDP_SHELL_SOCKET_DIR, DDP_SHELL_SOCKET_FILE);
    DDP_DEBUG("shell socket path %s\n", path);
    unlink(path);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, path, strlen(path));
    if (bind(g_shellSock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) < 0) {
        DDP_DEBUG("%s (%d) : shell socket bind fail\n", __FILE__, __LINE__);
        ret = -3;
        goto ddp_shell_start_over;
    }
    chmod(path, 0666);
    /*if (listen(g_shellSock, 2) < 0) {
        DDP_DEBUG("%s (%d) : cli socket listen fail\n", __FILE__, __LINE__);
        ret = -4;
        goto ddp_shell_start_over;
    }*/

ddp_shell_start_over:
    if (ret != 0) {
        ddp_shell_stop();
    }
    return ret;
}

INT4 ddp_srvv1_shell_add_sendmsg
(
	INT1* msg
)
{
	int len = strlen(msg);
	if( len > 0 && MSG_BUFFER_LENGTH - strlen(msg_buf) >= len ) {
		if(strlen(msg_buf) > 0 && msg_buf[strlen(msg_buf) - 1] != '\n') {
			char *break_line = "\n";
			strcat(msg_buf, break_line);
		}
		strcat(msg_buf, msg);
		return 0;
	}
	return -1;
}

/* ddp_shell_thread
 *   thread function to receive command from console.
 *
 * strThreadName : thread name
 *
 * return : 0 -> success, others -> error
 */
INT4
ddp_shell_thread
(
    INT1* strThreadName
)
{
    INT4 ret = 0;
    INT4 iLoop = 0;
    struct sockaddr_un peer;
    INT4 peer_len = sizeof(peer);
    INT4 read_len = 0;
    //INT4 connfd = 0;

    if (g_shellSock <= 0) { return -1; }

    while (1) {
        iLoop = ddp_get_loop_flag();
        if (iLoop & DDP_OVER_SIG) { break; }

        /*connfd = accept(g_cliSock, (struct sockaddr_un*)&peer, &peer_len);
        if (connfd < 0) {
            DDP_DEBUG("%s (%d) : accept error\n", __FILE__, __LINE__);
            continue;
        }*/
        /* recv and parse cmd */
        memset(msg_buf, 0, sizeof(msg_buf));
        read_len = recvfrom(g_shellSock, msg_buf, sizeof(msg_buf), MSG_DONTWAIT, (struct sockaddr*)&peer, (socklen_t*)&peer_len);
        if (read_len > 0) {
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_CMD, "Shell recv : %s\n", msg_buf);
            if (strcmp(msg_buf, "e") == 0) {
                memset(msg_buf, 0, sizeof(msg_buf));
                if (ddp_run_state(DDP_RUN_STATE_SET, DDP_RUN_STATE_RUN) == -1) {
                    strcpy(msg_buf, "Enable fail");
                } else {
                    strcpy(msg_buf, "Enable OK");
                }
            }
            else if (strcmp(msg_buf, "d") == 0) {
                memset(msg_buf, 0, sizeof(msg_buf));
                if (ddp_run_state(DDP_RUN_STATE_SET, DDP_RUN_STATE_HALT) == -1) {
                    strcpy(msg_buf, "Disable fail");
                } else {
                    strcpy(msg_buf, "Disable OK");
                }
            }
            else if (strcmp(msg_buf, "v") == 0) {
                memset(msg_buf, 0, sizeof(msg_buf));
                sprintf(msg_buf, "%d.%d.%d", DDP_ENGINE_MAJOR_VERSION, DDP_ENGINE_MINOR_VERSION, DDP_ENGINE_BUILD_NUMBER);
            }
            else if (strcmp(msg_buf, "s") == 0 || strncmp(msg_buf, "s ", 2) == 0) {
            	INT1 cmd[MSG_BUFFER_LENGTH];
            	strncpy(cmd, msg_buf, sizeof(cmd));
                memset(msg_buf, 0, sizeof(msg_buf));
                ddp_srvv1_req_discovery();

                INT1 *pSave;
                strtok_r(cmd , " ", &pSave);
                INT1 *pFiled2 = strtok_r(NULL, " ", &pSave);
                int timeout = 0;
				if(pFiled2)
					timeout = atoi(pFiled2);

                if((iLoop & 0x16)) {
                	if(timeout == 0)
                		timeout = 30;
                	sleep(timeout);
                }

            }
            else if (strcmp(msg_buf, "b") == 0) {
                memset(msg_buf, 0, sizeof(msg_buf));
                ddp_srvv1_req_basic_info();
            }
            else {
                memset(msg_buf, 0, sizeof(msg_buf));
                strcpy(msg_buf, "Unknown command");
            }
            /* send reply */
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_CMD, "Shell reply: %s\n", msg_buf);
            sendto(g_shellSock, msg_buf, strlen(msg_buf), 0, (struct sockaddr*)&peer, peer_len);

        }
        /* no string */
        else {
            sleep(1);
        }
        //if (connfd) { close(connfd); connfd = 0; }
    }

    return ret;
}
