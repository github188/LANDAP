
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ddp.h"
#include "version.h"


/* unix socket id for commandline */
INT4 g_shellSock = 0;


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
    INT1 buf[100];
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
        memset(buf, 0, sizeof(buf));
        read_len = recvfrom(g_shellSock, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr*)&peer, (socklen_t*)&peer_len);
        if (read_len > 0) {
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_CMD, "Shell recv : %s\n", buf);
            if (strcmp(buf, "e") == 0) {
                memset(buf, 0, sizeof(buf));
                if (ddp_run_state(DDP_RUN_STATE_SET, DDP_RUN_STATE_RUN) == -1) {
                    strcpy(buf, "Enable fail");
                } else {
                    strcpy(buf, "Enable OK");
                }
            }
            else if (strcmp(buf, "d") == 0) {
                memset(buf, 0, sizeof(buf));
                if (ddp_run_state(DDP_RUN_STATE_SET, DDP_RUN_STATE_HALT) == -1) {
                    strcpy(buf, "Disable fail");
                } else {
                    strcpy(buf, "Disable OK");
                }
            }
            else if (strcmp(buf, "v") == 0) {
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "%d.%d.%d", DDP_ENGINE_MAJOR_VERSION, DDP_ENGINE_MINOR_VERSION, DDP_ENGINE_BUILD_NUMBER);
            }
            else if (strcmp(buf, "s") == 0) {
                memset(buf, 0, sizeof(buf));
                ddp_search_devices();
            }
            else {
                memset(buf, 0, sizeof(buf));
                strcpy(buf, "Unknown command");
            }
            /* send reply */
            DDP_DEBUG_LEVEL(DDP_DEBUG_PRINT_CMD, "Shell reply: %s\n", buf);
            sendto(g_shellSock, buf, strlen(buf), 0, (struct sockaddr*)&peer, peer_len);
        }
        /* no string */
        else {
            sleep(1);
        }
        //if (connfd) { close(connfd); connfd = 0; }
    }

    return ret;
}
