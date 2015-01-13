
#include <stdio.h>

#include "ddp.h"


/* Main function: process starts here.
 */
INT4
main
(
    INT4 argc,
    INT1** argv
)
{
    printf("DDP starts\n");
    /* Process should enter ddp module and would not come back, unless it is terminated. */
    ddp_entrance(argc, argv);

    printf("DDP stops\n");
    return 0;
}
