
#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>


#include "ddp.h"

#define DDP_DEBUG_CMD 		0
#define MSG_BUFFER_LENGTH	4096

/* ddp_command_menu
 *   function to display help menu
 */
void
ddp_command_menu
(
    void
)
{
    printf("Usage: ddp [-hved]\n");
    printf("  h: display this menu\n");
    printf("  v: version\n");
    printf("  e: enable ddp\n");
    printf("  d: disable ddp\n");
}

/* main
 *   main function of ddp commandline
 */
INT4
main
(
    INT4 argc,
    INT1** argv
)
{
    INT4 fd = 0;
    INT1* opt_str = NULL;
    struct sockaddr_un cli;
    struct sockaddr_un srv;
    INT4 srv_len = sizeof(srv);
    INT1 line[MSG_BUFFER_LENGTH];
    INT1 path[20];
    INT4 read_len = 0;
    INT4 debugFlag = DDP_DEBUG_CMD;

    /* check whether ddpd is running or not */
    if (system("pid=$(ps -ef | grep ddpd | grep -v grep | awk '{print $2}'); \
                 if [ \"$pid\" = \"\" ] || [ \"$pid\" = \"0\" ]; then exit 0; \
                 else exit 1; fi") == 0) {
        printf("CMD : no ddpd running\n");
        return 0;
    }
    /* parse option string
     * require definition
     */
    if (argc == 1) {
        ddp_command_menu();
        goto ddp_command_main_over;
    } else if (argc > 1 && argv[1] == NULL) {
        if (debugFlag) { printf("CMD : no opt\n"); }
        goto ddp_command_main_over;
    }

    opt_str = (INT1*)malloc(64);
    memset(opt_str, 0, 64);
    if(argc == 3) {
    	sprintf(opt_str, "%s %s", argv[1], argv[2]);
    }
    else
    	strcpy(opt_str, argv[1]);

    /* create socket */
    fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (fd <= 0) {
        if (debugFlag) { printf("CMD : create socket fail\n"); }
        goto ddp_command_main_over;
    }
    
    memset(path, 0, sizeof(path));
    sprintf(path, "%s/%s", DDP_SHELL_SOCKET_DIR, DDP_SHELL_SOCKET_FILE);
    
    memset(&cli, 0, sizeof(cli));
    cli.sun_family = AF_LOCAL;
    strcpy(srv.sun_path, path);
    if (bind(fd, (struct sockaddr*)&cli, sizeof(cli)) < 0) {
        if (debugFlag) { printf("CMD : ddp command bind unix socket fail\n"); }
        goto ddp_command_main_over;
    }
    /* send command and option string */
    srv.sun_family = AF_LOCAL;
    strcpy(srv.sun_path, path);
    sendto(fd, opt_str, strlen(opt_str), 0, (struct sockaddr*)&srv, srv_len);
    /* receive reply */
    memset(line, 0, sizeof(line));
    read_len = recv(fd, line, sizeof(line), 0);
    if (debugFlag) { printf("CMD : read %d bytes\n", read_len); }
    /* display reply */
    if (read_len > 0) {
        printf("%s\n", line);
    }

    free(opt_str);

ddp_command_main_over:
    if (fd > 0) { close(fd); fd = 0; }
    return 0;
}
