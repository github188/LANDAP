
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "base64.h"

void
show_help
(
    void
)
{
    printf("Usage: base64_test <str>\n");
    printf("     str in ascii code\n");
}

INT4
main
(
    INT4 argc,
    INT1** argv
)
{
    INT1* strEncode = NULL;
    INT1* strDecode = NULL;
    INT4 iEncodeLen = 0;
    INT4 iDecodeLen = 0;
    INT4 iReturn = 0;

    printf("\nTest case for base64 : start\n\n");

    if (argc < 2) {
        show_help();
        iReturn = -1;
        goto test_end;
    }
    if (argv[1] == NULL || strlen(argv[1]) == 0) {
        show_help();
        iReturn = -2;
        goto test_end;
    }

    iEncodeLen = base64_calculate_encrypted_length(argv[1]);
    printf("  Encrypted string length (in byte) = %d\n", iEncodeLen);
    strEncode = (INT1*)malloc((iEncodeLen + 1) * sizeof(INT1));
    if (strEncode == NULL) {
        iReturn = -3;
        goto test_end;
    }
    memset(strEncode, 0, iEncodeLen + 1);
    base64_encode(argv[1], strEncode, iEncodeLen);
    printf("  Encrypted string : %s\n", strEncode);

    iDecodeLen = base64_calculate_decrypted_length(strEncode);
    printf("  Decrypted string length (in byte) = %d\n", iDecodeLen);
    strDecode = (INT1*)malloc((iDecodeLen + 1) * sizeof(INT1));
    if (strDecode == NULL) {
        iReturn = -4;
        goto test_end;
    }
    memset(strDecode, 0, iDecodeLen + 1);
    base64_decode(strEncode, strDecode, iDecodeLen);
    printf("  Decrypted string : %s\n", strDecode);

test_end:
    if (strEncode) { free(strEncode); strEncode = NULL; }
    if (strDecode) { free(strDecode); strDecode = NULL; }
    printf("\nTest case for base64 : end\n");
    return iReturn;
}
