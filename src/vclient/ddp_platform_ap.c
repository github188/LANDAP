
#include <stdio.h>
#include <string.h>

#include "ddp.h"
#include "ddp_platform.h"


extern struct ssid_entry ssid_table_24g[];
extern struct ssid_entry ssid_table_5g[];

/* ddp_platform_set_multi_ssid
 *   to set ssid into device configuration (the first 8 ssid groups).
 *
 *   band : frequency id
 *   table : new configuration
 *
 *   return : 0 -> success, others ->error
 */
INT4
ddp_platform_set_multi_ssid
(
    INT1 band,
    struct ssid_entry* table
)
{
    INT4 ret = 0;
    INT4 index = 0;
    
    if (table == NULL) { return -1; }
    if (band != DDP_WL_BAND_24GHZ && band != DDP_WL_BAND_5GHZ) { return -2; }

    /* 2.4GHz */
    if (band == DDP_WL_BAND_24GHZ) {
        for (index = 0; index < SSID_NUMBER; index++) {
            /* print out the old and new configuration for debug */
            if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
                printf("SSID %d\n", index);
                if (strlen(ssid_table_24g[index].name)) {
                    printf("  name %s -> ", ssid_table_24g[index].name);
                } else {
                    printf("  name (none) -> ");
                }
                if (strlen((table + index)->name)) {
                    printf("%s\n", (table + index)->name);
                } else {
                    printf("(none)\n");
                }
                printf("  vid %d -> %d\n", ssid_table_24g[index].vid, (table + index)->vid);
                printf("  security %d -> %d\n", ssid_table_24g[index].security, (table + index)->security);
                if (strlen(ssid_table_24g[index].password)) {
                    printf("  password %s -> ", ssid_table_24g[index].password);
                } else {
                    printf("  password (none) -> ");
                }
                if (strlen((table + index)->password)) {
                    printf("%s\n", (table + index)->password);
                } else {
                    printf("(none)\n");
                }
            }
            /* Platform developer should set the ssid configuration to system here.
             * Set the result to ret variable.
             */
            memcpy(ssid_table_24g[index].name, (table + index)->name, DDP_FIELD_LEN_SSID_NAME);
            ssid_table_24g[index].vid = (table + index)->vid;
            ssid_table_24g[index].security = (table + index)->security;
            memcpy(ssid_table_24g[index].password, (table + index)->password, DDP_FIELD_LEN_SSID_SEC_PASSWD);
        }
    }
    /* 5GHz */
    else {
        for (index = 0; index < SSID_NUMBER; index++) {
            /* print out the old and new configuration for debug */
            if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
                printf("SSID %d\n", index);
                if (strlen(ssid_table_5g[index].name)) {
                    printf("  name %s -> ", ssid_table_5g[index].name);
                } else {
                    printf("  name (none) -> ");
                }
                if (strlen((table + index)->name)) {
                    printf("%s\n", (table + index)->name);
                } else {
                    printf("(none)\n");
                }
                printf("  vid %d -> %d\n", ssid_table_5g[index].vid, (table + index)->vid);
                printf("  security %d -> %d\n", ssid_table_5g[index].security, (table + index)->security);
                if (strlen(ssid_table_5g[index].password)) {
                    printf("  password %s -> ", ssid_table_5g[index].password);
                } else {
                    printf("  password (none) -> ");
                }
                if (strlen((table + index)->password)) {
                    printf("%s\n", (table + index)->password);
                } else {
                    printf("(none)\n");
                }
            }
            /* Platform developer should set the ssid configuration to system here.
             * set the result to ret variable.
             */
            memcpy(ssid_table_5g[index].name, (table + index)->name, DDP_FIELD_LEN_SSID_NAME);
            ssid_table_5g[index].vid = (table + index)->vid;
            ssid_table_5g[index].security = (table + index)->security;
            memcpy(ssid_table_5g[index].password, (table + index)->password, DDP_FIELD_LEN_SSID_SEC_PASSWD);
        }
    }
    return ret;
}

/* ddp_platform_get_multi_ssid
 *   to get ssid into device configuration (first 8 ssid groups).
 *
 *   band : frequency id
 *   table : container to store configuraion
 *
 *   return : 0 -> success, others ->error
 */
INT4
ddp_platform_get_multi_ssid
(
    INT1 band,
    struct ssid_entry* table
)
{
    INT4 ret = 0;
    INT4 index = 0;
    
    if (table == NULL) { return -1; }
    if (band != DDP_WL_BAND_24GHZ && band != DDP_WL_BAND_5GHZ) { return -2; }

    /* 2.4GHz */
    if (band == DDP_WL_BAND_24GHZ) {
        /* Platform developer should retrieve ssid configuration here.
         * Set the result to ret variable.
         */
        for (index = 0; index < SSID_NUMBER; index++) {
            memcpy((table + index)->name, ssid_table_24g[index].name, DDP_FIELD_LEN_SSID_NAME);
            (table + index)->vid = ssid_table_24g[index].vid;
            (table + index)->security = ssid_table_24g[index].security;
            memcpy((table + index)->password, ssid_table_24g[index].password, DDP_FIELD_LEN_SSID_SEC_PASSWD);
        }
    }
    /* 5GHz */
    else {
        /* Platform developer should retrieve ssid configuration here.
         * Set the result to ret variable.
         */
        for (index = 0; index < SSID_NUMBER; index++) {
            memcpy((table + index)->name, ssid_table_5g[index].name, DDP_FIELD_LEN_SSID_NAME);
            (table + index)->vid = ssid_table_5g[index].vid;
            (table + index)->security = ssid_table_5g[index].security;
            memcpy((table + index)->password, ssid_table_5g[index].password, DDP_FIELD_LEN_SSID_SEC_PASSWD);
        }
    }
    /* print out the configuration for debug */
    if (g_debugFlag & DDP_DEBUG_PRINT_PLATFORM) {
        for (index = 0; index < SSID_NUMBER; index++) {
            printf("SSID %d\n", index);
            if (strlen((table + index)->name)) { printf("  name     %s\n", (table + index)->name); }
            else { printf("  name     (none)\n"); }
            printf("  vid      %d\n", (table + index)->vid);
            printf("  security %d\n", (table + index)->security);
            if (strlen((table + index)->password)) { printf("  password %s\n", (table + index)->password); }
            else { printf("  password (none)\n"); }
        }
    }
    return ret;
}
