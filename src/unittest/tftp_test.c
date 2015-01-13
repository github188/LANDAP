
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ddp.h"
#include "tftp_cli.h"


INT4
main(INT4 argc, INT1** argv)
{
    if (argc < 4) {
        printf("Usage: tftp_test [GET | PUT] <remote file> <local file>\n");
        exit(0);
    }
    printf("Test case of tftp cli : start\n");

    INT2 action = 0;
    UINT1 lo[] = {127, 0, 0, 1};
    struct tftp_info info;
    struct sockaddr_in* sa = NULL;

    memset(&info, 0, sizeof(info));
    if (strcmp(argv[1], "GET") == 0) { info.action = DDP_TFTP_GET; }
    else if (strcmp(argv[1], "PUT") == 0) { info.action = DDP_TFTP_PUT; }
    sa = (struct sockaddr_in*)&info.serv;
    sa->sin_family = AF_INET;
    memcpy((void*)&sa->sin_addr, lo, sizeof(lo));
    sa->sin_port = htons(DDP_TFTP_PORT);
    strcpy(info.fremote, argv[2]);
    strcpy(info.flocal, argv[3]);
    info.mode = g_tftpMode_bin;

    tftp_cli(&info);

    do {
        sleep(5);
    } while (info.status == DDP_UPGRADE_UNDER_PROCESS);

    printf("Test case of tftp cli : end\n");
    return 0;
}
